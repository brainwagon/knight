#ifndef STARS_H
#define STARS_H

#include "core.h"
#include "tonemap.h"

// Basic Star structure
typedef struct {
    int id;
    float ra;   // Right Ascension (radians)
    float dec;  // Declination (radians)
    float vmag; // Visual Magnitude
    float bv;   // B-V color index
    
    // Computed screen/horizon coordinates
    float az;   // Azimuth (radians)
    float alt;  // Altitude (radians)
    Vec3 direction; // Cartesian direction in horizon/world space
} Star;

// Load stars from the YBS catalog
// Returns number of stars loaded, or -1 on error.
// Caller is responsible for freeing *stars.
int load_stars(const char* filepath, Star** stars);

typedef struct {
    int width, height;
    float aspect;
    float tan_half_fov;
    Vec3 pos, forward, up, right;
    bool env_map;
} RenderCamera;

// Render stars to an HDR image using PSF
void render_stars(const Star* stars, int num_stars, const RenderCamera* cam, float aperture, ImageHDR* hdr);

#endif
