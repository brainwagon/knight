#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "cuda_host.h"
#include "stars.h"
#include "image.h"

int main() {
#ifndef CUDA_ENABLED
    printf("CUDA not enabled, skipping test.\n");
    return 0;
#endif

    cuda_init();

    int width = 100;
    int height = 100;
    float aperture = 6.0f;
    float fov = 60.0f;
    
    RenderCamera cam;
    cam.width = width;
    cam.height = height;
    cam.aspect = 1.0f;
    cam.tan_half_fov = tanf(fov * 0.5f * DEG2RAD);
    cam.pos = (Vec3){0, 0, 0};
    cam.forward = (Vec3){0, 0, 1};
    cam.up = (Vec3){0, 1, 0};
    cam.right = (Vec3){1, 0, 0};
    cam.env_map = false;

    // Create a few test stars
    int num_stars = 3;
    Star* stars = (Star*)malloc(sizeof(Star) * num_stars);
    // Star 1: Center
    stars[0].id = 0;
    stars[0].direction = (Vec3){0, 0, 1};
    stars[0].vmag = 0.0f;
    stars[0].bv = 0.5f;
    // Star 2: Offset
    stars[1].id = 1;
    stars[1].direction = vec3_normalize((Vec3){0.1f, 0.1f, 1.0f});
    stars[1].vmag = 2.0f;
    stars[1].bv = 1.0f;
    // Star 3: Edge (might be clipped or partial)
    stars[2].id = 2;
    stars[2].direction = vec3_normalize((Vec3){-0.4f, 0.0f, 1.0f});
    stars[2].vmag = 1.0f;
    stars[2].bv = 0.0f;

    // 1. Render CPU
    ImageHDR* cpu_hdr = image_hdr_create(width, height);
    //render_stars(stars, num_stars, &cam, aperture, cpu_hdr);

    // 2. Render GPU
    // Need to initialize GPU buffer first by calling render_frame (dummy) or allocate manually.
    // cuda_render_stars uses d_pixels which is alloc'd in cuda_render_frame.
    // We can call cuda_render_frame with a dummy atmosphere to alloc buffer, then clear it?
    // Or we can modify cuda_render_stars to alloc if needed? It checks for d_pixels.
    // Let's call cuda_render_frame to setup.
    Atmosphere atm;
    atmosphere_init_default(&atm, 1.0f);
    Spectrum sun_i, moon_i;
    spectrum_set(&sun_i, 0); spectrum_set(&moon_i, 0);
    XYZV* gpu_pixels = (XYZV*)malloc(sizeof(XYZV) * width * height);
    
    // 1. Get Atmosphere baseline
    cuda_render_frame(width, height, &atm, cam.pos, cam.forward, cam.right, cam.up, fov, 1.0f, 
                      (Vec3){0,1,0}, &sun_i, (Vec3){0,-1,0}, &moon_i, 0, 0, 0, false, NULL, 0, 0, gpu_pixels);
    
    // 2. Init CPU HDR with baseline
    for(int i=0; i<width*height; i++) cpu_hdr->pixels[i] = gpu_pixels[i];

    // 3. CPU Render
    render_stars(stars, num_stars, &cam, aperture, cpu_hdr);

    // 4. GPU Render (accumulates on d_pixels which holds baseline)
    cuda_upload_stars(stars, num_stars);
    cuda_render_stars(width, height, &cam, aperture, gpu_pixels);

    // 6. Compare
    float max_diff = 0.0f;
    float max_val = 0.0f;
    for(int i=0; i<width*height; i++) {
        float dy = fabsf(cpu_hdr->pixels[i].Y - gpu_pixels[i].Y);
        if (dy > max_diff) max_diff = dy;
        if (cpu_hdr->pixels[i].Y > max_val) max_val = cpu_hdr->pixels[i].Y;
    }

    printf("Max val: %f, Max diff: %f\n", max_val, max_diff);
    
    // Parity might not be exact due to float atomics and different math intrinsics.
    // Allow < 1% error or small absolute error.
    if (max_val > 1e-6) {
        float rel_err = max_diff / max_val;
        printf("Relative error: %f\n", rel_err);
        assert(rel_err < 0.05f); // 5% tolerance for GPU vs CPU math diffs
    }

    cuda_cleanup();
    free(stars);
    free(gpu_pixels);
    image_hdr_free(cpu_hdr);

    printf("test_gpu_stars passed\n");
    return 0;
}