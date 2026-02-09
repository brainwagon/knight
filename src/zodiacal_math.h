#ifndef ZODIACAL_MATH_H
#define ZODIACAL_MATH_H

#include "zodiacal.h"

// Obliquity of the Ecliptic (approx J2000)
// Using macro for constant to avoid storage issues in headers if not careful, or static const.
#define EPSILON_RAD (23.439f * DEG2RAD)

static inline HD void horizon_to_ecliptic_math(float alt, float az, float lat_deg, float lmst, float* ecl_lon, float* ecl_lat) {
    float lat = lat_deg * DEG2RAD;
    
    // 1. Horizon -> Equatorial
    float sin_dec = sinf(alt) * sinf(lat) + cosf(alt) * cosf(lat) * cosf(az);
    float dec = asinf(sin_dec);
    
    float cos_dec = cosf(dec);
    float ha = 0;
    if (fabsf(cos_dec) > 1e-4f) {
        float sin_ha = -sinf(az) * cosf(alt) / cos_dec;
        float cos_ha = (sinf(alt) - sinf(lat) * sin_dec) / (cosf(lat) * cos_dec);
        ha = atan2f(sin_ha, cos_ha);
    }
    
    float ra = lmst - ha; // RA = LMST - HA
    while (ra < 0) ra += TWO_PI;
    while (ra >= TWO_PI) ra -= TWO_PI;
    
    // 2. Equatorial -> Ecliptic
    float sin_beta = sin_dec * cosf(EPSILON_RAD) - cos_dec * sinf(EPSILON_RAD) * sinf(ra);
    *ecl_lat = asinf(sin_beta);
    
    float sin_lambda_part = sin_dec * sinf(EPSILON_RAD) + cos_dec * cosf(EPSILON_RAD) * sinf(ra);
    float cos_lambda_part = cos_dec * cosf(ra);
    
    *ecl_lon = atan2f(sin_lambda_part, cos_lambda_part);
    if (*ecl_lon < 0) *ecl_lon += TWO_PI;
}

static inline HD Spectrum compute_zodiacal_light_math(Vec3 view_dir, Vec3 sun_dir, float sun_ecliptic_lon_deg, float cam_lat, float lmst) {
    (void)sun_dir;
    Spectrum result;
    spectrum_zero(&result);
    
    // Convert view dir to Alt/Az
    float alt = asinf(view_dir.y);
    float az = atan2f(view_dir.x, view_dir.z); 
    
    // Horizon to Ecliptic
    float lambda, beta;
    horizon_to_ecliptic_math(alt, az, cam_lat, lmst, &lambda, &beta);
    
    // Relative longitude to Sun
    float sun_lambda = sun_ecliptic_lon_deg * DEG2RAD;
    float d_lambda = fabsf(lambda - sun_lambda);
    while (d_lambda > PI) d_lambda = TWO_PI - d_lambda;
    
    // Elongation (angular distance from sun)
    float cos_epsilon = cosf(beta) * cosf(d_lambda);
    float epsilon = acosf(cos_epsilon); // radians
    float epsilon_deg = epsilon * RAD2DEG;
    
    float eps_term = 1.0f / (powf(epsilon_deg, 1.5f) + 1.0f); // Peak near 0
    // Gegenschein: Gaussian at 180
    float gegen_term = 0.002f * expf(-powf((epsilon_deg - 180.0f)/15.0f, 2.0f));
    
    // Latitude falloff
    float beta_term = expf(-3.0f * fabsf(beta)); // Narrow band
    
    // Combine
    float intensity_base = (2000.0f * eps_term + 5.0f * gegen_term) * beta_term;
    
    float scale = 3.0e-7f; 
    
    float L = intensity_base * scale;
    
    spectrum_set(&result, L);
    
    return result;
}

#endif
