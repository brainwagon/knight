Below is a concise, implementation-oriented guide to the paper’s core ideas with concrete algorithmic steps, formulas, data structures, and default parameters to help drive a C implementation. It’s organized as a pipeline with modules you can implement independently and integrate. 🌙✨

Sections
- High-level pipeline 🧭
- Physical light sources and models 🔆
- Atmosphere and clouds 🌫️
- Stars: data, color, and appearance ⭐
- Spectral rendering and buffers 🌈
- Tone mapping and night appearance 🎚️
- Core data structures and key functions in C-like pseudocode 🧩
- Parameters and constants 📏
- Performance and validation 🧪
- Minimal end-to-end flowchart 🧵

High-level pipeline 🧭
1) Ephemerides
- Compute Sun and Moon positions (Meeus).
- Compute star apparent positions (precession/nutation + horizon transform).

2) Scene/light assembly
- Prepare spectral light models: Moon (direct + atmospheric scattering), zodiacal light, airglow, integrated starlight/diffuse constants.

3) Spectral renderer
- Spherical atmosphere with Rayleigh/Mie scattering.
- Distribution ray tracing + ray marching.
- Alpha buffer records background visibility (transmittance to space).

4) Star buffer
- Render stars into a sky image using catalog, spectral color from magnitude + B−V, then composite with atmospheric alpha.

5) Tone mapping
- Map intensities (Ward).
- Apply night hue mapping (blue shift driven by scotopic/mesopic state).
- Selective blur (loss of detail) preserving edges.
- Add stellar glare/stellations PSF.

6) Output
- Write tonemapped RGB image.

Physical light sources and models 🔆
A) Sun (indirect at night)
- Use Meeus for position.
- During astronomical twilight, include scattered sunlight in atmosphere.

B) Moon
1. Position:
- Meeus + diurnal parallax; include optical librations for accurate appearance.

2. Irradiance from Moon (at Earth):
- Lommel-Seeliger-based formula:
  Em(α, d) = (2/3) * C * (rm^2 / d^2) * (Es,m + Ee,m) * [1 − sin(α/2) * tan(α/2) * log(cot(α/4))]
  - C ≈ 0.072 (avg albedo), rm = lunar radius, d = Earth–Moon distance
  - Es,m = solar irradiance at Moon, Ee,m = earthshine irradiance at Moon

3. Appearance BRDF (Hapke-Lommel-Seeliger simplification):
  f(θi, θr, φ) = (2 / (3π)) * K(β,λ) * B(α,g) * S(φ) * (1 / (1 + cosθr / cosθi))
  - K(β,λ): albedo map (use Clementine albedo and height maps)
  - B(α,g): retrodirective function, g ~ 0.6 typical
  - S(φ): scatter law with t ≈ 0.1 to add slight forward scattering
- Opposition brightening not explicitly modeled (acceptable limitation).
- Earthshine: include Earth as a secondary light for Moon shading; Earth albedo ≈ 0.3; phase opposite of Moon.

C) Stars
- Apparent visual magnitude mv → irradiance Es (W/m^2):
  Es = 10^(-(mv + 19))
- Color from B−V index → effective temperature:
  Teff ≈ 7000 K / (B−V + 0.56)
- Spectrum: scale blackbody spectrum at Teff by Es; optionally attenuate in the Balmer continuum (< 364.6 nm).

D) Zodiacal light 🌌
- Sample a measured ecliptic-direction table; bilinear lookup by ray exit direction in ecliptic coords.

E) Airglow
- Add an emitting atmospheric layer (~110 km) with dominant lines:
  - 557.7 nm (O I), 630 nm (O I), 589.0–589.6 nm (Na doublet)
- Temporal variation can be ignored initially; treat as steady emission for night.

F) Diffuse galactic & cosmic background
- Add small constant term, e.g., 1e-8 W/m^2.

Atmosphere and clouds 🌫️
A) Spherical atmosphere
- Stratified layers with scale heights (molecules vs aerosols):
  - Rayleigh (molecular): strong λ^-4 dependence, nearly isotropic
  - Mie (aerosols): weak wavelength dependence, strong forward peak
- Use standard Rayleigh/Mie coefficients and phase functions (as in Nishita 1993).
- Integrate optical depth τ and in-scattered radiance along camera rays via ray marching; include multiple scattering using distribution ray tracing.

