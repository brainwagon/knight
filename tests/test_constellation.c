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

    

    // Count unique constellations

    char list[200][4];

    int unique_count = 0;

    for (int i = 0; i < boundary.count; i++) {

        int found = 0;

        for (int j = 0; j < unique_count; j++) {

            if (strcmp(boundary.vertices[i].abbr, list[j]) == 0) {

                found = 1;

                break;

            }

        }

        if (!found && unique_count < 200) {

            strcpy(list[unique_count], boundary.vertices[i].abbr);

            unique_count++;

        }

    }

    

    // There should be 88 constellations

    printf("test_load_boundaries passed: loaded %d vertices, %d constellations\n", boundary.count, unique_count);

    assert(unique_count == 88);

    

    free_constellation_boundaries(&boundary);

}





int main() {

    test_vertex_structure();

    test_load_boundaries();

    printf("All constellation tests passed!\n");

    return 0;

}
