#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "config.h"

void test_parse_aperture() {
    Config cfg;
    cfg.aperture = 6.0f;
    
    char* argv[] = {"knight", "--aperture", "10.0"};
    int argc = 3;
    
    parse_args(argc, argv, &cfg);
    
    printf("Expected aperture 10.0, got %.1f\n", cfg.aperture);
    assert(cfg.aperture == 10.0f);
    printf("test_parse_aperture passed\n");
}

void test_parse_aperture_short() {
    Config cfg;
    cfg.aperture = 6.0f;
    
    char* argv[] = {"knight", "-A", "25.0"};
    int argc = 3;
    
    parse_args(argc, argv, &cfg);
    
    printf("Expected aperture 25.0, got %.1f\n", cfg.aperture);
    assert(cfg.aperture == 25.0f);
    printf("test_parse_aperture_short passed\n");
}

void test_parse_tycho() {
    Config cfg;
    cfg.use_tycho = false;
    cfg.tycho_dir = "tycho";
    cfg.star_mag_limit = 6.0f;

    char* argv[] = {"knight", "--tycho", "--tycho-dir", "data/tycho2", "-m", "8.5"};
    int argc = 6;

    parse_args(argc, argv, &cfg);

    printf("Expected use_tycho true, got %d\n", cfg.use_tycho);
    assert(cfg.use_tycho == true);
    printf("Expected tycho_dir data/tycho2, got %s\n", cfg.tycho_dir);
    assert(strcmp(cfg.tycho_dir, "data/tycho2") == 0);
    printf("Expected star_mag_limit 8.5, got %.1f\n", cfg.star_mag_limit);
    assert(cfg.star_mag_limit == 8.5f);
    printf("test_parse_tycho passed\n");
}

int main() {
    test_parse_aperture();
    test_parse_aperture_short();
    test_parse_tycho();
    printf("All config tests passed!\n");
    return 0;
}
