#include "core.h"

// CIE 1931 2-degree XYZ matching functions (approximate for 380-770nm step 10nm)
// Data source: standard tables
static const float CIE_X[40] = {
    0.0014, 0.0042, 0.0143, 0.0435, 0.1344, 0.2839, 0.3483, 0.3362, 0.2908, 0.1954, // 380-470
    0.0956, 0.0320, 0.0049, 0.0093, 0.0633, 0.1655, 0.2904, 0.4334, 0.5945, 0.7621, // 480-570
    0.9163, 1.0263, 1.0622, 1.0026, 0.8544, 0.6424, 0.4479, 0.2835, 0.1649, 0.0874, // 580-670
    0.0468, 0.0227, 0.0114, 0.0058, 0.0029, 0.0014, 0.0007, 0.0003, 0.0002, 0.0001  // 680-770
};

static const float CIE_Y[40] = {
    0.0000, 0.0001, 0.0004, 0.0012, 0.0040, 0.0116, 0.0230, 0.0380, 0.0600, 0.0910, // 380-470
    0.1390, 0.2080, 0.3230, 0.5030, 0.7100, 0.8620, 0.9540, 0.9950, 0.9950, 0.9520, // 480-570
    0.8700, 0.7570, 0.6310, 0.5030, 0.3810, 0.2650, 0.1750, 0.1070, 0.0610, 0.0320, // 580-670
    0.0170, 0.0082, 0.0041, 0.0021, 0.0010, 0.0005, 0.0002, 0.0001, 0.0001, 0.0000  // 680-770
};

static const float CIE_Z[40] = {
    0.0065, 0.0201, 0.0679, 0.2074, 0.6456, 1.3856, 1.7471, 1.7721, 1.6692, 1.2876, // 380-470
    0.8130, 0.4652, 0.2720, 0.1582, 0.0782, 0.0422, 0.0203, 0.0087, 0.0039, 0.0021, // 480-570
    0.0017, 0.0011, 0.0008, 0.0003, 0.0002, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, // 580-670
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000  // 680-770
};

// CIE 1951 Scotopic V(lambda) - approximated
static const float CIE_V[40] = {
    0.0006, 0.0022, 0.0093, 0.0348, 0.1084, 0.2525, 0.4571, 0.6756, 0.8524, 0.9632, // 380-470
    0.9939, 0.9398, 0.8110, 0.6496, 0.4812, 0.3283, 0.2076, 0.1212, 0.0665, 0.0346, // 480-570
    0.0173, 0.0083, 0.0039, 0.0018, 0.0008, 0.0004, 0.0002, 0.0001, 0.0000, 0.0000, // 580-670
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000  // 680-770
};

void spectrum_zero(Spectrum* s) {
    memset(s->s, 0, sizeof(float) * SPECTRUM_BANDS);
}

void spectrum_set(Spectrum* s, float val) {
    for (int i = 0; i < SPECTRUM_BANDS; i++) s->s[i] = val;
}

void spectrum_add(Spectrum* dest, const Spectrum* src) {
    for (int i = 0; i < SPECTRUM_BANDS; i++) dest->s[i] += src->s[i];
}

void spectrum_mul(Spectrum* dest, float scalar) {
    for (int i = 0; i < SPECTRUM_BANDS; i++) dest->s[i] *= scalar;
}

void spectrum_mul_spec(Spectrum* dest, const Spectrum* src) {
    for (int i = 0; i < SPECTRUM_BANDS; i++) dest->s[i] *= src->s[i];
}

void blackbody_spectrum(float tempK, Spectrum* out) {
    // Planck's law: B(lambda, T) = (2hc^2 / lambda^5) * (1 / (exp(hc/(lambda*k*T)) - 1))
    // Constants:
    // h = 6.626e-34
    // c = 3.0e8
    // k = 1.38e-23
    // c1 = 2hc^2 approx 1.191e-16 W*m^2
    // c2 = hc/k  approx 1.4388e-2 m*K
    
    // Using nm for lambda:
    // lambda_m = lambda * 1e-9
    
    // To avoid huge/tiny numbers, we can compute relative power.
    // Or just use the formula and let the caller scale it (e.g., by stellar irradiance).
    
    double c1 = 1.191e-16; // W * m^2
    double c2 = 1.4388e-2; // m * K
    
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        float lambda_nm = LAMBDA_START + i * LAMBDA_STEP;
        double lambda_m = lambda_nm * 1e-9;
        
        double power_term = pow(lambda_m, 5.0);
        double exp_term = exp(c2 / (lambda_m * tempK)) - 1.0;
        
        out->s[i] = (float)(c1 / (power_term * exp_term));
    }
}

XYZV spectrum_to_xyzv(const Spectrum* s) {
    XYZV res = {0, 0, 0, 0};
    // Riemann sum integration: sum(S * matching * dLambda)
    // dLambda = 10 nm = 10 units roughly, but if matching functions are unit-integral normalized differently...
    // Standard is usually: Y = 683 * sum ... for lumens, but we want relative XYZ for now unless we do full radiometry.
    // Let's assume K_m = 683 lm/W is applied later or we work in radiometric units until display.
    // Just summation here.
    
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        res.X += s->s[i] * CIE_X[i];
        res.Y += s->s[i] * CIE_Y[i];
        res.Z += s->s[i] * CIE_Z[i];
        res.V += s->s[i] * CIE_V[i];
    }
    
    // Scale by dLambda?
    // If our spectrum is spectral radiance (W/sr/m2/nm), then multiplying by nm gives W/sr/m2.
    // So yes, multiply by 10 (step).
    // However, usually we just want relative values for rendering unless we need absolute units.
    // The paper specifies physical units. Let's multiply by step.
    float dLambda = LAMBDA_STEP;
    res.X *= dLambda;
    res.Y *= dLambda;
    res.Z *= dLambda;
    res.V *= dLambda;
    
    return res;
}

RGB xyz_to_srgb(float X, float Y, float Z) {
    // Simple sRGB transform
    // X, Y, Z assumed to be in range [0, 1] typically, or absolute.
    // We'll assume linear scaling is handled by tone mapping before this, 
    // BUT this function takes raw XYZ. 
    // Standard sRGB matrix (linear RGB):
    
    float r =  3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float b =  0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;
    
    return (RGB){r, g, b};
}

void write_pfm(const char* filename, int width, int height, const RGB* data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;
    
    // PFM Header
    // PF = RGB color, pf = grayscale
    // width height
    // scale (negative for little endian)
    fprintf(f, "PF\n%d %d\n-1.0\n", width, height);
    
    // PFM stores bottom-to-top usually? 
    // "The raster is a sequence of float values, proceeding from left to right across each scanline, 
    // the scanlines being written from bottom to top."
    // Let's write bottom-to-top.
    
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            RGB p = data[y * width + x];
            float scanline[3] = {p.r, p.g, p.b};
            fwrite(scanline, sizeof(float), 3, f);
        }
    }
    
    fclose(f);
}
