#include "atmosphere.h"
#include "atmosphere_math.h"

void atmosphere_init_default(Atmosphere* atm, float turbidity) {
    atm->earth_radius = EARTH_RADIUS;
    atm->atmosphere_radius = ATM_TOP;
    atm->rayleigh_scale_height = 8000.0f;
    atm->mie_scale_height = 1200.0f;
    atm->mie_g = 0.8f; // Strong forward scattering
    
    // Initialize Scattering Coefficients
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        float lambda = LAMBDA_START + i * LAMBDA_STEP;
        
        // Rayleigh
        float r_fac = powf(680.0f / lambda, 4.0f);
        atm->beta_rayleigh.s[i] = 5.8e-6f * r_fac;
        
        // Mie
        float m_fac = powf(550.0f / lambda, 1.3f);
        atm->beta_mie.s[i] = 2.0e-5f * m_fac * turbidity; 
    }
}

bool ray_sphere_intersect(Vec3 ray_origin, Vec3 ray_dir, float radius, float* t0, float* t1) {
    return ray_sphere_intersect_math(ray_origin, ray_dir, radius, t0, t1);
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
    return atmosphere_render_radiance(atm, ray_origin, ray_dir, sun_dir, sun_intensity, moon_dir, moon_intensity, out_alpha);
}

Spectrum atmosphere_transmittance(const Atmosphere* atm, Vec3 p, Vec3 dir) {
    return atmosphere_compute_transmittance(atm, p, dir);
}
