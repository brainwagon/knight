#include "constellation.h"
#include "ephemerides.h"
#include "font8x8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static void draw_line_rgb(ImageRGB* img, int x0, int y0, int x1, int y1, float r, float g, float b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < img->width && y0 >= 0 && y0 < img->height) {
            int idx = y0 * img->width + x0;
            img->pixels[idx].r = r;
            img->pixels[idx].g = g;
            img->pixels[idx].b = b;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int load_constellation_boundaries(const char* filepath, ConstellationBoundary* boundary) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;

    int capacity = 1024;
    boundary->vertices = (ConstellationVertex*)malloc(sizeof(ConstellationVertex) * capacity);
    boundary->count = 0;
    boundary->label_count = 0;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (boundary->count >= capacity) {
            capacity *= 2;
            boundary->vertices = (ConstellationVertex*)realloc(boundary->vertices, sizeof(ConstellationVertex) * capacity);
        }

        float ra_h, dec_d;
        char abbr[4];
        if (sscanf(line, "%f %f %3s", &ra_h, &dec_d, abbr) == 3) {
            ConstellationVertex* v = &boundary->vertices[boundary->count];
            v->ra = ra_h * 15.0f * DEG2RAD; // Convert hours to radians
            v->dec = dec_d * DEG2RAD;        // Convert degrees to radians
            strncpy(v->abbr, abbr, 3);
            v->abbr[3] = '\0';
            boundary->count++;
        }
    }
    fclose(f);

    // Calculate centroids for labels
    if (boundary->count > 0) {
        char current_abbr[4] = "";
        double sum_x = 0, sum_y = 0, sum_z = 0;
        int vertex_count = 0;

        for (int i = 0; i <= boundary->count; i++) {
            bool end_of_const = (i == boundary->count) || (vertex_count > 0 && strcmp(boundary->vertices[i].abbr, current_abbr) != 0);

            if (end_of_const) {
                if (boundary->label_count < 88) {
                    ConstellationLabel* l = &boundary->labels[boundary->label_count++];
                    double ax = sum_x / vertex_count;
                    double ay = sum_y / vertex_count;
                    double az = sum_z / vertex_count;
                    
                    l->ra = (float)atan2(ay, ax);
                    if (l->ra < 0) l->ra += TWO_PI;
                    l->dec = (float)asin(az / sqrt(ax*ax + ay*ay + az*az));
                    
                    strncpy(l->abbr, current_abbr, 3);
                    l->abbr[3] = '\0';
                }
                if (i == boundary->count) break;
                sum_x = 0; sum_y = 0; sum_z = 0; vertex_count = 0;
            }

            strcpy(current_abbr, boundary->vertices[i].abbr);
            double ra = boundary->vertices[i].ra;
            double dec = boundary->vertices[i].dec;
            sum_x += cos(dec) * cos(ra);
            sum_y += cos(dec) * sin(ra);
            sum_z += sin(dec);
            vertex_count++;
        }
    }

    return 0;
}

void constellation_equ_to_horizon(double jd, double lat, double lon, ConstellationBoundary* boundary) {
    double gmst = greenwich_mean_sidereal_time(jd);
    double lmst = local_mean_sidereal_time(gmst, lon);
    double lat_rad = lat * DEG2RAD;

    // Transform boundary vertices
    for (int i = 0; i < boundary->count; i++) {
        ConstellationVertex* v = &boundary->vertices[i];
        
        double ha = lmst - v->ra;
        
        double sin_alt = sin(lat_rad) * sin(v->dec) + cos(lat_rad) * cos(v->dec) * cos(ha);
        double alt = asin(sin_alt);
        
        double cos_az = (sin(v->dec) - sin(alt) * sin(lat_rad)) / (cos(alt) * cos(lat_rad));
        if (cos_az > 1.0) cos_az = 1.0;
        if (cos_az < -1.0) cos_az = -1.0;
        
        double az = acos(cos_az);
        if (sin(ha) > 0) az = 2.0 * PI - az;
        
        v->az = (float)az;
        v->alt = (float)alt;
        v->direction.x = (float)(cos(alt) * sin(az));
        v->direction.y = (float)sin(alt);
        v->direction.z = (float)(cos(alt) * cos(az));
    }

    // Transform label centroids
    for (int i = 0; i < boundary->label_count; i++) {
        ConstellationLabel* l = &boundary->labels[i];
        
        double ha = lmst - l->ra;
        double sin_alt = sin(lat_rad) * sin(l->dec) + cos(lat_rad) * cos(l->dec) * cos(ha);
        double alt = asin(sin_alt);
        
        double cos_az = (sin(l->dec) - sin(alt) * sin(lat_rad)) / (cos(alt) * cos(lat_rad));
        if (cos_az > 1.0) cos_az = 1.0;
        if (cos_az < -1.0) cos_az = -1.0;
        
        double az = acos(cos_az);
        if (sin(ha) > 0) az = 2.0 * PI - az;
        
        l->az = (float)az;
        l->alt = (float)alt;
        l->direction.x = (float)(cos(alt) * sin(az));
        l->direction.y = (float)sin(alt);
        l->direction.z = (float)(cos(alt) * cos(az));
    }
}

