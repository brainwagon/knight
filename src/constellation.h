#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include "core.h"

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

// A collection of vertices for all constellation boundaries
typedef struct {
    ConstellationVertex* vertices;
    int count;
} ConstellationBoundary;

// Load constellation boundaries from data/bound_in_20.txt
// Returns 0 on success, -1 on error.
int load_constellation_boundaries(const char* filepath, ConstellationBoundary* boundary);

// Transforms constellation vertex coordinates from Equatorial (RA/Dec) to Horizon (Alt/Az) and Cartesian direction.
void constellation_equ_to_horizon(double jd, double lat, double lon, ConstellationBoundary* boundary);

// Free constellation boundaries
void free_constellation_boundaries(ConstellationBoundary* boundary);

#endif
