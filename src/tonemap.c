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

void apply_night_post_processing(ImageHDR* src, ImageRGB* dst) {
    int count = src->width * src->height;
    
    // 1. Blue Shift (Mesopic)
    // Target Scotopic Blue
    float xb = 0.25f;
    float yb = 0.25f;
    
    // Exposure
    float exposure = 1000.0f;
    
    for (int i = 0; i < count; i++) {
        XYZV p = src->pixels[i];
        
        float Y = p.Y;
        if (Y <= 1e-9f) {
            dst->pixels[i] = (RGB){0,0,0};
            continue;
        }
        
        float logY = log10f(Y);
        
        // Rod saturation s
        // -2.0 to 0.6
        float s = smoothstep(-2.0f, 0.6f, logY);
        
        // Current chromaticity
        float sum = p.X + p.Y + p.Z;
        if (sum == 0) sum = 1.0f;
        float x = p.X / sum;
        float y = p.Y / sum;
        
        // Shift towards blue
        float x_new = (1.0f - s) * xb + s * x;
        float y_new = (1.0f - s) * yb + s * y;
        
        // Mix Luminance
        // V is scotopic luminance.
        // Ymixed = 0.4468 * (1-s) * V + s * Y
        float Y_mixed = 0.4468f * (1.0f - s) * p.V + s * Y;
        
        // Reconstruct XYZ from x_new, y_new, Y_mixed
        if (y_new < 1e-4f) y_new = 1e-4f;
        float new_sum = Y_mixed / y_new;
        float X_final = x_new * new_sum;
        float Z_final = (1.0f - x_new - y_new) * new_sum;
        float Y_final = Y_mixed;
        
        // Tone Map
        X_final *= exposure;
        Y_final *= exposure;
        Z_final *= exposure;
        
        // Basic linear to sRGB
        RGB rgb = xyz_to_srgb(X_final, Y_final, Z_final);
        
        // Gamma correction
        if (rgb.r < 0) rgb.r = 0;
        if (rgb.g < 0) rgb.g = 0;
        if (rgb.b < 0) rgb.b = 0;
        
        rgb.r = powf(rgb.r, 1.0f/2.2f);
        rgb.g = powf(rgb.g, 1.0f/2.2f);
        rgb.b = powf(rgb.b, 1.0f/2.2f);
        
        dst->pixels[i] = rgb;
    }
}

void apply_glare(ImageHDR* img) {
    int w = img->width;
    int h = img->height;
    XYZV* temp = (XYZV*)calloc(w * h, sizeof(XYZV));
    
    // Copy original
    memcpy(temp, img->pixels, w * h * sizeof(XYZV));
    
    // Threshold to prevent glowing sky
    float threshold = 0.01f; 
    
    // Sigma for Gaussian
    float sigma = 0.8f;
    float inv_2sigma2 = 1.0f / (2.0f * sigma * sigma);
    
    // Spread factor (total energy redistributed)
    float spread_factor = 0.15f;
            
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            XYZV p = temp[y * w + x];
            if (p.Y < threshold) continue; 
            
            // 5x5 Gaussian spread
            for (int ky = -2; ky <= 2; ky++) {
                for (int kx = -2; kx <= 2; kx++) {
                    if (kx == 0 && ky == 0) continue; // Don't add back to self
                    
                    int nx = x + kx;
                    int ny = y + ky;
                    
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        float dist2 = (float)(kx*kx + ky*ky);
                        float wgt = expf(-dist2 * inv_2sigma2);
                        
                        // Normalized weight approx (sum of 5x5 gaussian excluding center is roughly 3.3 for sigma=0.8)
                        // We scale so total spread is spread_factor.
                        float s = wgt * spread_factor * 0.2f; 
                        
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
