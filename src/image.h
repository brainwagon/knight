#ifndef IMAGE_H
#define IMAGE_H

#include "core.h"

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char* data;
} Image;

Image* image_load_jpeg(const char* filename);
void image_free(Image* img);

// Sample image using bilinear interpolation, uv in [0, 1]
// Returns a grey value or RGB? The paper uses spectral albedo maps.
// For now, let's return a float (grey) or RGB if needed.
// Since moon_albedo.jpg is likely grayscale or RGB, let's return a Spectrum or float.
float image_sample_bilinear(const Image* img, float u, float v);

#endif
