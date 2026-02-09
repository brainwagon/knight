#include "stars.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

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

float bv_to_temp(float bv) {
    float term1 = 1.0f / (0.92f * bv + 1.7f);
    float term2 = 1.0f / (0.92f * bv + 0.62f);
    return 4600.0f * (term1 + term2);
}

int load_stars(const char* filepath, float mag_limit, Star** stars) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        perror("Error opening star catalog");
        return 0;
    }

    int max_stars = 10000;
    *stars = (Star*)malloc(sizeof(Star) * max_stars);
    if (!*stars) return 0;
    int count = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) < 114) continue;
        
        char vmag_str[6];
        memcpy(vmag_str, line + 102, 5); vmag_str[5] = '\0';
        float vmag = (float)atof(vmag_str);

        if (vmag > mag_limit) continue;
        
        char ra_h_str[3], ra_m_str[3], ra_s_str[5];
        memcpy(ra_h_str, line + 75, 2); ra_h_str[2] = '\0';
        memcpy(ra_m_str, line + 77, 2); ra_m_str[2] = '\0';
        memcpy(ra_s_str, line + 79, 4); ra_s_str[4] = '\0';
        float ra_h = (float)atof(ra_h_str);
        float ra_m = (float)atof(ra_m_str);
        float ra_s = (float)atof(ra_s_str);
        float ra_deg = (ra_h + ra_m/60.0f + ra_s/3600.0f) * 15.0f;
        
        char dec_s_str[2], dec_d_str[3], dec_m_str[3], dec_sec_str[3];
        memcpy(dec_s_str, line + 83, 1); dec_s_str[1] = '\0';
        memcpy(dec_d_str, line + 84, 2); dec_d_str[2] = '\0';
        memcpy(dec_m_str, line + 86, 2); dec_m_str[2] = '\0';
        memcpy(dec_sec_str, line + 88, 2); dec_sec_str[2] = '\0';
        float dec_d = (float)atof(dec_d_str);
        float dec_m = (float)atof(dec_m_str);
        float dec_s = (float)atof(dec_sec_str);
        float dec_deg = dec_d + dec_m/60.0f + dec_s/3600.0f;
        if (dec_s_str[0] == '-') dec_deg = -dec_deg;
        
        char bv_str[6];
        memcpy(bv_str, line + 109, 5); bv_str[5] = '\0';
        float bv = (float)atof(bv_str);
        
        Star* s = &(*stars)[count];
        s->id = count;
        s->vmag = vmag;
        s->bv = bv;
        s->ra = ra_deg * DEG2RAD;
        s->dec = dec_deg * DEG2RAD;
        
        count++;
        if (count >= max_stars) break;
    }
    fclose(f);
    return count;
}

void render_stars(const Star* stars, int num_stars, const RenderCamera* cam, float aperture, ImageHDR* hdr) {
    init_cie_cache();

    float sigmas_ang[SPECTRUM_BANDS];
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        float lambda_nm = 380.0f + i * 10.0f;
        float theta = 1.22f * (lambda_nm * 1e-9f) / (aperture * 1e-3f);
        sigmas_ang[i] = theta * 0.42f;
    }

    float pinhole_f_px = 0;
    float pinhole_sa = 0;
    if (!cam->env_map) {
        pinhole_f_px = cam->height / (2.0f * cam->tan_half_fov);
        pinhole_sa = (4.0f * cam->tan_half_fov * cam->tan_half_fov * cam->aspect) / (cam->width * cam->height);
    }

    for (int i = 0; i < num_stars; i++) {
        Star s = stars[i];
        if (s.direction.y <= 0) continue; 

        float px, py;
        if (cam->env_map) {
            float s_az = atan2f(s.direction.x, s.direction.z) * RAD2DEG;
            if (s_az < 0) s_az += 360.0f;
            px = (s_az / 360.0f) * cam->width;
            py = (90.0f - asinf(s.direction.y) * RAD2DEG) / 180.0f * cam->height;
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
        spectrum_mul(&spec, flux / (xyz_bb.Y + 1e-20f));

        float T = expf(-0.1f / (s.direction.y + 0.01f)); 
        spectrum_mul(&spec, T);

        float solid_angle = pinhole_sa;
        if (cam->env_map) {
            solid_angle = (6.283185f / cam->width) * (3.14159f / cam->height) * cosf(asinf(s.direction.y));
        }

        for (int b = 0; b < SPECTRUM_BANDS; b++) {
            float sigma_px;
            if (cam->env_map) {
                sigma_px = sigmas_ang[b] * cam->width / 6.283185f;
            } else {
                sigma_px = sigmas_ang[b] * pinhole_f_px;
            }
            
            // Perceptual minimum size to avoid sub-pixel invisibility
            if (sigma_px < 0.5f) sigma_px = 0.5f;

            int radius = (int)(sigma_px * 4.0f) + 1;
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
            float rad_factor_base = val_band / (solid_angle + 1e-15f);

            for (int iy = y_start; iy <= y_end; iy++) {
                for (int ix = x_start; ix <= x_end; ix++) {
                    float weight = integrate_gaussian_2d((float)ix - px, (float)iy - py, (float)ix + 1.0f - px, (float)iy + 1.0f - py, sigma_px);
                    float rad_factor = weight * rad_factor_base;
                    
                    int idx = iy * cam->width + ix;
                    hdr->pixels[idx].X += rad_factor * xyz_base.X;
                    hdr->pixels[idx].Y += rad_factor * xyz_base.Y;
                    hdr->pixels[idx].Z += rad_factor * xyz_base.Z;
                    hdr->pixels[idx].V += rad_factor * xyz_base.V;
                }
            }
        }
    }
}
