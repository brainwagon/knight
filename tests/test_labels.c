#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "constellation.h"

void test_label_offset() {
    ImageRGB* img = image_rgb_create(100, 100);
    RGB red = {1.0f, 0.0f, 0.0f};
    
    // Draw "Sun" at 50, 50 with 8px offset
    // Target position: x = 50+8 = 58, y = 50-4 = 46 (for vertical centering of 8x8 font)
    draw_label_offset(img, 50, 50, 8, "Sun", red);
    
    // Check for pixels of 'S' (first char of Sun)
    // MSB of 'S' (0x3C = 00111100) at row 0 is empty.
    // 'S' row 1: 0x66 = 01100110. Bit 6 (col 1) and Bit 5 (col 2) should be set.
    // Pixel (58+1, 46+1) should be red.
    int px = 58 + 1;
    int py = 46 + 1;
    assert(img->pixels[py * 100 + px].r == 1.0f);
    
    image_rgb_free(img);
    printf("test_label_offset passed\n");
}

void test_label_clipping() {
    ImageRGB* img = image_rgb_create(100, 100);
    RGB red = {1.0f, 0.0f, 0.0f};
    
    // Draw label near right edge, should not crash
    draw_label_offset(img, 95, 50, 8, "Jupiter", red);
    
    // Draw label near bottom edge
    draw_label_offset(img, 50, 98, 8, "Moon", red);
    
    image_rgb_free(img);
    printf("test_label_clipping passed\n");
}

int main() {
    test_label_offset();
    test_label_clipping();
    printf("All label tests passed!\n");
    return 0;
}