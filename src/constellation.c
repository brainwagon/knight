#include "constellation.h"
#include <stdlib.h>

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

void free_constellation_boundaries(ConstellationBoundary* boundary) {
    if (boundary->vertices) {
        free(boundary->vertices);
        boundary->vertices = NULL;
    }
    boundary->count = 0;
}
