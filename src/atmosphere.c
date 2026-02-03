#include "atmosphere.h"

void atmosphere_init_default(Atmosphere* atm) {
    atm->earth_radius = EARTH_RADIUS;
    atm->atmosphere_radius = ATM_TOP;
    atm->rayleigh_scale_height = 8000.0f;
    atm->mie_scale_height = 1200.0f;
    atm->mie_g = 0.8f; // Strong forward scattering
    
    // Initialize Scattering Coefficients
    // Rayleigh ~ lambda^-4
    // Ref: Nishita 93 or Preetham
    // BetaR(lambda) = 5.8e-6 * (680 / lambda)^4  (if lambda in nm) -> wait, units.
    // 5.8e-6 m^-1 at 680nm.
    
    // Mie ~ lambda^-1.3 approx, or constant. BetaM(0) ~ 2e-5 usually?
    // Let's use 2e-5 at 550nm.
    
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        float lambda = LAMBDA_START + i * LAMBDA_STEP;
        
        // Rayleigh
        // (lambda/680)^-4
        float r_fac = powf(680.0f / lambda, 4.0f);
        atm->beta_rayleigh.s[i] = 5.8e-6f * r_fac;
        
        // Mie
        // BetaM = 4e-6; // Clear day?
        // (lambda/550)^-1.3?
        // Let's assume slight dependence
        // BetaM = 2e-5 * (550/lambda)^1.3 ?
        // Or constant 2.0e-5
        // Summary says "weak wavelength dependence".
        // Let's use 4e-6 to avoid too much haze for night.
        // Actually, night sky needs to be dark.
        // Let's use 1e-5.
        // atm->beta_mie.s[i] = 2e-5f; 
        float m_fac = powf(550.0f / lambda, 1.3f);
        atm->beta_mie.s[i] = 2.0e-5f * m_fac; 
    }
}

bool ray_sphere_intersect(Vec3 ray_origin, Vec3 ray_dir, float radius, float* t0, float* t1) {
    // Solve |o + td|^2 = r^2
    // t^2 + 2(o.d)t + o.o - r^2 = 0
    float a = 1.0f; // d is normalized
    float b = 2.0f * vec3_dot(ray_origin, ray_dir);
    float c = vec3_dot(ray_origin, ray_origin) - radius * radius;
    
    float disc = b*b - 4*a*c;
    if (disc < 0) return false;
    
    float sqrt_disc = sqrtf(disc);
    *t0 = (-b - sqrt_disc) / (2*a);
    *t1 = (-b + sqrt_disc) / (2*a);
    
    if (*t1 < 0) return false; // Both behind
    if (*t0 < 0) *t0 = 0; // Inside
    
    return true;
}

// Phase functions
static float phase_rayleigh(float cos_theta) {
    return (3.0f / (16.0f * PI)) * (1.0f + cos_theta * cos_theta);
}

static float phase_mie(float cos_theta, float g) {
    float g2 = g * g;
    float denom = 1.0f + g2 - 2.0f * g * cos_theta;
    return (1.0f / (4.0f * PI)) * ((1.0f - g2) / powf(denom, 1.5f));
}

// Optical depth calculation
static void get_optical_depth(const Atmosphere* atm, Vec3 p, Vec3 dir, float dist, Spectrum* depth_r, Spectrum* depth_m) {
    // Simple integration
    int steps = 8;
    float dt = dist / steps;
    float od_r = 0;
    float od_m = 0;
    
    for (int i = 0; i < steps; i++) {
        float t = (i + 0.5f) * dt;
        Vec3 sample_p = vec3_add(p, vec3_mul(dir, t));
        float h = vec3_length(sample_p) - atm->earth_radius;
        if (h < 0) h = 0;
        
        od_r += expf(-h / atm->rayleigh_scale_height) * dt;
        od_m += expf(-h / atm->mie_scale_height) * dt;
    }
    
    // Multiply by coeff
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        depth_r->s[i] = od_r * atm->beta_rayleigh.s[i];
        depth_m->s[i] = od_m * atm->beta_mie.s[i];
    }
}

