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

int main() {
    test_vertex_structure();
    printf("All constellation tests passed!\n");
    return 0;
}