#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "core.h"

// We will implement this in core.c
// float integrate_gaussian_2d(float x0, float y0, float x1, float y1, float sigma);

void test_gaussian_integral() {
    // Over the whole plane, it should be 1.0
    float sigma = 1.0f;
    float large = 10.0f * sigma;
    float total = integrate_gaussian_2d(-large, -large, large, large, sigma);
    printf("Integral over large area: %f (expected ~1.0)\n", total);
    assert(fabsf(total - 1.0f) < 1e-4f);
    
    // Symmetry
    float half = integrate_gaussian_2d(0, -large, large, large, sigma);
    printf("Integral over half plane: %f (expected ~0.5)\n", half);
    assert(fabsf(half - 0.5f) < 1e-4f);
    
    // Quarter plane
    float quarter = integrate_gaussian_2d(0, 0, large, large, sigma);
    printf("Integral over quarter plane: %f (expected ~0.25)\n", quarter);
    assert(fabsf(quarter - 0.25f) < 1e-4f);
    
    printf("test_gaussian_integral passed\n");
}

int main() {
    test_gaussian_integral();
    printf("All math tests passed!\n");
    return 0;
}