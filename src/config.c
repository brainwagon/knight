#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

void print_help(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  -l, --lat <deg>      Observer latitude (default: 45.0)\n");
    printf("  -L, --lon <deg>      Observer longitude (default: 0.0)\n");
    printf("  -d, --date <Y-M-D>   Simulation date (default: today)\n");
    printf("  -t, --time <hour>    UTC hour (default: now)\n");
    printf("  -a, --alt <deg>      Viewer altitude (default: 10.0)\n");
    printf("  -z, --az <deg>       Viewer azimuth (0=N, 90=E, 180=S, 270=W, default: 270.0)\n");
    printf("  -f, --fov <deg>      Field of view (default: 60.0)\n");
    printf("  -w, --width <px>     Image width (default: 640)\n");
    printf("  -h, --height <px>    Image height (default: 480)\n");
    printf("  -o, --output <file>  Output filename (default: output.pfm)\n");
    printf("  -c, --convert        Convert PFM to PNG using ImageMagick\n");
    printf("  -T, --track <body|planet> Track celestial body (sun, moon, mercury, venus, mars, jupiter, saturn)\n");
    printf("  -e, --exposure <val> Exposure boost in f-stops (default: 0.0)\n");
    printf("  -E, --env            Generate cylindrical environment map\n");
    printf("  -n, --no-moon        Disable moon rendering\n");
    printf("  -O, --outline        Render constellation outlines\n");
    printf("      --outline-color <hex> Color for outlines (default: 00FF00)\n");
    printf("  -u, --turbidity <val> Atmospheric turbidity (Mie scattering multiplier, default: 1.0)\n");
    printf("  -A, --aperture <mm>  Observer aperture diameter in mm (default: 6.0)\n");
    printf("  -B, --bloom          Enable bloom/glare effect\n");
    printf("  -s, --bloom-size <deg> Bloom/glare size in degrees (default: 0.02)\n");
    printf("      --mode <cpu|gpu> Rendering mode (default: cpu)\n");
    printf("      --help           Show this help\n");
}

static struct option long_options[] = {
    {"lat",     required_argument, 0, 'l'},
    {"lon",     required_argument, 0, 'L'},
    {"date",    required_argument, 0, 'd'},
    {"time",    required_argument, 0, 't'},
    {"alt",     required_argument, 0, 'a'},
    {"az",      required_argument, 0, 'z'},
    {"fov",     required_argument, 0, 'f'},
    {"width",   required_argument, 0, 'w'},
    {"height",  required_argument, 0, 'h'},
    {"output",  required_argument, 0, 'o'},
    {"convert", no_argument,       0, 'c'},
    {"track",   required_argument, 0, 'T'},
    {"exposure",required_argument, 0, 'e'},
    {"env",     no_argument,       0, 'E'},
    {"no-moon", no_argument,       0, 'n'},
    {"outline", no_argument,       0, 'O'},
    {"outline-color", required_argument, 0, 'C'},
    {"turbidity", required_argument, 0, 'u'},
    {"aperture", required_argument, 0, 'A'},
    {"bloom",   no_argument,       0, 'B'},
    {"bloom-size", required_argument, 0, 's'},
    {"mode",    required_argument, 0, 'M'},
    {"help",    no_argument,       0, '?'},
    {0, 0, 0, 0}
};

void parse_args(int argc, char** argv, Config* cfg) {
    int opt;
    optind = 1;
    while ((opt = getopt_long(argc, argv, "l:L:d:t:a:z:f:w:h:o:cT:e:EnOu:A:Bs:C:M:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l': cfg->lat = atof(optarg); break;
            case 'L': cfg->lon = atof(optarg); break;
            case 'd': sscanf(optarg, "%d-%d-%d", &cfg->year, &cfg->month, &cfg->day); break;
            case 't': 
                if (strchr(optarg, ':')) {
                    int h = 0, m = 0;
                    float s = 0;
                    sscanf(optarg, "%d:%d:%f", &h, &m, &s);
                    cfg->hour = h + m / 60.0 + s / 3600.0;
                } else {
                    cfg->hour = atof(optarg); 
                }
                break;
            case 'a': cfg->cam_alt = atof(optarg); cfg->custom_cam = true; break;
            case 'z': cfg->cam_az = atof(optarg); cfg->custom_cam = true; break;
            case 'f': cfg->fov = atof(optarg); break;
            case 'w': cfg->width = atoi(optarg); break;
            case 'h': cfg->height = atoi(optarg); break;
            case 'o': cfg->output_filename = optarg; break;
            case 'c': cfg->convert_to_png = true; break;
            case 'T': cfg->track_body = optarg; cfg->custom_cam = true; break;
            case 'e': cfg->exposure_boost = atof(optarg); break;
            case 'E': cfg->env_map = true; break;
            case 'n': cfg->render_moon = false; break;
            case 'O': cfg->render_outlines = true; break;
            case 'C': {
                unsigned int hex = 0;
                if (optarg[0] == '#') sscanf(optarg + 1, "%x", &hex);
                else sscanf(optarg, "%x", &hex);
                cfg->outline_color.r = ((hex >> 16) & 0xFF) / 255.0f;
                cfg->outline_color.g = ((hex >> 8) & 0xFF) / 255.0f;
                cfg->outline_color.b = (hex & 0xFF) / 255.0f;
                break;
            }
            case 'u': cfg->turbidity = atof(optarg); break;
            case 'A': cfg->aperture = atof(optarg); break;
            case 'B': cfg->bloom = true; break;
            case 's': cfg->bloom_size = atof(optarg); break;
            case 'M': cfg->mode = optarg; break;
            case '?': print_help(argv[0]); exit(0);
            default: break;
        }
    }
}
