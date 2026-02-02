#include "core.h"
#include "ephemerides.h"
#include "stars.h"
#include "atmosphere.h"
#include "tonemap.h"

#define FOV 60.0f

int main(int argc, char** argv) {
    bool render_moon = true;
    int year = 2026, month = 2, day = 17;
    double hour = 18.25; 
    double lat = 45.0; 
    double lon = 0.0;
    float cam_alt = 10.0f;
    float cam_az = 270.0f;
    float fov = 60.0f;
    int width = 640;
    int height = 480;
    bool custom_cam = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-moon") == 0) {
            render_moon = false;
        } else if (strcmp(argv[i], "--lat") == 0 && i + 1 < argc) {
            lat = atof(argv[++i]);
        } else if (strcmp(argv[i], "--lon") == 0 && i + 1 < argc) {
            lon = atof(argv[++i]);
        } else if (strcmp(argv[i], "--time") == 0 && i + 1 < argc) {
            hour = atof(argv[++i]);
        } else if (strcmp(argv[i], "--date") == 0 && i + 1 < argc) {
            sscanf(argv[++i], "%d-%d-%d", &year, &month, &day);
        } else if (strcmp(argv[i], "--alt") == 0 && i + 1 < argc) {
            cam_alt = atof(argv[++i]);
            custom_cam = true;
        } else if (strcmp(argv[i], "--az") == 0 && i + 1 < argc) {
            cam_az = atof(argv[++i]);
            custom_cam = true;
        } else if (strcmp(argv[i], "--fov") == 0 && i + 1 < argc) {
            fov = atof(argv[++i]);
        } else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            width = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            height = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --lat <deg>      Observer latitude (default: 45.0)\n");
            printf("  --lon <deg>      Observer longitude (default: 0.0)\n");
            printf("  --date <Y-M-D>   Simulation date (default: 2026-02-17)\n");
            printf("  --time <hour>    UTC hour (default: 18.25)\n");
            printf("  --alt <deg>      Viewer altitude (default: 10.0)\n");
            printf("  --az <deg>       Viewer azimuth (0=N, 90=E, 180=S, 270=W, default: 270.0)\n");
            printf("  --fov <deg>      Field of view (default: 60.0)\n");
            printf("  --width <px>     Image width (default: 640)\n");
            printf("  --height <px>    Image height (default: 480)\n");
            printf("  --no-moon        Disable moon rendering\n");
            return 0;
        }
    }

    printf("Initializing Knight Renderer...\n");
    printf("Resolution: %dx%d\n", width, height);
    
    // 1. Setup Data
    Atmosphere atm;
    atmosphere_init_default(&atm);
    
    Star* stars = NULL;
    int num_stars = load_stars("data/ybsc5.dat", &stars);
    printf("Loaded %d stars.\n", num_stars);
    
    // 2. Setup Time/Location
    double jd = get_julian_day(year, month, day, hour);

    printf("Observer Location: Lat %.2f, Lon %.2f\n", lat, lon);
    printf("Simulation Time: %04d-%02d-%02d %02.2f UTC (JD %.2f)\n", year, month, day, hour, jd);
    
    Vec3 sun_dir, moon_dir;
    sun_moon_position(jd, lat, lon, &sun_dir, &moon_dir);
    
    printf("Sun Dir (Horizon): %.2f %.2f %.2f (Alt: %.2f deg)\n", sun_dir.x, sun_dir.y, sun_dir.z, asinf(sun_dir.y) * RAD2DEG);
    printf("Moon Dir (Horizon): %.2f %.2f %.2f (Alt: %.2f deg)\n", moon_dir.x, moon_dir.y, moon_dir.z, asinf(moon_dir.y) * RAD2DEG);

    Planet planets[5];
    planets_position(jd, lat, lon, planets);
    for (int i=0; i<5; i++) {
        if (planets[i].alt > 0) {
            printf("Planet %s: Alt %.1f, Az %.1f, Mag %.1f\n", planets[i].name, planets[i].alt*RAD2DEG, planets[i].az*RAD2DEG, planets[i].vmag);
        }
    }
    
    // Light Sources
    Spectrum sun_intensity;
    spectrum_set(&sun_intensity, 100.0f); 
    
    Spectrum moon_intensity;
    float moon_phase_factor = 1.0f;
    if (render_moon) {
        float cos_elong = vec3_dot(sun_dir, moon_dir);
        float alpha = acosf(-cos_elong); 
        float a2 = alpha * 0.5f;
        float a4 = alpha * 0.25f;
        if (alpha < 0.01f) moon_phase_factor = 1.0f;
        else if (alpha > PI - 0.01f) moon_phase_factor = 0.0f;
        else {
            moon_phase_factor = (1.0f - sinf(a2) * tanf(a2) * logf(1.0f/tanf(a4)));
        }
        if (moon_phase_factor < 0) moon_phase_factor = 0;

        moon_intensity = sun_intensity;
        spectrum_mul(&moon_intensity, 1.0e-6f * moon_phase_factor); 
        printf("Moon Phase Factor: %.3f (Alpha: %.1f deg)\n", moon_phase_factor, alpha * RAD2DEG);
    } else {
        spectrum_zero(&moon_intensity);
    }
    
    // 3. Render
    ImageHDR* hdr = image_hdr_create(width, height);
    
    printf("Field of View: %.1f degrees\n", fov);
    printf("Rendering Atmosphere...\n");
    float aspect = (float)width / (float)height;
    float tan_half_fov = tanf(fov * 0.5f * DEG2RAD);
    
    // Camera basis
    Vec3 cam_pos = {0, EARTH_RADIUS + 10.0f, 0}; 
    Vec3 cam_forward;

    if (!custom_cam && moon_dir.y > 0) {
        cam_forward = moon_dir; 
        printf("Tracking Moon position.\n");
    } else {
        float rad_az = cam_az * DEG2RAD;
        float rad_alt = cam_alt * DEG2RAD;
        cam_forward.x = cosf(rad_alt) * sinf(rad_az);
        cam_forward.y = sinf(rad_alt);
        cam_forward.z = cosf(rad_alt) * cosf(rad_az);
        printf("Camera Direction: Az %.1f, Alt %.1f\n", cam_az, cam_alt);
    }
    cam_forward = vec3_normalize(cam_forward);
    
    Vec3 world_up = {0, 1, 0};
    if (fabsf(cam_forward.y) > 0.99f) world_up = (Vec3){0, 0, 1};

    Vec3 cam_right = vec3_normalize(vec3_cross(world_up, cam_forward));
    Vec3 cam_up = vec3_cross(cam_forward, cam_right);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Screen coords: y=0 is top (+v), y=height-1 is bottom (-v)
            float u = (2.0f * (x + 0.5f) / width - 1.0f) * aspect * tan_half_fov;
            float v = (1.0f - 2.0f * (y + 0.5f) / height) * tan_half_fov;
            
            // Ray Dir
            Vec3 dir = vec3_add(cam_forward, vec3_add(vec3_mul(cam_right, u), vec3_mul(cam_up, v)));
            dir = vec3_normalize(dir);
            
            float alpha_atm = 1.0f;
            Spectrum L = atmosphere_render(&atm, cam_pos, dir, sun_dir, &sun_intensity, moon_dir, &moon_intensity, &alpha_atm);
            
            // Ground Plane: If we hit earth, alpha_atm is set to 0 or we can check hit
            float t_e0, t_e1;
            if (ray_sphere_intersect(cam_pos, dir, EARTH_RADIUS, &t_e0, &t_e1)) {
                // Ground: Very dark
                Spectrum ground;
                spectrum_set(&ground, 1.0e-8f); 
                spectrum_add(&L, &ground);
                alpha_atm = 0.0f; // Ground occludes stars
            }

            // Draw Moon Disk
            if (render_moon) {
                float cos_theta_moon = vec3_dot(dir, moon_dir);
                if (cos_theta_moon > 0.99999f) { // approx cos(0.26 deg)
                       // Check if moon is above ground
                       if (moon_dir.y > 0) {
                           float dx = vec3_dot(dir, cam_right) - vec3_dot(moon_dir, cam_right);
                           float dy = vec3_dot(dir, cam_up) - vec3_dot(moon_dir, cam_up);
                           float dist = sqrtf(dx*dx + dy*dy) / 0.0045f;
                           if (dist <= 1.0f) {
                               float dz = sqrtf(1.0f - dist*dist);
                               float nx = dx / 0.0045f;
                               float ny = dy / 0.0045f;
                               float nz = dz;
                               Vec3 N = vec3_add(vec3_add(vec3_mul(cam_right, nx), vec3_mul(cam_up, ny)), vec3_mul(moon_dir, nz));
                               N = vec3_normalize(N);
                               float ndotl = vec3_dot(N, sun_dir);
                               if (ndotl < 0) ndotl = 0;
                               ndotl += 0.01f;
                               Spectrum moon_disk = sun_intensity;
                               spectrum_mul(&moon_disk, 0.12f * ndotl * alpha_atm);
                               spectrum_add(&L, &moon_disk);
                           }
                       }
                }
            }

            XYZV xyzv = spectrum_to_xyzv(&L);
            hdr->pixels[y * width + x] = xyzv;
        }
        if (y % 50 == 0) printf("Row %d\n", y);
    }
    
    // 4. Stars
    if (num_stars > 0) {
        printf("Rendering Stars...\n");
        star_equ_to_horizon(jd, lat, lon, stars, num_stars);
        
        int visible_stars = 0;
        int on_screen_stars = 0;

        for (int i = 0; i < num_stars; i++) {
            Star s = stars[i];
            if (s.direction.y <= 0) continue; 
            
            visible_stars++;

            float dx = vec3_dot(s.direction, cam_right);
            float dy = vec3_dot(s.direction, cam_up);
            float dz = vec3_dot(s.direction, cam_forward);
            
            if (dz <= 0) continue; 
            
            float u = dx / dz;
            float v = dy / dz;
            
            // Inverse of the top-down projection: y=0 is top (+v)
            float px = (u / (aspect * tan_half_fov) + 1.0f) * 0.5f * width;
            float py = (1.0f - v / tan_half_fov) * 0.5f * height;
            
            if (px >= 0 && px < width && py >= 0 && py < height) {
                // Check if ground occludes
                float t0, t1;
                if (ray_sphere_intersect(cam_pos, s.direction, EARTH_RADIUS, &t0, &t1)) continue;

                on_screen_stars++;
                float T = expf(-0.1f / (s.direction.y + 0.01f)); 
                
                float ang_w = (2.0f * tanf(fov * 0.5f * DEG2RAD)) / width;
                float ang_h = (2.0f * tanf(fov * 0.5f * DEG2RAD)) / (width / aspect);
                float solid_angle = ang_w * ang_h; 
                
                float flux = powf(10.0f, -0.4f * s.vmag) * 2.0e-5f; 
                float radiance = flux / solid_angle;

                int idx = (int)py * width + (int)px;
                hdr->pixels[idx].Y += radiance * T;
                hdr->pixels[idx].X += radiance * T; 
                hdr->pixels[idx].Z += radiance * T;
                hdr->pixels[idx].V += radiance * T; 
            }
        }
        printf("Stars: %d above horizon, %d on screen.\n", visible_stars, on_screen_stars);
    }

    // 4.5 Planets
    printf("Rendering Planets...\n");
    for (int i = 0; i < 5; i++) {
        Planet p = planets[i];
        if (p.alt <= 0) continue;

        float dx = vec3_dot(p.direction, cam_right);
        float dy = vec3_dot(p.direction, cam_up);
        float dz = vec3_dot(p.direction, cam_forward);
        
        if (dz <= 0) continue; 
        
        float u = dx / dz;
        float v = dy / dz;
        
        float px = (u / (aspect * tan_half_fov) + 1.0f) * 0.5f * width;
        float py = (1.0f - v / tan_half_fov) * 0.5f * height;
        
        if (px >= 0 && px < width && py >= 0 && py < height) {
            float t0, t1;
            if (ray_sphere_intersect(cam_pos, p.direction, EARTH_RADIUS, &t0, &t1)) continue;

            float T = expf(-0.1f / (p.direction.y + 0.01f)); 
            float ang_w = (2.0f * tanf(fov * 0.5f * DEG2RAD)) / width;
            float ang_h = (2.0f * tanf(fov * 0.5f * DEG2RAD)) / (width / aspect);
            float solid_angle = ang_w * ang_h; 
            
            float flux = powf(10.0f, -0.4f * p.vmag) * 2.0e-5f; 
            float radiance = flux / solid_angle;

            int idx = (int)py * width + (int)px;
            hdr->pixels[idx].Y += radiance * T;
            hdr->pixels[idx].X += radiance * T; 
            hdr->pixels[idx].Z += radiance * T;
            hdr->pixels[idx].V += radiance * T; 
        }
    }
    
    // 5. Tone Map
    printf("Tone Mapping...\n");
    printf("Applying Stellar Glare...\n");
    apply_glare(hdr);
    
    ImageRGB* output = image_rgb_create(width, height);
    apply_night_post_processing(hdr, output);
    
    // 6. Write
    write_pfm("output.pfm", width, height, output->pixels);
    printf("Done. Saved to output.pfm\n");
    
    image_hdr_free(hdr);
    image_rgb_free(output);
    free(stars);
    
    return 0;
}
