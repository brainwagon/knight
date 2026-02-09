#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include <stdint.h>
#include <stdbool.h>
#include "core.h"
#include "image.h"
#include "tonemap.h"

// Vertex on a constellation boundary polygon
typedef struct {
    float ra;   // Right Ascension (radians)
    float dec;  // Declination (radians)
    char abbr[4]; // 3-letter abbreviation (e.g., "Ori")
    
    // Computed screen/horizon coordinates
    float az;   // Azimuth (radians)
    float alt;  // Altitude (radians)
    Vec3 direction; // Cartesian direction in horizon/world space
} ConstellationVertex;

// A label for a constellation at its centroid
typedef struct {
    float ra;   // Right Ascension (radians)
    float dec;  // Declination (radians)
    char abbr[4]; // 3-letter abbreviation
    
    // Computed screen/horizon coordinates
    float az;
    float alt;
    Vec3 direction;
} ConstellationLabel;

// A collection of vertices for all constellation boundaries
typedef struct {
    ConstellationVertex* vertices;
    int count;

    ConstellationLabel labels[88];
    int label_count;
} ConstellationBoundary;

// Load constellation boundaries from data/bound_in_20.txt
// Returns 0 on success, -1 on error.
int load_constellation_boundaries(const char* filepath, ConstellationBoundary* boundary);

// Transforms constellation vertex coordinates from Equatorial (RA/Dec) to Horizon (Alt/Az) and Cartesian direction.
void constellation_equ_to_horizon(double jd, double lat, double lon, ConstellationBoundary* boundary);

// Project a single vertex to screen coordinates. Returns true if in front of camera.
bool project_vertex(Vec3 v_dir, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect, int width, int height, float* px, float* py);

// Draw constellation outlines to the final image buffer (post-tonemapping)
void draw_constellation_outlines(ImageRGB* img, ConstellationBoundary* boundary, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect, RGB color);

// Draw constellation labels at their centroids
void draw_constellation_labels(ImageRGB* img, ConstellationBoundary* boundary, Vec3 cam_fwd, Vec3 cam_up, Vec3 cam_right, float tan_half_fov, float aspect, RGB color);

// Draw a single 8x8 character
void draw_char(ImageRGB* img, int x, int y, char c, float r, float g, float b);

// Draw a centered 3-letter abbreviation
void draw_label_centered(ImageRGB* img, int x, int y, const char* label, float r, float g, float b);

// Draw a label with an X offset, vertically centered
void draw_label_offset(ImageRGB* img, int x, int y, int offset_x, const char* label, RGB color);

// Free constellation boundaries
void free_constellation_boundaries(ConstellationBoundary* boundary);

#endif