B) Clouds ☁️
- Procedural 3D density via turbulence/fractal noise.
- Henyey–Greenstein phase function, gHG ≈ 0.8–0.9 (forward scattering).
- March within cloud medium; sample direct and indirect in-scattered radiance.
- Record partial background visibility into alpha for star compositing.

Stars: data, color, and appearance ⭐
A) Catalog
- Yale Bright Star Catalog (≈ 9000 stars up to mag +6.5).

B) Coordinates
- Correct catalog (J2000) by precession/nutation; convert to local horizon (altitude/azimuth) via a single rotation matrix computed from time and site.

C) Star rendering
- Build a “star buffer” in screen space:
  1) Project visible stars to screen via sky dome mapping.
  2) Evaluate per-star spectral radiance from Es and blackbody spectrum.
  3) Rasterize as sub-pixel splats; brightness ~ photometric calibration.
- Composite: star_buffer *= alpha_background_from_atmosphere, then add to main image.

Spectral rendering and buffers 🌈
A) Spectral sampling
- 380–780 nm in 10 nm steps (40 bands).
- Store radiance per band throughout transport; end with XYZV.

B) Radiance integration
- Primary camera rays:
  - March through atmosphere; accumulate:
    - Transmittance T = exp(-τ)
    - In-scattered radiance (Rayleigh + Mie + airglow)
  - Spawn stochastic secondary samples for multiple scattering (few samples suffice in thin atmosphere).
- Scene geometry (terrain, Moon):
  - Use global illumination for diffuse geometry (spectral irradiance cache).
  - For the Moon, shade spherical geometry with Hapke-LS BRDF and albedo/height maps.

C) Alpha buffer (background visibility)
- For each camera ray, when it exits the atmosphere, store alpha = exp(-τ_to_space). Used to attenuate stars and zodiacal light.

Tone mapping and night appearance 🎚️
A) Intensity mapping
- Use Ward’s contrast-based operator for luminance compression.

B) Hue mapping (blue shift) 💙
- Convert XYZV → mesopic mix using “rod saturation” s based on Y:
  - s = 0 if log10(Y) < -2
  - s = cubic smoothstep from -2 to 0.6 (mesopic)
  - s = 1 if log10(Y) ≥ 0.6
- Target bluish chromaticity for scotopic pixels:
  - Choose (xb, yb) ≈ (0.25, 0.25)
  - New chromaticity: x = (1−s)*xb + s*x, y = (1−s)*yb + s*y
- Mix scotopic luminance V and photopic Y:
  - Ymixed = 0.4468*(1−s)*V + s*Y
- Recompute X,Z from (x,y,Ymixed).
- Effect: fully scotopic areas shift toward blue; mesopic areas partially; photopic unchanged.

C) Loss of detail (edge-preserving blur) 🔍
- Compute gradient magnitude on a smoothed luminance image.
- Blur standard deviation σ per-pixel:
  σ = σ0 * (gradStrong − gradMag) / gradStrong, clamped to [0, σ0]
  - gradStrong: minimum gradient where blur stops (empirical)
  - σ0: calibrated so 4 cycles/degree grating is just invisible under moonlight

D) Stellations/glare around stars ✴️
- Apply scotopic glare PSF (Spencer et al. 1995) to bright stellar pixels.

Core data structures and key functions in C-like pseudocode 🧩
A) Spectra and images
- Spectrum: fixed-length array for 40 bands
  typedef struct { float s[40]; } Spectrum;

- XYZV buffer (floating-point HDR), plus alpha
  typedef struct { float X, Y, Z, V; } XYZV;
  typedef struct { float r, g, b; } RGB;

B) Ephemerides
- Meeus Sun/Moon; star transforms
  void sun_moon_position(double jd, double lat, double lon, Vec3* sun_dir, Vec3* moon_dir);
  void star_equ_to_horizon(double jd, double lat, double lon, Star* catalog, int n);

C) Moon irradiance and BRDF
  float moon_irradiance(float alpha, float d_earth_moon, float Es_m, float Ee_m);
  float moon_brdf(float thetai, float thetar, float phi, float K, float alpha, float g, float t);

D) Atmosphere marching
  typedef struct { /* density profiles, phase functions, coeffs */ } Atmosphere;
  RaymarchResult march_camera_ray(const Atmosphere* atm, Ray ray);

  typedef struct {
    Spectrum L;  // accumulated radiance
    float alpha; // exp(-tau_to_space)
  } RaymarchResult;

