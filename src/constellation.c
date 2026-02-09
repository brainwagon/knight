#include "constellation.h"
#include "ephemerides.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int load_constellation_boundaries(const char* filepath, ConstellationBoundary* boundary) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;

    int capacity = 1024;
    boundary->vertices = (ConstellationVertex*)malloc(sizeof(ConstellationVertex) * capacity);
    boundary->count = 0;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (boundary->count >= capacity) {
            capacity *= 2;
            boundary->vertices = (ConstellationVertex*)realloc(boundary->vertices, sizeof(ConstellationVertex) * capacity);
        }

        float ra_h, dec_d;
        char abbr[4];
        if (sscanf(line, "%f %f %3s", &ra_h, &dec_d, abbr) == 3) {
            ConstellationVertex* v = &boundary->vertices[boundary->count];
            v->ra = ra_h * 15.0f * DEG2RAD; // Convert hours to radians
            v->dec = dec_d * DEG2RAD;        // Convert degrees to radians
            strncpy(v->abbr, abbr, 3);
            v->abbr[3] = '\0';
            boundary->count++;
        }
    }

    fclose(f);
    return 0;
}

void constellation_equ_to_horizon(double jd, double lat, double lon, ConstellationBoundary* boundary) {
    double gmst = greenwich_mean_sidereal_time(jd);
    double lmst = local_mean_sidereal_time(gmst, lon);
    double lat_rad = lat * DEG2RAD;

    for (int i = 0; i < boundary->count; i++) {
        ConstellationVertex* v = &boundary->vertices[i];
        
        double ha = lmst - v->ra;
        
        double sin_alt = sin(lat_rad) * sin(v->dec) + cos(lat_rad) * cos(v->dec) * cos(ha);
        double alt = asin(sin_alt);
        
        double cos_az = (sin(v->dec) - sin(alt) * sin(lat_rad)) / (cos(alt) * cos(lat_rad));
        // Clamp cos_az to [-1, 1] to avoid NaN from rounding errors
        if (cos_az > 1.0) cos_az = 1.0;
        if (cos_az < -1.0) cos_az = -1.0;
        
        double az = acos(cos_az);
        if (sin(ha) > 0) az = 2.0 * PI - az;
        
        v->az = (float)az;
        v->alt = (float)alt;
        
        // Cartesian direction in horizon space: Y=Up, Z=North, X=East
        v->direction.x = (float)(cos(alt) * sin(az));
        v->direction.y = (float)sin(alt);
        v->direction.z = (float)(cos(alt) * cos(az));
    }
}

void free_constellation_boundaries(ConstellationBoundary* boundary) {
    if (boundary->vertices) {
        free(boundary->vertices);
        boundary->vertices = NULL;
    }
    boundary->count = 0;
}