#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include "core.h"

// Math constants
#define EARTH_RADIUS 6360000.0f
#define ATM_TOP 6440000.0f // 80km atmosphere

typedef struct {
    float rayleigh_scale_height; // e.g. 8000 m
    float mie_scale_height;      // e.g. 1200 m
    Spectrum beta_rayleigh;      // Scattering Coeffs at sea level
    Spectrum beta_mie;           // Scattering Coeffs at sea level
    float mie_g;                 // Henyey-Greenstein g
    float earth_radius;          // 6360 km
    float atmosphere_radius;     // 6420 km
} Atmosphere;

// Setup default Earth atmosphere
void atmosphere_init_default(Atmosphere* atm);

// Computes intersection distances with a sphere
// Returns true if hit. t0 is near, t1 is far.
bool ray_sphere_intersect(Vec3 ray_origin, Vec3 ray_dir, float radius, float* t0, float* t1);

// Ray marches the atmosphere
// ray_origin: camera position relative to earth center (e.g. (0, R_earth + 1m, 0))
// ray_dir: normalized direction
// sun_dir: direction TO sun
// moon_dir: direction TO moon (for moon light)
// sun_intensity: Extraterrestrial solar irradiance (spectral)
// moon_intensity: Extraterrestrial lunar irradiance (spectral)
// out_transmittance: Transmittance to space (or infinity)
// Returns: In-scattered radiance
Spectrum atmosphere_render(
    const Atmosphere* atm,
    Vec3 ray_origin,
    Vec3 ray_dir,
    Vec3 sun_dir,
    const Spectrum* sun_intensity,
    Vec3 moon_dir,
    const Spectrum* moon_intensity,
    float* out_alpha // Transmittance (average or per-channel? Summary says alpha is exp(-tau). Used for stars.)
    // We will return a float alpha (luminance transmittance or green channel) for star composition.
);

// Calculates transmittance from point p to space along direction dir
Spectrum atmosphere_transmittance(const Atmosphere* atm, Vec3 p, Vec3 dir);

#endif
