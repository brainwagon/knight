#include "core.h"
#include "ephemerides.h"
#include "stars.h"
#include "atmosphere.h"
#include "tonemap.h"
#include "image.h"
#include <getopt.h>
#include <time.h>
#include <strings.h>

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
    {"help",    no_argument,       0, '?'},
    {0, 0, 0, 0}
};

typedef struct {
    bool render_moon;
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
} Config;

void parse_args(int argc, char** argv, Config* cfg) {
    int opt;
    while ((opt = getopt_long(argc, argv, "l:L:d:t:a:z:f:w:h:o:cT:e:En", long_options, NULL)) != -1) {
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
            case '?': print_help(argv[0]); exit(0);
            default: break;
        }
    }
}

int main(int argc, char** argv) {
    Config cfg;
    cfg.render_moon = true;
    cfg.convert_to_png = false;
    cfg.track_body = NULL;
    
    // Default to current UTC time
    time_t now = time(NULL);
    struct tm* t = gmtime(&now);
    cfg.year = t->tm_year + 1900;
    cfg.month = t->tm_mon + 1;
    cfg.day = t->tm_mday;
    cfg.hour = t->tm_hour + t->tm_min / 60.0 + t->tm_sec / 3600.0;

    cfg.lat = 45.0; 
    cfg.lon = 0.0;
    cfg.cam_alt = 10.0f;
    cfg.cam_az = 270.0f;
    cfg.fov = 60.0f;
    cfg.width = 640;
    cfg.height = 480;
    cfg.exposure_boost = 0.0f;
    cfg.output_filename = "output.pfm";
    cfg.custom_cam = false;
    cfg.env_map = false;

    // 1. Process KNIGHT_OPTS environment variable
    char* env_opts = getenv("KNIGHT_OPTS");
    if (env_opts) {
        char* opts_copy = strdup(env_opts);
        int env_argc = 0;
        char* env_argv[64];
        env_argv[env_argc++] = argv[0]; // dummy program name
        
        char* token = strtok(opts_copy, " ");
        while (token && env_argc < 64) {
            env_argv[env_argc++] = token;
            token = strtok(NULL, " ");
        }
        
        optind = 1; // reset getopt
        parse_args(env_argc, env_argv, &cfg);
    }

    // 2. Process command line arguments
    optind = 1; // reset getopt
    parse_args(argc, argv, &cfg);

    printf("Initializing Knight Renderer...\n");
    printf("Resolution: %dx%d\n", cfg.width, cfg.height);
    if (!cfg.render_moon) printf("Option: Moon rendering DISABLED.\n");
    printf("Output file: %s\n", cfg.output_filename);
    printf("Exposure boost: %.1f stops\n", cfg.exposure_boost);
    
    Atmosphere atm;
    atmosphere_init_default(&atm);
    
    Image* moon_tex = NULL;
    if (cfg.render_moon) moon_tex = image_load_jpeg("data/moon_albedo.jpg");

    Star* stars = NULL;
    int num_stars = load_stars("data/ybsc5.dat", &stars);
    printf("Loaded %d stars.\n", num_stars);
    
    double jd = get_julian_day(cfg.year, cfg.month, cfg.day, cfg.hour);
    printf("Observer Location: Lat %.2f, Lon %.2f\n", cfg.lat, cfg.lon);
    printf("Simulation Time: %04d-%02d-%02d %02.2f UTC (JD %.2f)\n", cfg.year, cfg.month, cfg.day, cfg.hour, jd);
    
    double sunrise, sunset, astro_dawn, astro_dusk;
    sun_rise_set(jd, cfg.lat, cfg.lon, &sunrise, &sunset, &astro_dawn, &astro_dusk);
    if (astro_dawn >= 0) printf("Astro Dawn     : %02d:%02d UTC\n", (int)astro_dawn, (int)((astro_dawn - (int)astro_dawn) * 60));
    if (sunrise >= 0)    printf("Sunrise        : %02d:%02d UTC\n", (int)sunrise, (int)((sunrise - (int)sunrise) * 60));
    if (sunset >= 0)     printf("Sunset         : %02d:%02d UTC\n", (int)sunset, (int)((sunset - (int)sunset) * 60));
    if (astro_dusk >= 0) printf("Astro Dusk     : %02d:%02d UTC\n", (int)astro_dusk, (int)((astro_dusk - (int)astro_dusk) * 60));
    
    Vec3 sun_dir, moon_dir;
    sun_moon_position(jd, cfg.lat, cfg.lon, &sun_dir, &moon_dir);
    
    float s_alt = asinf(sun_dir.y) * RAD2DEG;
    float s_az = atan2f(sun_dir.x, sun_dir.z) * RAD2DEG;
    if (s_az < 0) s_az += 360.0f;
    printf("Sun Position : Alt %6.2f, Az %6.2f\n", s_alt, s_az);

    float m_alt = asinf(moon_dir.y) * RAD2DEG;
    float m_az = atan2f(moon_dir.x, moon_dir.z) * RAD2DEG;
    if (m_az < 0) m_az += 360.0f;
    printf("Moon Position: Alt %6.2f, Az %6.2f\n", m_alt, m_az);

    Planet planets[5];
    planets_position(jd, cfg.lat, cfg.lon, planets);
    for (int i=0; i<5; i++) {
        if (planets[i].alt > 0) {
            float p_az = planets[i].az * RAD2DEG;
            if (p_az < 0) p_az += 360.0f;
            printf("Planet %-8s: Alt %6.2f, Az %6.2f, Mag %5.1f\n", planets[i].name, planets[i].alt*RAD2DEG, p_az, planets[i].vmag);
        }
    }
    
    Spectrum sun_intensity;
    spectrum_set(&sun_intensity, 100.0f); 
    
    Spectrum moon_intensity;
    float moon_phase_factor = 1.0f;
    if (cfg.render_moon) {
        float cos_elong = vec3_dot(sun_dir, moon_dir);
        float alpha = acosf(-cos_elong);
        float a2 = alpha * 0.5f;
        float a4 = alpha * 0.25f;
        if (alpha < 0.01f) moon_phase_factor = 1.0f;
        else if (alpha > PI - 0.01f) moon_phase_factor = 0.0f;
        else moon_phase_factor = (1.0f - sinf(a2) * tanf(a2) * logf(1.0f/tanf(a4)));
        if (moon_phase_factor < 0) moon_phase_factor = 0;
        moon_intensity = sun_intensity;
        spectrum_mul(&moon_intensity, 1.0e-6f * moon_phase_factor); 
        printf("Moon Phase       : %s (Factor %.3f, Alpha %.1f deg)\n", get_moon_phase_name(jd), moon_phase_factor, alpha * RAD2DEG);
    } else spectrum_zero(&moon_intensity);
    
    ImageHDR* hdr = image_hdr_create(cfg.width, cfg.height);
    float aspect = (float)cfg.width / (float)cfg.height;
    float tan_half_fov = tanf(cfg.fov * 0.5f * DEG2RAD);
    
    Vec3 cam_pos = {0, EARTH_RADIUS + 10.0f, 0}; 
    Vec3 cam_forward;
    float final_cam_alt, final_cam_az;

    if (cfg.track_body) {
        bool found = false;
        if (strcasecmp(cfg.track_body, "sun") == 0) {
            final_cam_alt = s_alt;
            final_cam_az = s_az;
            cam_forward = sun_dir;
            found = true;
        } else if (strcasecmp(cfg.track_body, "moon") == 0) {
            if (!cfg.render_moon) printf("Warning: Tracking Moon but moon rendering is disabled.\n");
            final_cam_alt = m_alt;
            final_cam_az = m_az;
            cam_forward = moon_dir;
            found = true;
        } else {
            for (int i=0; i<5; i++) {
                if (strcasecmp(cfg.track_body, planets[i].name) == 0) {
                    final_cam_alt = planets[i].alt * RAD2DEG;
                    final_cam_az = planets[i].az * RAD2DEG;
                    if (final_cam_az < 0) final_cam_az += 360.0f;
                    cam_forward = planets[i].direction;
                    found = true;
                    break;
                }
            }
        }
        
        if (found) {
            printf("Tracking body: %s at Alt %.2f, Az %.2f\n", cfg.track_body, final_cam_alt, final_cam_az);
        } else {
            printf("Warning: Celestial body '%s' not found. Using defaults.\n", cfg.track_body);
            final_cam_alt = cfg.cam_alt;
            final_cam_az = cfg.cam_az;
            float rad_az = final_cam_az * DEG2RAD;
            float rad_alt = final_cam_alt * DEG2RAD;
            cam_forward.x = cosf(rad_alt) * sinf(rad_az);
            cam_forward.y = sinf(rad_alt);
            cam_forward.z = cosf(rad_alt) * cosf(rad_az);
        }
    } else if (!cfg.custom_cam && moon_dir.y > 0) {
        cam_forward = moon_dir; 
        final_cam_alt = m_alt;
        final_cam_az = m_az;
        printf("Tracking Moon position (default).\n");
    } else {
        final_cam_alt = cfg.cam_alt;
        final_cam_az = cfg.cam_az;
        float rad_az = final_cam_az * DEG2RAD;
        float rad_alt = final_cam_alt * DEG2RAD;
        cam_forward.x = cosf(rad_alt) * sinf(rad_az);
        cam_forward.y = sinf(rad_alt);
        cam_forward.z = cosf(rad_alt) * cosf(rad_az);
    }
    printf("Viewer Position: Alt %6.2f, Az %6.2f\n", final_cam_alt, final_cam_az);
    cam_forward = vec3_normalize(cam_forward);
    
    Vec3 world_up = {0, 1, 0};
    if (fabsf(cam_forward.y) > 0.99f) world_up = (Vec3){0, 0, 1};
    Vec3 cam_right = vec3_normalize(vec3_cross(world_up, cam_forward));
    Vec3 cam_up = vec3_cross(cam_forward, cam_right);
    
    printf("Rendering Atmosphere...\n");
    for (int y = 0; y < cfg.height; y++) {
        for (int x = 0; x < cfg.width; x++) {
            Vec3 dir;
            if (cfg.env_map) {
                float az_rad = (float)x / cfg.width * TWO_PI;
                float alt_rad = (0.5f - (float)y / cfg.height) * PI;
                dir.x = cosf(alt_rad) * sinf(az_rad);
                dir.y = sinf(alt_rad);
                dir.z = cosf(alt_rad) * cosf(az_rad);
            } else {
                float u = (2.0f * (x + 0.5f) / cfg.width - 1.0f) * aspect * tan_half_fov;
                float v = (1.0f - 2.0f * (y + 0.5f) / cfg.height) * tan_half_fov;
                dir = vec3_add(cam_forward, vec3_add(vec3_mul(cam_right, u), vec3_mul(cam_up, v)));
                dir = vec3_normalize(dir);
            }
            
            float alpha_atm = 1.0f;
            Spectrum L = atmosphere_render(&atm, cam_pos, dir, sun_dir, &sun_intensity, moon_dir, &moon_intensity, &alpha_atm);
            
            float t_e0, t_e1;
            if (ray_sphere_intersect(cam_pos, dir, EARTH_RADIUS, &t_e0, &t_e1)) {
                Spectrum ground;
                spectrum_set(&ground, 1.0e-8f); 
                spectrum_add(&L, &ground);
                alpha_atm = 0.0f; 
            }

            if (cfg.render_moon) {
                float cos_theta_moon = vec3_dot(dir, moon_dir);
                if (cos_theta_moon > 0.99999f && moon_dir.y > 0) {
                    Vec3 m_up_vec = {0, 1, 0};
                    if (fabsf(moon_dir.y) > 0.99f) m_up_vec = (Vec3){0, 0, 1};
                    Vec3 m_right = vec3_normalize(vec3_cross(m_up_vec, moon_dir));
                    Vec3 m_actual_up = vec3_cross(moon_dir, m_right);
                    float dx = vec3_dot(dir, m_right);
                    float dy = vec3_dot(dir, m_actual_up);
                    float dist = sqrtf(dx*dx + dy*dy) / 0.0045f;
                    if (dist <= 1.0f) {
                        float dz = sqrtf(1.0f - dist*dist);
                        Vec3 N = vec3_add(vec3_add(vec3_mul(m_right, dx/0.0045f), vec3_mul(m_actual_up, dy/0.0045f)), vec3_mul(moon_dir, -dz));
                        N = vec3_normalize(N);
                        float albedo = 0.12f;
                        if (moon_tex) albedo = image_sample_bilinear(moon_tex, (atan2f(N.x, N.z) + PI) / TWO_PI, acosf(N.y) / PI) * 0.2f;
                        float ndotl = vec3_dot(N, sun_dir);
                        if (ndotl < 0) ndotl = 0;
                        Spectrum moon_disk = sun_intensity;
                        // Add a small amount of earthshine (0.005) to the shadow side
                        spectrum_mul(&moon_disk, albedo * (ndotl + 0.005f) * alpha_atm);
                        spectrum_add(&L, &moon_disk);
                    }
                }
            }

            // Render Sun Disk
            float cos_theta_sun = vec3_dot(dir, sun_dir);
            if (cos_theta_sun > 0.99999f && sun_dir.y > -0.02f) {
                Spectrum sun_disk = sun_intensity;
                spectrum_mul(&sun_disk, alpha_atm);
                spectrum_add(&L, &sun_disk);
            }
            hdr->pixels[y * cfg.width + x] = spectrum_to_xyzv(&L);
        }
        if (y % 50 == 0) printf("Row %d\n", y);
    }
    
    if (num_stars > 0) {
        printf("Rendering Stars...\n");
        star_equ_to_horizon(jd, cfg.lat, cfg.lon, stars, num_stars);
        int on_screen_stars = 0;
        for (int i = 0; i < num_stars; i++) {
            Star s = stars[i];
            if (s.direction.y <= 0) continue; 
            float px, py;
            if (cfg.env_map) {
                float s_az = atan2f(s.direction.x, s.direction.z) * RAD2DEG;
                if (s_az < 0) s_az += 360.0f;
                px = (s_az / 360.0f) * cfg.width;
                py = (90.0f - asinf(s.direction.y) * RAD2DEG) / 180.0f * cfg.height;
            } else {
                float dz = vec3_dot(s.direction, cam_forward);
                if (dz <= 0) continue; 
                px = (vec3_dot(s.direction, cam_right) / dz / (aspect * tan_half_fov) + 1.0f) * 0.5f * cfg.width;
                py = (1.0f - vec3_dot(s.direction, cam_up) / dz / tan_half_fov) * 0.5f * cfg.height;
            }
            if (px >= 0 && px < cfg.width && py >= 0 && py < cfg.height) {
                float t0, t1;
                if (ray_sphere_intersect(cam_pos, s.direction, EARTH_RADIUS, &t0, &t1)) continue;
                on_screen_stars++;
                float solid_angle = cfg.env_map ? (TWO_PI/cfg.width)*(PI/cfg.height)*cosf(asinf(s.direction.y)) : (4.0f*tan_half_fov*tan_half_fov*aspect)/(cfg.width*cfg.height);
                float radiance = powf(10.0f, -0.4f * s.vmag) * 2.0e-5f / (solid_angle + 1e-12f);
                float T = expf(-0.1f / (s.direction.y + 0.01f)); 
                int idx = (int)py * cfg.width + (int)px;
                hdr->pixels[idx].Y += radiance * T; hdr->pixels[idx].X += radiance * T; 
                hdr->pixels[idx].Z += radiance * T; hdr->pixels[idx].V += radiance * T; 
            }
        }
        printf("Stars on screen: %d\n", on_screen_stars);
    }

    printf("Rendering Planets...\n");
    for (int i = 0; i < 5; i++) {
        Planet p = planets[i];
        if (p.alt <= 0) continue;
        float px, py;
        if (cfg.env_map) {
            float p_az_deg = atan2f(p.direction.x, p.direction.z) * RAD2DEG;
            if (p_az_deg < 0) p_az_deg += 360.0f;
            px = (p_az_deg / 360.0f) * cfg.width;
            py = (90.0f - p.alt*RAD2DEG) / 180.0f * cfg.height;
        } else {
            float dz = vec3_dot(p.direction, cam_forward);
            if (dz <= 0) continue; 
            px = (vec3_dot(p.direction, cam_right) / dz / (aspect * tan_half_fov) + 1.0f) * 0.5f * cfg.width;
            py = (1.0f - vec3_dot(p.direction, cam_up) / dz / tan_half_fov) * 0.5f * cfg.height;
        }
        if (px >= 0 && px < cfg.width && py >= 0 && py < cfg.height) {
            float t0, t1;
            if (ray_sphere_intersect(cam_pos, p.direction, EARTH_RADIUS, &t0, &t1)) continue;
            float solid_angle = cfg.env_map ? (TWO_PI/cfg.width)*(PI/cfg.height)*cosf(p.alt) : (4.0f*tan_half_fov*tan_half_fov*aspect)/(cfg.width*cfg.height);
            float radiance = powf(10.0f, -0.4f * p.vmag) * 2.0e-5f / (solid_angle + 1e-12f);
            float T = expf(-0.1f / (p.direction.y + 0.01f)); 
            int idx = (int)py * cfg.width + (int)px;
            hdr->pixels[idx].Y += radiance * T; hdr->pixels[idx].X += radiance * T; 
            hdr->pixels[idx].Z += radiance * T; hdr->pixels[idx].V += radiance * T; 
        }
    }
    
    printf("Tone Mapping...\n");
    apply_glare(hdr);
    ImageRGB* output = image_rgb_create(cfg.width, cfg.height);
    apply_night_post_processing(hdr, output, cfg.exposure_boost);
    write_pfm(cfg.output_filename, cfg.width, cfg.height, output->pixels);
    printf("Done. Saved to %s\n", cfg.output_filename);
    
    if (cfg.convert_to_png) {
        char png_filename[256];
        strncpy(png_filename, cfg.output_filename, sizeof(png_filename) - 1);
        png_filename[sizeof(png_filename) - 1] = '\0';
        char* last_dot = strrchr(png_filename, '.');
        if (last_dot) {
            *last_dot = '\0';
        }
        strcat(png_filename, ".png");
        
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "convert %s %s", cfg.output_filename, png_filename);
        printf("Converting to PNG: %s\n", cmd);
        if (system(cmd) != 0) {
            fprintf(stderr, "Error: ImageMagick conversion failed. Is 'convert' installed?\n");
        }
    }
    
    image_hdr_free(hdr); image_rgb_free(output); image_free(moon_tex); free(stars);
    return 0;
}