Spectrum atmosphere_render(
    const Atmosphere* atm,
    Vec3 ray_origin,
    Vec3 ray_dir,
    Vec3 sun_dir,
    const Spectrum* sun_intensity,
    Vec3 moon_dir,
    const Spectrum* moon_intensity,
    float* out_alpha
) {
    Spectrum result;
    spectrum_zero(&result);
    *out_alpha = 0.0f;
    
    float t0, t1;
    if (!ray_sphere_intersect(ray_origin, ray_dir, atm->atmosphere_radius, &t0, &t1)) {
        // Space view?
        return result; 
    }
    
    // If we hit earth, clamp t1
    float t_earth0, t_earth1;
    if (ray_sphere_intersect(ray_origin, ray_dir, atm->earth_radius, &t_earth0, &t_earth1)) {
        if (t_earth0 < t1) t1 = t_earth0;
    }
    
    int steps = 16;
    float dt = (t1 - t0) / steps;
    
    // Cosines for phase
    float mu_sun = vec3_dot(ray_dir, sun_dir);
    float mu_moon = vec3_dot(ray_dir, moon_dir);
    
    float pr_sun = phase_rayleigh(mu_sun);
    float pm_sun = phase_mie(mu_sun, atm->mie_g);
    
    float pr_moon = phase_rayleigh(mu_moon);
    float pm_moon = phase_mie(mu_moon, atm->mie_g);
    
    // Accumulators
    // Actually we need to integrate L += (Beta * P * L_in * T_view) * dt
    
    // Current optical depth from camera
    Spectrum tau_view_r = {0};
    Spectrum tau_view_m = {0};
    
    for (int i = 0; i < steps; i++) {
        float t = t0 + (i + 0.5f) * dt;
        Vec3 p = vec3_add(ray_origin, vec3_mul(ray_dir, t));
        float h = vec3_length(p) - atm->earth_radius;
        if (h < 0) h = 0;
        
        float rho_r = expf(-h / atm->rayleigh_scale_height);
        float rho_m = expf(-h / atm->mie_scale_height);
        
        // Transmittance from camera to p
        // We can accumulate tau incrementally
        float d_tau_r = rho_r * dt;
        float d_tau_m = rho_m * dt;
        
        // Update view depth (center of segment approx)
        // Better: accumulate full dt, use half for this sample?
        // Let's just accumulate.
        
        // Transmittance to sun/moon (Light path)
        Spectrum tau_sun_r = {0}, tau_sun_m = {0};
        Spectrum tau_moon_r = {0}, tau_moon_m = {0};
        
        float t_sun0 = 0, t_sun1 = 0;
        ray_sphere_intersect(p, sun_dir, atm->atmosphere_radius, &t_sun0, &t_sun1);
        get_optical_depth(atm, p, sun_dir, t_sun1, &tau_sun_r, &tau_sun_m);

        float t_moon0 = 0, t_moon1 = 0;
        ray_sphere_intersect(p, moon_dir, atm->atmosphere_radius, &t_moon0, &t_moon1);
        get_optical_depth(atm, p, moon_dir, t_moon1, &tau_moon_r, &tau_moon_m);
        
        for (int k = 0; k < SPECTRUM_BANDS; k++) {
            // Light path transmittance
            float T_sun = expf(-(tau_sun_r.s[k] + tau_sun_m.s[k]));
            float T_moon = expf(-(tau_moon_r.s[k] + tau_moon_m.s[k]));
            
            // View path transmittance (accumulated so far)
            // Note: we add half step for p, or accumulate previous?
            // Simple accumulation:
            float current_view_tau = (tau_view_r.s[k] + d_tau_r * 0.5f) * atm->beta_rayleigh.s[k]
                                   + (tau_view_m.s[k] + d_tau_m * 0.5f) * atm->beta_mie.s[k];
            
            float T_view = expf(-current_view_tau);
            
            // Scattering Coeffs at p
            float beta_r = rho_r * atm->beta_rayleigh.s[k];
            float beta_m = rho_m * atm->beta_mie.s[k];
            
            // In-scattering
            // L_scat = (beta_r * Pr + beta_m * Pm) * L_in * T_view * dt
            
            float S_sun = (beta_r * pr_sun + beta_m * pm_sun) * sun_intensity->s[k] * T_sun;
            float S_moon = (beta_r * pr_moon + beta_m * pm_moon) * moon_intensity->s[k] * T_moon;
            
            result.s[k] += (S_sun + S_moon) * T_view * dt;
        }
        
        // Accumulate view optical depth
        // We need to multiply by beta.
        // Wait, beta is spectral.
        for (int k = 0; k < SPECTRUM_BANDS; k++) {
             tau_view_r.s[k] += d_tau_r; // Density integral
             tau_view_m.s[k] += d_tau_m;
        }
    }
    
    // Final alpha (Transmittance to space)
    // Use Green channel (approx index 20 -> 580nm or so) or Average
    // Index 17 (550nm).
    int idx = 17;
    float final_tau = tau_view_r.s[idx] * atm->beta_rayleigh.s[idx] + tau_view_m.s[idx] * atm->beta_mie.s[idx];
    *out_alpha = expf(-final_tau);
    
    return result;
}

Spectrum atmosphere_transmittance(const Atmosphere* atm, Vec3 p, Vec3 dir) {
    Spectrum t;
    spectrum_set(&t, 1.0f);
    
    float t0, t1;
    if (ray_sphere_intersect(p, dir, atm->atmosphere_radius, &t0, &t1)) {
        // We are usually inside, so we care about distance to exit (t1? or just ray length if we assume p is inside)
        // ray_sphere_intersect returns t0, t1 relative to ray origin.
        // If p is inside, t0 < 0, t1 > 0. Distance is t1.
        // If p is on ground, t0 ~ 0, t1 > 0.
        float dist = t1;
        if (t1 < 0) dist = 0; // Pointing down/away?
        
        Spectrum depth_r, depth_m;
        get_optical_depth(atm, p, dir, dist, &depth_r, &depth_m);
        
        for (int i=0; i<SPECTRUM_BANDS; i++) {
             float tau = depth_r.s[i] + depth_m.s[i];
             t.s[i] = expf(-tau);
        }
    }
    return t;
}
