#include "constellation.h"
#include "ephemerides.h"
#include "font8x8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
        double sum_ra = 0, sum_dec = 0;
        int vertex_count = 0;

        for (int i = 0; i <= boundary->count; i++) {
            bool end_of_const = (i == boundary->count) || (vertex_count > 0 && strcmp(boundary->vertices[i].abbr, current_abbr) != 0);

            if (end_of_const) {
                if (boundary->label_count < 88) {
                    ConstellationLabel* l = &boundary->labels[boundary->label_count++];
                    l->ra = (float)(sum_ra / vertex_count);
                    l->dec = (float)(sum_dec / vertex_count);
                    strncpy(l->abbr, current_abbr, 3);
                    l->abbr[3] = '\0';
                }
                if (i == boundary->count) break;
                sum_ra = 0; sum_dec = 0; vertex_count = 0;
            }

            strcpy(current_abbr, boundary->vertices[i].abbr);
            sum_ra += boundary->vertices[i].ra;
            sum_dec += boundary->vertices[i].dec;
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

static void draw_line(Image* img, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < img->width && y0 >= 0 && y0 < img->height) {
            int idx = (y0 * img->width + x0) * 3;
            img->data[idx + 0] = r;
            img->data[idx + 1] = g;
            img->data[idx + 2] = b;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_constellation_outlines(Image* img, ConstellationBoundary* boundary, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect) {
    for (int i = 0; i < boundary->count - 1; i++) {
        ConstellationVertex* v0 = &boundary->vertices[i];
        ConstellationVertex* v1 = &boundary->vertices[i + 1];

        // Only draw if same constellation
        if (strcmp(v0->abbr, v1->abbr) != 0) continue;

        // Skip if both below horizon
        if (v0->alt < 0 && v1->alt < 0) continue;

        float x0, y0, x1, y1;
        if (project_vertex(v0->direction, cam_fwd, cam_up, cam_right, tan_half_fov, aspect, img->width, img->height, &x0, &y0) &&
            project_vertex(v1->direction, cam_fwd, cam_up, cam_right, tan_half_fov, aspect, img->width, img->height, &x1, &y1)) {
            
            // Subtle blue-grey for outlines
            draw_line(img, (int)x0, (int)y0, (int)x1, (int)y1, 60, 70, 90);
        }
    }
}

void draw_char(Image* img, int x, int y, char c, uint8_t r, uint8_t g, uint8_t b) {
    if (c < 32 || c >= 127) return;
    const uint8_t* glyph = font8x8_basic[(int)c];
    if (!glyph) return;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (glyph[row] & (1 << col)) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                    int idx = (py * img->width + px) * 3;
                    img->data[idx + 0] = r;
                    img->data[idx + 1] = g;
                    img->data[idx + 2] = b;
                }
            }
        }
    }
}

void draw_label_centered(Image* img, int x, int y, const char* label, uint8_t r, uint8_t g, uint8_t b) {
    int len = (int)strlen(label);
    int total_width = len * 8;
    int start_x = x - total_width / 2;
    int start_y = y - 4; // Center of 8x8 font

    for (int i = 0; i < len; i++) {
        draw_char(img, start_x + i * 8, start_y, label[i], r, g, b);
    }
}

void free_constellation_boundaries(ConstellationBoundary* boundary) {
    if (boundary->vertices) {
        free(boundary->vertices);
        boundary->vertices = NULL;
    }
    boundary->count = 0;
}