E) Clouds
  float cloud_density(Vec3 p); // turbulence-based
  Phase HG; // Henyey-Greenstein with gHG

F) Star color and rendering
  float magnitude_to_irradiance(float mv) { return powf(10.0f, -(mv + 19.0f)); }
  float BminusV_to_Teff(float BmV) { return 7000.0f / (BmV + 0.56f); }
  void blackbody_spectrum(float Teff, Spectrum* outNorm);
  void scale_spectrum(Spectrum* s, float scale);

  void render_stars_to_buffer(const Star* stars, int n, const Camera* cam, ImageHDR* starBuf);

G) Zodiacal light and airglow
  float zodiacal_lookup(float ecl_lon, float ecl_lat); // bilinear
  Spectrum airglow_emission(float altitude_km);        // lines at 557.7, 589.0/589.6, 630 nm

H) Spectral to XYZV and tone mapping
  XYZV spectrum_to_XYZV(const Spectrum* L);
  RGB ward_tonemap(const XYZV* hdr, /* display params */);

  // Hue mapping and blue shift
  float rod_saturation(float Y);
  void apply_blue_shift_and_mesopic_mix(XYZV* p);

  // Edge-preserving blur
  void selective_blur(ImageF* luminance, float gradStrong, float sigma0);

  // Glare
  void apply_stellar_glare(ImageRGB* img, const PSF* glare_psf);

Parameters and constants 📏
- Spectral sampling: 380–780 nm, Δλ = 10 nm (40 bands)
- Atmosphere scale heights (typical):
  - Rayleigh: ~8 km; Mie: ~1.2 km (site/weather dependent)
- Phase functions:
  - Rayleigh: PR(θ) = 3/(16π) * (1 + cos^2 θ)
  - Mie: Henyey–Greenstein with gHG ≈ 0.8–0.9
- Moon:
  - C (avg albedo) ≈ 0.072; t ≈ 0.1; g ≈ 0.6
- Earth albedo: ~0.3
- Background light:
  - Integrated starlight (illumination): ~3e-8 W/m^2
  - Diffuse galactic + cosmic term: ~1e-8 W/m^2
- Blue-shift target chromaticity: (xb, yb) ≈ (0.25, 0.25)
- Scotopic mix factor: Ymixed = 0.4468*(1−s)*V + s*Y
- Ward tone mapping: use standard contrast-based operator (gamma ~ 1.1–1.5 depending on display modality)

Performance and validation 🧪
- Distribution ray tracing in a thin atmosphere converges with few in-scattering samples; stratified sampling for stability.
- Cache spectral irradiance for diffuse GI.
- Precompute per-band Rayleigh/Mie coefficients and phase function tables.
- Validate pieces independently:
  - Atmosphere-only sky colors vs Nishita sky curves
  - Moon irradiance vs known full-moon illuminance (~0.1–0.3 lux at ground)
  - Star color temperatures vs B−V references
  - Airglow visible green line (557.7 nm) dominance on moonless nights
  - Edge-preserving blur: confirm text becomes unreadable under moonlight while edges remain recognizable

Minimal end-to-end flowchart 🧵
1) Precompute:
- Load star catalog; Moon albedo/height maps; zodiacal table; CIE color matching functions.

2) For a time/location:
- Compute Sun/Moon positions; star transforms to horizon.

3) Render spectral image:
- Ray march atmosphere (Rayleigh + Mie + airglow) with multiple scattering.
- Shade terrain/geometry using spectral GI.
- Record alpha = exp(-τ_to_space).

4) Stars:
- Build star buffer with spectral colors; composite starBuf *= alpha; add to image.

5) Convert spectral → XYZV.

6) Night appearance:
- Apply hue mapping (s-based blue shift, mixed Y/V).
- Edge-preserving blur (selective loss of detail).
- Ward tone mapping to display range.
- Stellar glare PSF.

7) Output RGB image.

Notes and limitations ⚠️
- Opposition surge and detailed Moon microgeometry not included.
- Mesopic photometry is approximated via empirical blend between V and Y with a blue shift.
- Parameters like gradStrong, σ0, glare thresholds should be tuned with test images (moonless vs full-moon nights).
- For higher dynamic range (with artificial lights), replace Ward with a modern HDR operator but keep the same hue/blur/glare steps.

This plan should give you clean module boundaries and enough math, data, and defaults to guide a robust C implementation of the paper’s night rendering system. 🌙⭐️
