#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../src/constellation.h"
#include "../src/ephemerides.h"

void test_safe_abbr_copy() {
    const char* temp_filename = "temp_constellation.txt";
    FILE* f = fopen(temp_filename, "w");
    if (!f) {
        perror("Failed to create temp file");
        exit(1);
    }
    // Write a line with a 3-letter abbreviation
    fprintf(f, "12.0 45.0 Ori\n");
    fclose(f);

    ConstellationBoundary boundary;
    // load_constellation_boundaries uses internal buffer for line, 
    // it should handle the RA, Dec and 3-char abbr.
    int result = load_constellation_boundaries(temp_filename, &boundary);
    assert(result == 0);
    assert(boundary.count == 1);
    
    ConstellationVertex* v = &boundary.vertices[0];
    printf("Loaded abbreviation: '%s'\n", v->abbr);
    assert(strcmp(v->abbr, "Ori") == 0);
    // Ensure null termination at index 3 (size is 4)
    assert(v->abbr[3] == '\0');

    free_constellation_boundaries(&boundary);
    remove(temp_filename);
    printf("test_safe_abbr_copy passed\n");
}

int main() {
    test_safe_abbr_copy();
    return 0;
}
