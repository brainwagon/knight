#ifndef ATMOSPHERE_MATH_H
#define ATMOSPHERE_MATH_H

#include "atmosphere.h"

static inline HD bool ray_sphere_intersect_math(Vec3 ray_origin, Vec3 ray_dir, float radius, float* t0, float* t1) {
    // Solve |o + td|^2 = r^2
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

static inline HD float phase_rayleigh_math(float cos_theta) {
    return (3.0f / (16.0f * PI)) * (1.0f + cos_theta * cos_theta);
}

static inline HD float phase_mie_math(float cos_theta, float g) {
    float g2 = g * g;
    float denom = 1.0f + g2 - 2.0f * g * cos_theta;
    return (1.0f / (4.0f * PI)) * ((1.0f - g2) / powf(denom, 1.5f));
}

static inline HD void get_optical_depth_math(const Atmosphere* atm, Vec3 p, Vec3 dir, float dist, Spectrum* depth_r, Spectrum* depth_m) {
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

static inline HD Spectrum atmosphere_render_radiance(
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
    if (!ray_sphere_intersect_math(ray_origin, ray_dir, atm->atmosphere_radius, &t0, &t1)) {
        return result; 
    }
    
    float t_earth0, t_earth1;
    if (ray_sphere_intersect_math(ray_origin, ray_dir, atm->earth_radius, &t_earth0, &t_earth1)) {
        if (t_earth0 < t1) t1 = t_earth0;
    }
    
    int steps = 16;
    float dt = (t1 - t0) / steps;
    
    float mu_sun = vec3_dot(ray_dir, sun_dir);
    float mu_moon = vec3_dot(ray_dir, moon_dir);
    
    float pr_sun = phase_rayleigh_math(mu_sun);
    float pm_sun = phase_mie_math(mu_sun, atm->mie_g);
    
    float pr_moon = phase_rayleigh_math(mu_moon);
    float pm_moon = phase_mie_math(mu_moon, atm->mie_g);
    
    Spectrum tau_view_r; spectrum_zero(&tau_view_r);
    Spectrum tau_view_m; spectrum_zero(&tau_view_m);
    
    for (int i = 0; i < steps; i++) {
        float t = t0 + (i + 0.5f) * dt;
        Vec3 p = vec3_add(ray_origin, vec3_mul(ray_dir, t));
        float h = vec3_length(p) - atm->earth_radius;
        if (h < 0) h = 0;
        
        float rho_r = expf(-h / atm->rayleigh_scale_height);
        float rho_m = expf(-h / atm->mie_scale_height);
        
        float d_tau_r = rho_r * dt;
        float d_tau_m = rho_m * dt;
        
        Spectrum tau_sun_r, tau_sun_m;
        spectrum_zero(&tau_sun_r); spectrum_zero(&tau_sun_m);
        Spectrum tau_moon_r, tau_moon_m;
        spectrum_zero(&tau_moon_r); spectrum_zero(&tau_moon_m);
        
        float t_sun0 = 0, t_sun1 = 0;
        ray_sphere_intersect_math(p, sun_dir, atm->atmosphere_radius, &t_sun0, &t_sun1);
        get_optical_depth_math(atm, p, sun_dir, t_sun1, &tau_sun_r, &tau_sun_m);

        float t_moon0 = 0, t_moon1 = 0;
        ray_sphere_intersect_math(p, moon_dir, atm->atmosphere_radius, &t_moon0, &t_moon1);
        get_optical_depth_math(atm, p, moon_dir, t_moon1, &tau_moon_r, &tau_moon_m);
        
        for (int k = 0; k < SPECTRUM_BANDS; k++) {
            float T_sun = expf(-(tau_sun_r.s[k] + tau_sun_m.s[k]));
            float T_moon = expf(-(tau_moon_r.s[k] + tau_moon_m.s[k]));
            
            float current_view_tau = (tau_view_r.s[k] + d_tau_r * 0.5f) * atm->beta_rayleigh.s[k]
                                   + (tau_view_m.s[k] + d_tau_m * 0.5f) * atm->beta_mie.s[k];
            
            float T_view = expf(-current_view_tau);
            
            float beta_r = rho_r * atm->beta_rayleigh.s[k];
            float beta_m = rho_m * atm->beta_mie.s[k];
            
            float S_sun = (beta_r * pr_sun + beta_m * pm_sun) * sun_intensity->s[k] * T_sun;
            float S_moon = (beta_r * pr_moon + beta_m * pm_moon) * moon_intensity->s[k] * T_moon;
            
            result.s[k] += (S_sun + S_moon) * T_view * dt;
            
            tau_view_r.s[k] += d_tau_r;
            tau_view_m.s[k] += d_tau_m;
        }
    }
    
    int idx = 17;
    float final_tau = tau_view_r.s[idx] * atm->beta_rayleigh.s[idx] + tau_view_m.s[idx] * atm->beta_mie.s[idx];
    *out_alpha = expf(-final_tau);
    
    return result;
}

static inline HD Spectrum atmosphere_compute_transmittance(const Atmosphere* atm, Vec3 p, Vec3 dir) {
    Spectrum t;
    spectrum_set(&t, 1.0f);
    
    float t0, t1;
    if (ray_sphere_intersect_math(p, dir, atm->atmosphere_radius, &t0, &t1)) {
        float dist = t1;
        if (t1 < 0) dist = 0; 
        
        Spectrum depth_r, depth_m;
        spectrum_zero(&depth_r); spectrum_zero(&depth_m);
        get_optical_depth_math(atm, p, dir, dist, &depth_r, &depth_m);
        
        for (int i=0; i<SPECTRUM_BANDS; i++) {
             float tau = depth_r.s[i] + depth_m.s[i];
             t.s[i] = expf(-tau);
        }
    }
    return t;
}

#endif
