#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stars.h"

// Mock YBS data with specific magnitudes
const char* test_ybs_file = "mock_ybsc5.dat";

void create_mock_ybs() {
    FILE* f = fopen(test_ybs_file, "w");
    // Line length >= 114
    // Vmag is at 102 (5 chars)
    // 1. Mag 2.0 (Bright)
    fprintf(f, "%-102s%5.2f%-10s\n", "Star1", 2.00, "...");
    // 2. Mag 5.0 (Borderline)
    fprintf(f, "%-102s%5.2f%-10s\n", "Star2", 5.00, "...");
    // 3. Mag 7.0 (Dim)
    fprintf(f, "%-102s%5.2f%-10s\n", "Star3", 7.00, "...");
    fclose(f);
}

void test_mag_filtering() {
    create_mock_ybs();
    Star* stars = NULL;
    
    // Test limit 6.0: Should load 2 stars (2.0 and 5.0)
    int num = load_stars(test_ybs_file, 6.0f, &stars);
    printf("Limit 6.0: Loaded %d stars (expected 2)\n", num);
    assert(num == 2);
    if (stars) { free(stars); stars = NULL; }
    
    // Test limit 4.0: Should load 1 star (2.0)
    num = load_stars(test_ybs_file, 4.0f, &stars);
    printf("Limit 4.0: Loaded %d stars (expected 1)\n", num);
    assert(num == 1);
    if (stars) { free(stars); stars = NULL; }

    // Test limit 1.0: Should load 0 stars
    num = load_stars(test_ybs_file, 1.0f, &stars);
    printf("Limit 1.0: Loaded %d stars (expected 0)\n", num);
    assert(num == 0);
    if (stars) { free(stars); stars = NULL; }
    
    remove(test_ybs_file);
    printf("test_mag_filtering passed\n");
}

int main() {
    test_mag_filtering();
    return 0;
}