#include "zodiacal.h"
#include <math.h>

// Obliquity of the Ecliptic (approx J2000)
static const float EPSILON = 23.439f * DEG2RAD;

void horizon_to_ecliptic(float alt, float az, float lat_deg, float lmst, float* ecl_lon, float* ecl_lat) {
    float lat = lat_deg * DEG2RAD;
    
    // 1. Horizon -> Equatorial
    // sin(Dec) = sin(Alt)sin(Lat) + cos(Alt)cos(Lat)cos(Az)  (Note: Az=0 is North?? In this codebase Az=0 is North, 90 is East)
    // Wait, let's verify codebase convention.
    // main.c: "0=N, 90=E". So standard astronomical azimuth.
    // Formula: sin(Dec) = sin(Alt)sin(Lat) + cos(Alt)cos(Lat)cos(Az)
    // cos(HA) = (sin(Alt) - sin(Lat)sin(Dec)) / (cos(Lat)cos(Dec))
    // sin(HA) = -sin(Az)cos(Alt)/cos(Dec)
    
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
    // sin(beta) = sin(dec)cos(eps) - cos(dec)sin(eps)sin(ra)
    // cos(beta)cos(lambda) = cos(dec)cos(ra)
    // cos(beta)sin(lambda) = sin(dec)sin(eps) + cos(dec)cos(eps)sin(ra)
    
    float sin_beta = sin_dec * cosf(EPSILON) - cos_dec * sinf(EPSILON) * sinf(ra);
    *ecl_lat = asinf(sin_beta);
    
    float cos_beta = cosf(*ecl_lat);
    float sin_lambda_part = sin_dec * sinf(EPSILON) + cos_dec * cosf(EPSILON) * sinf(ra);
    float cos_lambda_part = cos_dec * cosf(ra);
    
    *ecl_lon = atan2f(sin_lambda_part, cos_lambda_part);
    if (*ecl_lon < 0) *ecl_lon += TWO_PI;
}

Spectrum compute_zodiacal_light(Vec3 view_dir, Vec3 sun_dir, float sun_ecliptic_lon_deg, float cam_lat, float lmst) {
    Spectrum result;
    spectrum_zero(&result);
    
    // Convert view dir to Alt/Az
    float alt = asinf(view_dir.y);
    float az = atan2f(view_dir.x, view_dir.z); // code convention: x=East? No.
    // main.c: sun_dir.x = cos(alt)sin(az). x is East component?
    // z = cos(alt)cos(az). z is North component?
    // Let's stick to consistent atan2(x, z).
    
    // Horizon to Ecliptic
    float lambda, beta;
    horizon_to_ecliptic(alt, az, cam_lat, lmst, &lambda, &beta);
    
    // Relative longitude to Sun
    float sun_lambda = sun_ecliptic_lon_deg * DEG2RAD;
    float d_lambda = fabsf(lambda - sun_lambda);
    while (d_lambda > PI) d_lambda = TWO_PI - d_lambda;
    
    // Elongation (angular distance from sun)
    // cos(epsilon) = sin(beta_s)sin(beta) + cos(beta_s)cos(beta)cos(d_lambda)
    // Sun beta approx 0.
    float cos_epsilon = cosf(beta) * cosf(d_lambda);
    float epsilon = acosf(cos_epsilon); // radians
    float epsilon_deg = epsilon * RAD2DEG;
    
    // Simplified Zodiacal Light Model
    // Based on visual characteristics:
    // 1. Intensity drops with elongation (epsilon)
    // 2. Intensity drops with ecliptic latitude (beta) - confined to plane
    // 3. Gegenschein bump at 180 deg
    
    // Leinert et al. 1998 Table 17 is the gold standard, but we approximate.
    // I_zod(eps, beta) approx I0 * f(eps) * h(beta)
    
    // Shape function f(eps) in degrees:
    // Very bright < 20 deg (False Dawn/Dusk)
    // Fades to minimum around 135 deg
    // Slight rise at 180 (Gegenschein)
    
    // Analytical approx inspired by fitting Leinert's data:
    // brightness (S10) approx:
    // 10^(A - B*log(sin(eps))) ? No, let's use sum of power laws.
    
    // A decent heuristic for visual appearance:
    float eps_term = 1.0f / (powf(epsilon_deg, 1.5f) + 1.0f); // Peak near 0
    // Gegenschein: Gaussian at 180
    float gegen_term = 0.002f * expf(-powf((epsilon_deg - 180.0f)/15.0f, 2.0f));
    
    // Latitude falloff
    // Width is roughly 10-15 degrees
    float beta_term = expf(-3.0f * fabsf(beta)); // Narrow band
    
    // Combine
    float intensity_base = (2000.0f * eps_term + 5.0f * gegen_term) * beta_term;
    
    // Scale to physical units (approximate)
    // Typical zodiacal light is ~ 200 S10 units at 90 deg elongation?
    // We need radiance. 1 S10 unit ~ 1.25e-9 cd/m^2 approx.
    // Let's tune 'intensity_base' to be roughly correct relative to star brightness.
    // Star mag 0 ~ 1e-6 radiance.
    // Zodiacal light is faint, roughly Vmag 22 per arcsec^2?
    // Let's use a global scale factor that looks good.
    float scale = 3.0e-7f; 
    
    float L = intensity_base * scale;
    
    // Spectrum: Zodiacal light is sunlight scattered by dust.
    // It's slightly redder than sun? Or mostly white (sun color)?
    // Usually assumed to be Solar Spectrum.
    
    // Assume we have a global Sun spectrum available or hardcoded.
    // We'll create a generic solar spectrum here.
    Spectrum sun_spec;
    spectrum_set(&sun_spec, 100.0f); // Base white
    // Ideally use blackbody(5800)
    
    spectrum_mul_spec(&sun_spec, &result); // currently 0
    spectrum_set(&result, L);
    
    // Apply sun color (simplified as white for now, matching main.c sun_intensity base)
    // spectrum_mul_spectrum(&result, &sun_spec); 
    
    return result;
}
