#include "stars.h"

static float parse_float(const char* line, int start, int len) {
    char buf[32];
    if (len >= 31) len = 31;
    memcpy(buf, line + start, len);
    buf[len] = '\0';
    int empty = 1;
    for (int i = 0; i < len; i++) {
        if (buf[i] != ' ') { empty = 0; break; }
    }
    if (empty) return NAN;
    return strtof(buf, NULL);
}

static int parse_int(const char* line, int start, int len) {
    char buf[32];
    if (len >= 31) len = 31;
    memcpy(buf, line + start, len);
    buf[len] = '\0';
    int empty = 1;
    for (int i = 0; i < len; i++) {
        if (buf[i] != ' ') { empty = 0; break; }
    }
    if (empty) return 0;
    return atoi(buf);
}

int load_stars(const char* filepath, Star** stars) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;
    
    int max_stars = 10000;
    *stars = (Star*)malloc(sizeof(Star) * max_stars);
    int count = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) < 114) continue;
        
        float vmag = parse_float(line, 102, 5);
        if (isnan(vmag)) continue;
        
        float bv = parse_float(line, 109, 5);
        if (isnan(bv)) bv = 0.0f;
        
        float rah = (float)parse_int(line, 75, 2);
        float ram = (float)parse_int(line, 77, 2);
        float ras = parse_float(line, 79, 4);
        if (isnan(ras)) ras = 0.0f;
        
        char de_sign = line[83];
        float ded = (float)parse_int(line, 84, 2);
        float dem = (float)parse_int(line, 86, 2);
        float des = (float)parse_int(line, 88, 2);
        
        float ra_deg = (rah + ram / 60.0f + ras / 3600.0f) * 15.0f;
        float dec_deg = ded + dem / 60.0f + des / 3600.0f;
        if (de_sign == '-') dec_deg = -dec_deg;
        
        Star* s = &(*stars)[count];
        s->id = count;
        s->vmag = vmag;
        s->bv = bv;
        s->ra = ra_deg * 3.14159f / 180.0f;
        s->dec = dec_deg * 3.14159f / 180.0f;
        
        count++;
        if (count >= max_stars) break;
    }
    
    fclose(f);
    return count;
}

// Approximate blackbody temperature from B-V index
float bv_to_temp(float bv) {
    float term1 = 1.0f / (0.92f * bv + 1.7f);
    float term2 = 1.0f / (0.92f * bv + 0.62f);
    return 4600.0f * (term1 + term2);
}

// Cached XYZ matching values multiplied by dLambda
static XYZV CIE_XYZV_weighted[SPECTRUM_BANDS];
static bool cie_cached = false;

static void init_cie_cache() {
    if (cie_cached) return;
    Spectrum s;
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        spectrum_zero(&s);
        s.s[i] = 1.0f;
        CIE_XYZV_weighted[i] = spectrum_to_xyzv(&s);
    }
    cie_cached = true;
}

void render_stars(const Star* stars, int num_stars, const RenderCamera* cam, float aperture, ImageHDR* hdr) {
    init_cie_cache();

    float sigmas_ang[SPECTRUM_BANDS];
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        float lambda_nm = 380.0f + i * 10.0f;
        float theta = 1.22f * (lambda_nm * 1e-9f) / (aperture * 1e-3f);
        sigmas_ang[i] = theta * 0.42f;
    }

    for (int i = 0; i < num_stars; i++) {
        Star s = stars[i];
        if (s.direction.y <= 0) continue; 

        float px, py;
        if (cam->env_map) {
            float s_az = atan2f(s.direction.x, s.direction.z) * 180.0f / 3.14159f;
            if (s_az < 0) s_az += 360.0f;
            px = (s_az / 360.0f) * cam->width;
            py = (90.0f - asinf(s.direction.y) * 180.0f / 3.14159f) / 180.0f * cam->height;
        } else {
            float dz = vec3_dot(s.direction, cam->forward);
            if (dz <= 0) continue; 
            px = (vec3_dot(s.direction, cam->right) / dz / (cam->aspect * cam->tan_half_fov) + 1.0f) * 0.5f * cam->width;
            py = (1.0f - vec3_dot(s.direction, cam->up) / dz / cam->tan_half_fov) * 0.5f * cam->height;
        }

        if (px < -20 || px >= cam->width + 20 || py < -20 || py >= cam->height + 20) continue;

        Spectrum spec;
        blackbody_spectrum(bv_to_temp(s.bv), &spec);
        XYZV xyz_bb = spectrum_to_xyzv(&spec);
        float flux = powf(10.0f, -0.4f * s.vmag) * 2.0e-5f;
        spectrum_mul(&spec, flux / (xyz_bb.V + 1e-20f));

        float T = expf(-0.1f / (s.direction.y + 0.01f)); 
        spectrum_mul(&spec, T);

        for (int b = 0; b < SPECTRUM_BANDS; b++) {
            float sigma_px;
            if (cam->env_map) {
                sigma_px = sigmas_ang[b] * cam->width / 6.283185f;
            } else {
                float f_px = cam->height / (2.0f * cam->tan_half_fov);
                sigma_px = sigmas_ang[b] * f_px;
            }
            
            if (sigma_px < 0.01f) sigma_px = 0.01f;

            int radius = (int)(sigma_px * 5.0f) + 1;
            int x_start = (int)px - radius;
            int x_end = (int)px + radius;
            int y_start = (int)py - radius;
            int y_end = (int)py + radius;

            if (x_start < 0) x_start = 0;
            if (x_end >= cam->width) x_end = cam->width - 1;
            if (y_start < 0) y_start = 0;
            if (y_end >= cam->height) y_end = cam->height - 1;

            float val_band = spec.s[b];
            XYZV xyz_base = CIE_XYZV_weighted[b];
            
            for (int iy = y_start; iy <= y_end; iy++) {
                for (int ix = x_start; ix <= x_end; ix++) {
                    float x0 = (float)ix - px;
                    float x1 = (float)ix + 1.0f - px;
                    float y0 = (float)iy - py;
                    float y1 = (float)iy + 1.0f - py;
                    
                    float weight = integrate_gaussian_2d(x0, y0, x1, y1, sigma_px);
                    float energy = val_band * weight;
                    
                    int idx = iy * cam->width + ix;
                    hdr->pixels[idx].X += energy * xyz_base.X;
                    hdr->pixels[idx].Y += energy * xyz_base.Y;
                    hdr->pixels[idx].Z += energy * xyz_base.Z;
                    hdr->pixels[idx].V += energy * xyz_base.V;
                }
            }
        }
    }
}