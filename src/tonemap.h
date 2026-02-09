#ifndef TONEMAP_H
#define TONEMAP_H

#include "core.h"

typedef struct {
    int width;
    int height;
    XYZV* pixels; // Raw spectral/XYZV data
} ImageHDR;

typedef struct {
    int width;
    int height;
    RGB* pixels;
} ImageRGB;

ImageHDR* image_hdr_create(int w, int h);
void image_hdr_free(ImageHDR* img);

ImageRGB* image_rgb_create(int w, int h);
void image_rgb_free(ImageRGB* img);

// Main post-processing pipeline
// 1. Blue Shift (XYZV -> XYZ modified)
// 2. Tone map (XYZ -> RGB)
// 3. Blur (optional)
void apply_night_post_processing(ImageHDR* src, ImageRGB* dst, float exposure_boost_stops);

// Applies a simple glare/bloom effect to bright pixels
void apply_glare(ImageHDR* img, float bloom_size_deg, float fov_deg);

#endif
