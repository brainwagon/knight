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
    
    printf("Expected aperture 10.0, got %.1f\\n", cfg.aperture);
    assert(cfg.aperture == 10.0f);
    printf("test_parse_aperture passed\\n");
}

void test_parse_aperture_short() {
    Config cfg;
    cfg.aperture = 6.0f;
    
    char* argv[] = {"knight", "-A", "25.0"};
    int argc = 3;
    
    parse_args(argc, argv, &cfg);
    
    printf("Expected aperture 25.0, got %.1f\\n", cfg.aperture);
    assert(cfg.aperture == 25.0f);
    printf("test_parse_aperture_short passed\\n");
}

int main() {
    test_parse_aperture();
    test_parse_aperture_short();
    printf("All config tests passed!\\n");
    return 0;
}
