#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "constellation.h"

void test_project_vertex_env() {
    printf("Testing project_vertex_env...\n");
    
    Vec3 v_north = {0, 0, 1}; // az=0, alt=0
    float px, py;
    project_vertex_env(v_north, 360, 180, &px, &py);
    assert(fabsf(px - 0.0f) < 1e-5f);
    assert(fabsf(py - 90.0f) < 1e-5f);

    Vec3 v_east = {1, 0, 0}; // az=90, alt=0
    project_vertex_env(v_east, 360, 180, &px, &py);
    assert(fabsf(px - 90.0f) < 1e-5f);
    assert(fabsf(py - 90.0f) < 1e-5f);

    Vec3 v_zenith = {0, 1, 0}; // alt=90
    project_vertex_env(v_zenith, 360, 180, &px, &py);
    assert(fabsf(py - 0.0f) < 1e-5f);

    printf("test_project_vertex_env passed\n");
}

void test_spherical_subdivision() {
    printf("Testing spherical_subdivision (via draw_line_subdivided)...\n");
    // We can't easily test pixels here without a full image buffer, 
    // but we can test if it doesn't crash and if the logic seems sound.
    ImageRGB* img = image_rgb_create(360, 180);
    RGB green = {0, 1, 0};
    
    Vec3 p0 = {0, 0, 1}; // 0 deg
    Vec3 p1 = {0, 0, -1}; // 180 deg (far away, will force subdivision)
    
    draw_line_subdivided(img, p0, p1, true, (Vec3){0,0,1}, (Vec3){0,1,0}, (Vec3){1,0,0}, 1.0f, 1.0f, green);
    
        image_rgb_free(img);
    
        printf("test_spherical_subdivision passed\n");
    
    }
    
    
    
    void test_meridian_wrap() {
    
        printf("Testing meridian wrap-around...\n");
    
        ImageRGB* img = image_rgb_create(100, 100);
    
        RGB red = {1, 0, 0};
    
        
    
        float rad0 = 359.0f * DEG2RAD;
    
        Vec3 p0 = {sinf(rad0), 0, cosf(rad0)};
    
        
    
        float rad1 = 1.0f * DEG2RAD;
    
        Vec3 p1 = {sinf(rad1), 0, cosf(rad1)};
    
        
    
        draw_line_subdivided(img, p0, p1, true, (Vec3){0,0,1}, (Vec3){0,1,0}, (Vec3){1,0,0}, 1.0f, 1.0f, red);
    
        
    
        bool found_left = false;
    
        bool found_right = false;
    
        for (int y=0; y<100; y++) {
    
            if (img->pixels[y*100 + 0].r > 0.5f) found_left = true;
    
            if (img->pixels[y*100 + 99].r > 0.5f) found_right = true;
    
        }
    
        
    
        assert(found_left && found_right);
    
        
    
            image_rgb_free(img);
    
        
    
            printf("test_meridian_wrap passed\n");
    
        
    
        }
    
        
    
        
    
        
    
        void test_label_wrap() {
    
        
    
            printf("Testing label wrap-around...\n");
    
        
    
            ImageRGB* img = image_rgb_create(100, 100);
    
        
    
            
    
        
    
            // Draw 'O' at x=98.
    
        
    
            // 'O' row 1 is 0x66 (01100110).
    
        
    
            // Bits set at col 1, 2, 5, 6 relative to start.
    
        
    
            // col 1: 98+1 = 99
    
        
    
            // col 2: 98+2 = 100 -> 0
    
        
    
            // col 5: 98+5 = 103 -> 3
    
        
    
            // col 6: 98+6 = 104 -> 4
    
        
    
            draw_char(img, 98, 50, 'O', 1, 0, 0);
    
        
    
            
    
        
    
            assert(img->pixels[51 * 100 + 99].r > 0.5f);
    
        
    
            assert(img->pixels[51 * 100 + 0].r > 0.5f);
    
        
    
            assert(img->pixels[51 * 100 + 3].r > 0.5f);
    
        
    
            assert(img->pixels[51 * 100 + 4].r > 0.5f);
    
        
    
            
    
        
    
            image_rgb_free(img);
    
        
    
            printf("test_label_wrap passed\n");
    
        
    
        }
    
        
    
        
    
        
    
        int main() {
    
        
    
            test_project_vertex_env();
    
        
    
            test_spherical_subdivision();
    
        
    
            test_meridian_wrap();
    
        
    
            test_label_wrap();
    
        
    
            printf("All env projection tests passed!\n");
    
        
    
            return 0;
    
        
    
        }
    
        
    
        
    
    