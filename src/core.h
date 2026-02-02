#ifndef CORE_H
#define CORE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define INV_PI 0.31830988618379067154f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

// Spectral sampling
#define SPECTRUM_BANDS 40
#define LAMBDA_START 380.0f
#define LAMBDA_STEP 10.0f
#define LAMBDA_END 780.0f

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float s[SPECTRUM_BANDS];
} Spectrum;

typedef struct {
    float X, Y, Z, V;
} XYZV;

typedef struct {
    float r, g, b;
} RGB;

// Vec3 functions
static inline Vec3 vec3_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vec3 vec3_mul(Vec3 a, float s) { return (Vec3){a.x * s, a.y * s, a.z * s}; }
static inline float vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
static inline float vec3_length(Vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
static inline Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len > 0.0f) return vec3_mul(v, 1.0f / len);
    return v;
}

// Spectrum functions
void spectrum_zero(Spectrum* s);
void spectrum_set(Spectrum* s, float val);
void spectrum_add(Spectrum* dest, const Spectrum* src);
void spectrum_mul(Spectrum* dest, float scalar);
void spectrum_mul_spec(Spectrum* dest, const Spectrum* src);

// Blackbody radiation
// Returns spectral radiance (W/m^2/sr/nm) normalized to some extent or raw? 
// Usually raw Planck: B(lambda, T) = (2hc^2 / lambda^5) * (1 / (exp(hc/lambdakT) - 1))
// We will return relative values or normalized, handled by caller scaling.
void blackbody_spectrum(float tempK, Spectrum* out);

// PFM Output
// Writes a portable float map. Width, height, and RGB data (3 floats per pixel).
// Note: PFM is usually RGB. We might want to write our tone-mapped RGB to it.
void write_pfm(const char* filename, int width, int height, const RGB* data);

// Color conversion
XYZV spectrum_to_xyzv(const Spectrum* s);
RGB xyz_to_srgb(float X, float Y, float Z);

#endif
