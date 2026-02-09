#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "core.h"

typedef struct {
    bool render_moon;
    bool render_outlines;
    bool convert_to_png;
    char* track_body;
    int year, month, day;
    double hour;
    double lat, lon;
    float cam_alt, cam_az, fov;
    int width, height;
    float exposure_boost;
    char* output_filename;
    bool custom_cam;
    bool env_map;
    float turbidity;
    char* mode;
    float aperture; // in mm
    bool bloom;
    float bloom_size; // in degrees
    RGB outline_color;
} Config;

void print_help(const char* progname);
void parse_args(int argc, char** argv, Config* cfg);

#endif
