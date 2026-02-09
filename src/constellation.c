#include "constellation.h"
#include <stdlib.h>

int load_constellation_boundaries(const char* filepath, ConstellationBoundary* boundary) {
    (void)filepath;
    (void)boundary;
    return -1; // Not implemented yet
}

void free_constellation_boundaries(ConstellationBoundary* boundary) {
    if (boundary->vertices) {
        free(boundary->vertices);
        boundary->vertices = NULL;
    }
    boundary->count = 0;
}
