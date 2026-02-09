#include "tonemap.h"

ImageHDR* image_hdr_create(int w, int h) {
    ImageHDR* img = (ImageHDR*)malloc(sizeof(ImageHDR));
    img->width = w;
    img->height = h;
    img->pixels = (XYZV*)calloc(w * h, sizeof(XYZV));
    return img;
}

void image_hdr_free(ImageHDR* img) {
    if (img) {
        free(img->pixels);
        free(img);
    }
}

ImageRGB* image_rgb_create(int w, int h) {
    ImageRGB* img = (ImageRGB*)malloc(sizeof(ImageRGB));
    img->width = w;
    img->height = h;
    img->pixels = (RGB*)calloc(w * h, sizeof(RGB));
    return img;
}

void image_rgb_free(ImageRGB* img) {
    if (img) {
        free(img->pixels);
        free(img);
    }
}

static float smoothstep(float edge0, float edge1, float x) {
    x = (x - edge0) / (edge1 - edge0);
    if (x < 0.0f) x = 0.0f;
    if (x > 1.0f) x = 1.0f;
    return x * x * (3.0f - 2.0f * x);
}

void apply_night_post_processing(ImageHDR* src, ImageRGB* dst, float exposure_boost_stops) {
    int count = src->width * src->height;
    
    // 1. Calculate Log-Average Luminance for Auto-Exposure
    float sum_log_Y = 0;
    int valid_pixels = 0;
    float max_Y = 0;

    for (int i = 0; i < count; i++) {
        float Y = src->pixels[i].Y;
        // Ignore very dark pixels (like the artificial ground ~1e-8) to prevent skewing auto-exposure
        if (Y > 1e-6f) {
            sum_log_Y += logf(Y);
            valid_pixels++;
            if (Y > max_Y) max_Y = Y;
        }
    }
    
    float L_avg = (valid_pixels > 0) ? expf(sum_log_Y / valid_pixels) : 0.001f;
    
    // Clamp L_avg to a minimum floor to avoid over-exposing deep night
    if (L_avg < 1.0e-5f) L_avg = 1.0e-5f;

    printf("DEBUG: Scene L_avg: %e, MaxY: %e\n", L_avg, max_Y);
    
    // Key value: 0.18 is "middle grey". 
    // For night, we want it lower, but twilight needs something reasonable.
    // Let's use a key that scales slightly with brightness.
    float key = 0.18f;
    if (L_avg < 1.0e-4f) key = 0.05f; // Dimmer for very dark scenes

    // Apply exposure boost (f-stops)
    key *= powf(2.0f, exposure_boost_stops);
    
    // 2. Blue Shift (Mesopic)
    float xb = 0.25f;
    float yb = 0.25f;
    
    for (int i = 0; i < count; i++) {
        XYZV p = src->pixels[i];
        
        float Y = p.Y;
        if (Y <= 0) {
            dst->pixels[i] = (RGB){0,0,0};
            continue;
        }
        
        // Rod saturation s
        float logY = log10f(Y + 1e-9f);
        float s = smoothstep(-2.0f, 0.6f, logY);
        
        // Current chromaticity
        float xyz_sum = p.X + p.Y + p.Z;
        if (xyz_sum == 0) xyz_sum = 1.0f;
        float x = p.X / xyz_sum;
        float y = p.Y / xyz_sum;
        
        // Shift towards blue
        float x_new = (1.0f - s) * xb + s * x;
        float y_new = (1.0f - s) * yb + s * y;
        
        // Mix Luminance (Purkinje)
        float Y_mixed = 0.4468f * (1.0f - s) * p.V + s * Y;
        
        // Reconstruct XYZ
        if (y_new < 1e-4f) y_new = 1e-4f;
        float new_sum = Y_mixed / y_new;
        float X_final = x_new * new_sum;
        float Z_final = (1.0f - x_new - y_new) * new_sum;
        float Y_final = Y_mixed;
        
        // 3. Reinhard Tone Mapping
        // Scale by key/L_avg
        float L_scaled = (Y_final * key) / L_avg;
        
        // Simple Reinhard: L_d = L_s / (1 + L_s)
        // We use a white point to allow some burning
        float L_white = 1000.0f; 
        float Y_tonemapped = (L_scaled * (1.0f + L_scaled / (L_white * L_white))) / (1.0f + L_scaled);
        
        float scale = Y_tonemapped / (Y_final + 1e-9f);
        X_final *= scale;
        Y_final *= scale;
        Z_final *= scale;
        
        // 4. Linear to sRGB and Gamma
        RGB rgb = xyz_to_srgb(X_final, Y_final, Z_final);
        
        if (rgb.r < 0) rgb.r = 0;
        if (rgb.g < 0) rgb.g = 0;
        if (rgb.b < 0) rgb.b = 0;
        if (rgb.r > 1) rgb.r = 1;
        if (rgb.g > 1) rgb.g = 1;
        if (rgb.b > 1) rgb.b = 1;
        
        rgb.r = powf(rgb.r, 1.0f/2.2f);
        rgb.g = powf(rgb.g, 1.0f/2.2f);
        rgb.b = powf(rgb.b, 1.0f/2.2f);
        
        dst->pixels[i] = rgb;
    }
}

void apply_glare(ImageHDR* img, float bloom_size_deg, float fov_deg) {
    int w = img->width;
    int h = img->height;
    XYZV* temp = (XYZV*)calloc(w * h, sizeof(XYZV));
    
    // Copy original
    memcpy(temp, img->pixels, w * h * sizeof(XYZV));
    
    // Threshold to prevent glowing sky. 0.01 is bright enough for stars but low enough for consistency.
    float threshold = 0.01f; 
    
    // Sigma for Gaussian. Make it angularly consistent.
    float sigma = (bloom_size_deg / fov_deg) * w;
    if (sigma < 0.8f) sigma = 0.8f; // Minimum blur
    
    float inv_2sigma2 = 1.0f / (2.0f * sigma * sigma);
    
    // Spread factor (total energy redistributed)
    float spread_factor = 0.05f;
            
    int k_radius = (int)(sigma * 3.0f);
    if (k_radius < 2) k_radius = 2;
    if (k_radius > 10) k_radius = 10; // Cap for performance
            
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            XYZV p = temp[y * w + x];
            if (p.Y < threshold) continue; 
            
            for (int ky = -k_radius; ky <= k_radius; ky++) {
                for (int kx = -k_radius; kx <= k_radius; kx++) {
                    if (kx == 0 && ky == 0) continue; 
                    
                    int nx = x + kx;
                    int ny = y + ky;
                    
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        float dist2 = (float)(kx*kx + ky*ky);
                        float wgt = expf(-dist2 * inv_2sigma2);
                        
                        // Normalized weight approx
                        float s = wgt * spread_factor * (1.0f / (6.28f * sigma * sigma)); 
                        
                        XYZV* dest = &img->pixels[ny * w + nx];
                        dest->X += p.X * s;
                        dest->Y += p.Y * s;
                        dest->Z += p.Z * s;
                        dest->V += p.V * s;
                    }
                }
            }
        }
    }
    
    free(temp);
}
