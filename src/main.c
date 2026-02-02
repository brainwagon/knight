#include "core.h"
#include "ephemerides.h"
#include "stars.h"
#include "atmosphere.h"
#include "tonemap.h"

#define WIDTH 1280
#define HEIGHT 960
#define FOV 60.0f

int main(int argc, char** argv) {
    bool render_moon = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-moon") == 0) {
            render_moon = false;
        }
    }

    printf("Initializing Knight Renderer...\n");
    if (!render_moon) {
        printf("Option: Moon rendering DISABLED.\n");
    }
    
    // 1. Setup Data
    Atmosphere atm;
    atmosphere_init_default(&atm);
    
    Star* stars = NULL;
    int num_stars = load_stars("data/ybsc5.dat", &stars);
    printf("Loaded %d stars.\n", num_stars);
    
    // 2. Setup Time/Location
    // 2026-02-17 18:15 UTC (Approx 30 mins after sunset)
    int year = 2026, month = 2, day = 17;
    double hour = 18.25; 
    double lat = 45.0; // 45 deg N
    double lon = 0.0;  // 0 deg E
    double jd = get_julian_day(year, month, day, hour);

    printf("Observer Location: Lat %.2f, Lon %.2f\n", lat, lon);
    printf("Simulation Time: %04d-%02d-%02d %02d:00 UTC (JD %.2f)\n", year, month, day, (int)hour, jd);
    
    Vec3 sun_dir, moon_dir;
    sun_moon_position(jd, lat, lon, &sun_dir, &moon_dir);
    
    printf("Sun Dir (Horizon): %.2f %.2f %.2f\n", sun_dir.x, sun_dir.y, sun_dir.z);
    printf("Sun Altitude: %.2f degrees\n", asinf(sun_dir.y) * RAD2DEG);
    
    printf("Moon Dir (Horizon): %.2f %.2f %.2f\n", moon_dir.x, moon_dir.y, moon_dir.z);
    printf("Moon Altitude: %.2f degrees\n", asinf(moon_dir.y) * RAD2DEG);

    Planet planets[5];
    planets_position(jd, lat, lon, planets);
    for (int i=0; i<5; i++) {
        if (planets[i].alt > 0) {
            printf("Planet %s: Alt %.1f, Az %.1f, Mag %.1f\n", planets[i].name, planets[i].alt*RAD2DEG, planets[i].az*RAD2DEG, planets[i].vmag);
        }
    }
    
    // Light Sources
    Spectrum sun_intensity;
    // blackbody_spectrum(5778.0f, &sun_intensity);
    spectrum_set(&sun_intensity, 100.0f); // High value
    
    Spectrum moon_intensity;
    float moon_phase_factor = 1.0f;
    if (render_moon) {
        // Calculate phase angle alpha (Sun-Moon-Earth angle)
        // elongation = angle between sun and moon from earth
        float cos_elong = vec3_dot(sun_dir, moon_dir);
        float alpha = acosf(-cos_elong); // phase angle 0 (full) to PI (new)
        
        // Lunar phase function f(alpha) from paper
        // f(alpha) = (1 - sin(alpha/2) * tan(alpha/2) * log(cot(alpha/4)))
        float a2 = alpha * 0.5f;
        float a4 = alpha * 0.25f;
        if (alpha < 0.01f) moon_phase_factor = 1.0f;
        else if (alpha > PI - 0.01f) moon_phase_factor = 0.0f;
        else {
            moon_phase_factor = (1.0f - sinf(a2) * tanf(a2) * logf(1.0f/tanf(a4)));
        }
        // Clamp
        if (moon_phase_factor < 0) moon_phase_factor = 0;

        moon_intensity = sun_intensity;
        // Base ratio ~1e-6 * phase factor
        spectrum_mul(&moon_intensity, 1.0e-6f * moon_phase_factor); 
        
        printf("Moon Phase Factor: %.3f (Alpha: %.1f deg)\n", moon_phase_factor, alpha * RAD2DEG);
    } else {
        spectrum_zero(&moon_intensity);
    }
    
    // 3. Render
    ImageHDR* hdr = image_hdr_create(WIDTH, HEIGHT);
    
    printf("Field of View: %.1f degrees\n", FOV);
    printf("Rendering Atmosphere...\n");
    float aspect = (float)WIDTH / (float)HEIGHT;
    float tan_half_fov = tanf(FOV * 0.5f * DEG2RAD);
    
    // Camera basis
    Vec3 cam_pos = {0, EARTH_RADIUS + 10.0f, 0}; 
    
    // Point West (Az = 270) at 10 deg altitude
    // x = cos(10)sin(270) = -0.9848
    // y = sin(10) = 0.1736
    // z = cos(10)cos(270) = 0
    Vec3 cam_forward = {-0.9848f, 0.1736f, 0.0f};
    cam_forward = vec3_normalize(cam_forward);
    printf("Looking West at 10 degrees altitude.\n");
    
    Vec3 world_up = {0, 1, 0};
    if (fabsf(cam_forward.y) > 0.99f) world_up = (Vec3){0, 0, 1};

    Vec3 cam_right = vec3_normalize(vec3_cross(world_up, cam_forward));
    Vec3 cam_up = vec3_cross(cam_forward, cam_right);
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Screen coords: y=0 is top (+v), y=HEIGHT-1 is bottom (-v)
            float u = (2.0f * (x + 0.5f) / WIDTH - 1.0f) * aspect * tan_half_fov;
            float v = (1.0f - 2.0f * (y + 0.5f) / HEIGHT) * tan_half_fov;
            
            // Ray Dir
            Vec3 dir = vec3_add(cam_forward, vec3_add(vec3_mul(cam_right, u), vec3_mul(cam_up, v)));
            dir = vec3_normalize(dir);
            
            float alpha_atm = 1.0f;
            Spectrum L = atmosphere_render(&atm, cam_pos, dir, sun_dir, &sun_intensity, moon_dir, &moon_intensity, &alpha_atm);
            
            // Ground Plane: If we hit earth, alpha_atm is set to 0 or we can check hit
            float t_e0, t_e1;
            if (ray_sphere_intersect(cam_pos, dir, EARTH_RADIUS, &t_e0, &t_e1)) {
                // Ground: Very dark grey/brown
                Spectrum ground;
                spectrum_set(&ground, 0.001f); 
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
            hdr->pixels[y * WIDTH + x] = xyzv;
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
            float px = (u / (aspect * tan_half_fov) + 1.0f) * 0.5f * WIDTH;
            float py = (1.0f - v / tan_half_fov) * 0.5f * HEIGHT;
            
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                // Check if ground occludes
                float t0, t1;
                if (ray_sphere_intersect(cam_pos, s.direction, EARTH_RADIUS, &t0, &t1)) continue;

                on_screen_stars++;
                float T = expf(-0.1f / (s.direction.y + 0.01f)); 
                
                float ang_w = (2.0f * tanf(FOV * 0.5f * DEG2RAD)) / WIDTH;
                float ang_h = (2.0f * tanf(FOV * 0.5f * DEG2RAD)) / (WIDTH / aspect);
                float solid_angle = ang_w * ang_h; 
                
                float flux = powf(10.0f, -0.4f * s.vmag) * 2.0e-5f; 
                float radiance = flux / solid_angle;

                int idx = (int)py * WIDTH + (int)px;
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
        
        float px = (u / (aspect * tan_half_fov) + 1.0f) * 0.5f * WIDTH;
        float py = (1.0f - v / tan_half_fov) * 0.5f * HEIGHT;
        
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
            float t0, t1;
            if (ray_sphere_intersect(cam_pos, p.direction, EARTH_RADIUS, &t0, &t1)) continue;

            float T = expf(-0.1f / (p.direction.y + 0.01f)); 
            float ang_w = (2.0f * tanf(FOV * 0.5f * DEG2RAD)) / WIDTH;
            float ang_h = (2.0f * tanf(FOV * 0.5f * DEG2RAD)) / (WIDTH / aspect);
            float solid_angle = ang_w * ang_h; 
            
            float flux = powf(10.0f, -0.4f * p.vmag) * 2.0e-5f; 
            float radiance = flux / solid_angle;

            int idx = (int)py * WIDTH + (int)px;
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
    
    ImageRGB* output = image_rgb_create(WIDTH, HEIGHT);
    apply_night_post_processing(hdr, output);
    
    // 6. Write
    write_pfm("output.pfm", WIDTH, HEIGHT, output->pixels);
    printf("Done. Saved to output.pfm\n");
    
    image_hdr_free(hdr);
    image_rgb_free(output);
    free(stars);
    
    return 0;
}
