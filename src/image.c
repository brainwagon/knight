#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <stddef.h>
#include <math.h>

Image* image_load_jpeg(const char* filename) {
    FILE* infile = fopen(filename, "rb");
    if (!infile) {
        fprintf(stderr, "Error: could not open %s\n", filename);
        return NULL;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    Image* img = (Image*)malloc(sizeof(Image));
    img->width = cinfo.output_width;
    img->height = cinfo.output_height;
    img->channels = cinfo.output_components;
    img->data = (unsigned char*)malloc(img->width * img->height * img->channels);

    int row_stride = img->width * img->channels;
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char* buffer_array[1];
        buffer_array[0] = img->data + (cinfo.output_scanline * row_stride);
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    printf("Loaded texture %s (%dx%d, %d channels)\n", filename, img->width, img->height, img->channels);
    return img;
}

void image_free(Image* img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

static float get_pixel_val(const Image* img, int px, int py) {
    int idx = (py * img->width + px) * img->channels;
    if (img->channels == 1) {
        return img->data[idx] / 255.0f;
    } else {
        // Average first 3 channels (RGB)
        float sum = (float)img->data[idx] + (float)img->data[idx+1] + (float)img->data[idx+2];
        return (sum / 3.0f) / 255.0f;
    }
}

float image_sample_bilinear(const Image* img, float u, float v) {
    if (!img) return 0.12f;

    // Wrap u, clamp v
    u = fmodf(u, 1.0f);
    if (u < 0) u += 1.0f;
    if (v < 0) v = 0;
    if (v > 1) v = 1;

    float x = u * (img->width - 1);
    float y = v * (img->height - 1);

    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = (x0 + 1) % img->width;
    int y1 = (y0 + 1);
    if (y1 >= img->height) y1 = img->height - 1;

    float dx = x - (float)x0;
    float dy = y - (float)y0;

    float p00 = get_pixel_val(img, x0, y0);
    float p10 = get_pixel_val(img, x1, y0);
    float p01 = get_pixel_val(img, x0, y1);
    float p11 = get_pixel_val(img, x1, y1);

    return p00 * (1.0f - dx) * (1.0f - dy) + 
           p10 * dx * (1.0f - dy) + 
           p01 * (1.0f - dx) * dy + 
           p11 * dx * dy;
}