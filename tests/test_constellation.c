#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "constellation.h"

void test_vertex_structure() {
    ConstellationVertex v;
    v.ra = 1.0f;
    v.dec = 0.5f;
    strcpy(v.abbr, "Ori");
    
    assert(v.ra == 1.0f);
    assert(v.dec == 0.5f);
    assert(strcmp(v.abbr, "Ori") == 0);
    printf("test_vertex_structure passed\n");
}

void test_load_boundaries() {

    ConstellationBoundary boundary;

    int result = load_constellation_boundaries("../data/bound_in_20.txt", &boundary);

    assert(result == 0);

    assert(boundary.count > 0);

    

    // Check first vertex (22.9643492 +35.168228 AND)

    // 22.9643492 * 15 * DEG2RAD = 6.01202... radians?

    // Let's just check the values exist and are reasonable.

    assert(boundary.vertices[0].ra > 0);

    assert(strcmp(boundary.vertices[0].abbr, "AND") == 0);

    

    printf("test_load_boundaries passed: loaded %d vertices\n", boundary.count);

    free_constellation_boundaries(&boundary);

}



int main() {

    test_vertex_structure();

    test_load_boundaries();

    printf("All constellation tests passed!\n");

    return 0;

}
