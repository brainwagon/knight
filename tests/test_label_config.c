#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

void test_parse_label_bodies() {
    Config cfg;
    cfg.label_bodies = false;
    
    char* argv[] = {"knight", "--label-bodies"};
    int argc = 2;
    
    parse_args(argc, argv, &cfg);
    
    assert(cfg.label_bodies == true);
    printf("test_parse_label_bodies passed\n");
}

void test_parse_label_bodies_short() {
    Config cfg;
    cfg.label_bodies = false;
    
    char* argv[] = {"knight", "-j"};
    int argc = 2;
    
    parse_args(argc, argv, &cfg);
    
    assert(cfg.label_bodies == true);
    printf("test_parse_label_bodies_short passed\n");
}

void test_parse_label_color() {
    Config cfg;
    cfg.label_color = (RGB){0, 0, 0};
    
    char* argv[] = {"knight", "--label-color", "00FF00"};
    int argc = 3;
    
    parse_args(argc, argv, &cfg);
    
    assert(cfg.label_color.r == 0.0f);
    assert(cfg.label_color.g == 1.0f);
    assert(cfg.label_color.b == 0.0f);
    printf("test_parse_label_color passed\n");
}

int main() {
    test_parse_label_bodies();
    test_parse_label_bodies_short();
    test_parse_label_color();
    printf("All label config tests passed!\n");
    return 0;
}