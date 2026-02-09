#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "stars.h"
#include "image.h"
#include "tonemap.h"

void test_psf_resolution_independence() {
    Star s;
    s.direction = (Vec3){0, 0.5f, 0.866f}; // 30 deg altitude
    s.vmag = 0.0f;
    s.bv = 0.0f;
    s.id = 0;

    RenderCamera cam1;
    cam1.width = 100;
    cam1.height = 100;
    cam1.aspect = 1.0f;
    cam1.tan_half_fov = tanf(30.0f * 3.14159f / 180.0f);
    cam1.pos = (Vec3){0, 0, 0};
    cam1.forward = (Vec3){0, 0.5f, 0.866f};
    cam1.up = (Vec3){0, 0.866f, -0.5f};
    cam1.right = (Vec3){1, 0, 0};
    cam1.env_map = false;

    ImageHDR* hdr1 = image_hdr_create(100, 100);
    render_stars(&s, 1, &cam1, 6.0f, hdr1);

    float total_y1 = 0;
    for (int i = 0; i < 100 * 100; i++) total_y1 += hdr1->pixels[i].Y;

    RenderCamera cam2 = cam1;
    cam2.width = 200;
    cam2.height = 200;
    ImageHDR* hdr2 = image_hdr_create(200, 200);
    render_stars(&s, 1, &cam2, 6.0f, hdr2);

    float total_y2 = 0;
    for (int i = 0; i < 200 * 200; i++) total_y2 += hdr2->pixels[i].Y;

    printf("Total Y (100x100): %e\\n", (double)total_y1);
    printf("Total Y (200x200): %e\\n", (double)total_y2);
    
    assert(total_y1 > 0);
    float diff = fabsf(total_y1 - total_y2) / (total_y1 + 1e-20f);
    assert(diff < 0.01f);
    printf("test_psf_resolution_independence passed\\n");

    image_hdr_free(hdr1);
    image_hdr_free(hdr2);
}

int main() {
    test_psf_resolution_independence();
    return 0;
}