bool project_vertex(Vec3 v_dir, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect, int width, int height, float* px, float* py) {
    float dz = vec3_dot(v_dir, cam_fwd);
    if (dz <= 0) return false;

    *px = (vec3_dot(v_dir, cam_right) / dz / (aspect * tan_half_fov) + 1.0f) * 0.5f * width;
    *py = (1.0f - vec3_dot(v_dir, cam_up) / dz / tan_half_fov) * 0.5f * height;

    return true;
}

void project_vertex_env(Vec3 v_dir, int width, int height, float* px, float* py) {
    float az = atan2f(v_dir.x, v_dir.z) * RAD2DEG;
    if (az < 0) az += 360.0f;
    float alt = asinf(v_dir.y) * RAD2DEG;
    
    *px = (az / 360.0f) * width;
    *py = (90.0f - alt) / 180.0f * height;
}

static void draw_line_env_wrapped(ImageRGB* img, float x0, float y0, float x1, float y1, RGB color) {
    int w = img->width;
    if (fabsf(x1 - x0) > w * 0.5f) {
        if (x1 > x0) {
            float x1_virtual = x1 - w;
            float t = (0.0f - x0) / (x1_virtual - x0);
            float y_edge = y0 + t * (y1 - y0);
            draw_line_rgb(img, (int)roundf(x0), (int)roundf(y0), 0, (int)roundf(y_edge), color.r, color.g, color.b);
            draw_line_rgb(img, w - 1, (int)roundf(y_edge), (int)roundf(x1), (int)roundf(y1), color.r, color.g, color.b);
        } else {
            float x1_virtual = x1 + w;
            float t = ((float)w - x0) / (x1_virtual - x0);
            float y_edge = y0 + t * (y1 - y0);
            draw_line_rgb(img, (int)roundf(x0), (int)roundf(y0), w - 1, (int)roundf(y_edge), color.r, color.g, color.b);
            draw_line_rgb(img, 0, (int)roundf(y_edge), (int)roundf(x1), (int)roundf(y1), color.r, color.g, color.b);
        }
    } else {
        draw_line_rgb(img, (int)roundf(x0), (int)roundf(y0), (int)roundf(x1), (int)roundf(y1), color.r, color.g, color.b);
    }
}

static void subdivide_and_draw_env(ImageRGB* img, Vec3 p0, Vec3 p1, float max_cos, RGB color) {
    float cos_theta = vec3_dot(p0, p1);
    if (cos_theta < max_cos) {
        Vec3 mid = vec3_add(p0, p1);
        float len = vec3_length(mid);
        if (len < 1e-6f) {
            // Opposite vectors. Pick a midpoint by using a perpendicular vector.
            Vec3 perp = (fabsf(p0.x) < 0.9f) ? (Vec3){1, 0, 0} : (Vec3){0, 1, 0};
            mid = vec3_cross(p0, perp);
        }
        mid = vec3_normalize(mid);
        subdivide_and_draw_env(img, p0, mid, max_cos, color);
        subdivide_and_draw_env(img, mid, p1, max_cos, color);
    } else {
        float x0, y0, x1, y1;
        project_vertex_env(p0, img->width, img->height, &x0, &y0);
        project_vertex_env(p1, img->width, img->height, &x1, &y1);
        draw_line_env_wrapped(img, x0, y0, x1, y1, color);
    }
}

void draw_line_subdivided(ImageRGB* img, Vec3 p0, Vec3 p1, bool env_map, 
                          Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, 
                          float tan_half_fov, float aspect, RGB color) {
    if (env_map) {
        // cos(2 degrees) = 0.99939
        subdivide_and_draw_env(img, p0, p1, 0.99939f, color);
    } else {
        float x0, y0, x1, y1;
        if (project_vertex(p0, cam_fwd, cam_up, cam_right, tan_half_fov, aspect, img->width, img->height, &x0, &y0) &&
            project_vertex(p1, cam_fwd, cam_up, cam_right, tan_half_fov, aspect, img->width, img->height, &x1, &y1)) {
            draw_line_rgb(img, (int)roundf(x0), (int)roundf(y0), (int)roundf(x1), (int)roundf(y1), color.r, color.g, color.b);
        }
    }
}

void draw_constellation_outlines(ImageRGB* img, ConstellationBoundary* boundary, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect, bool env_map, RGB color) {
    for (int i = 0; i < boundary->count - 1; i++) {
        ConstellationVertex* v0 = &boundary->vertices[i];
        ConstellationVertex* v1 = &boundary->vertices[i + 1];

        if (strcmp(v0->abbr, v1->abbr) != 0) continue;
        
        // Skip if both are below horizon
        if (v0->direction.y < 0 && v1->direction.y < 0) continue;

        Vec3 p0 = v0->direction;
        Vec3 p1 = v1->direction;

        // Clip to horizon (y=0)
        if (p0.y < 0 || p1.y < 0) {
            float t = p0.y / (p0.y - p1.y);
            Vec3 intersect = vec3_add(p0, vec3_mul(vec3_sub(p1, p0), t));
            intersect = vec3_normalize(intersect);
            if (p0.y < 0) p0 = intersect;
            else p1 = intersect;
        }

        draw_line_subdivided(img, p0, p1, env_map, cam_fwd, cam_up, cam_right, tan_half_fov, aspect, color);
    }
}

void draw_constellation_labels(ImageRGB* img, ConstellationBoundary* boundary, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect, bool env_map, RGB color) {
    for (int i = 0; i < boundary->label_count; i++) {
        ConstellationLabel* l = &boundary->labels[i];
        if (l->alt < 0) continue;

        float px, py;
        bool visible = false;
        if (env_map) {
            project_vertex_env(l->direction, img->width, img->height, &px, &py);
            visible = true;
        } else {
            visible = project_vertex(l->direction, cam_fwd, cam_up, cam_right, tan_half_fov, aspect, img->width, img->height, &px, &py);
        }

        if (visible) {
            if (py >= 0 && py < img->height) {
                draw_label_centered(img, (int)roundf(px), (int)roundf(py), l->abbr, color.r, color.g, color.b);
            }
        }
    }
}

void draw_char(ImageRGB* img, int x, int y, char c, float r, float g, float b) {
    if (c < 32 || c >= 127) return;
    const uint8_t* glyph = font8x8_basic[(int)c];
    if (!glyph) return;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (0x80 >> col)) {
                int px = x + col;
                int py = y + row;
                
                // Horizontal wrap
                px = (px % img->width + img->width) % img->width;
                
                if (py >= 0 && py < img->height) {
                    int idx = py * img->width + px;
                    img->pixels[idx].r = r;
                    img->pixels[idx].g = g;
                    img->pixels[idx].b = b;
                }
            }
        }
    }
}

void draw_label_centered(ImageRGB* img, int x, int y, const char* label, float r, float g, float b) {
    int len = (int)strlen(label);
    int total_width = len * 8;
    int start_x = x - total_width / 2;
    int start_y = y - 4;

    for (int i = 0; i < len; i++) {
        draw_char(img, start_x + i * 8, start_y, label[i], r, g, b);
    }
}

void draw_label_offset(ImageRGB* img, int x, int y, int offset_x, const char* label, RGB color) {
    int len = (int)strlen(label);
    int start_x = x + offset_x;
    int start_y = y - 4; // Vertically center 8x8 font

    for (int i = 0; i < len; i++) {
        draw_char(img, start_x + i * 8, start_y, label[i], color.r, color.g, color.b);
    }
}

void free_constellation_boundaries(ConstellationBoundary* boundary) {
    if (boundary->vertices) {
        free(boundary->vertices);
        boundary->vertices = NULL;
    }
    boundary->count = 0;
}