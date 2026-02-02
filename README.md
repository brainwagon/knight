# Knight - Night Sky Renderer

An implementation of "A Physically-Based Night Sky Model" in vanilla C.

## ⚠️ Disclaimer

**This code was written entirely by Gemini CLI** based on a high-level summary of the original "Night Rendering" paper. No serious code review has been performed. It is highly experimental and almost certainly contains physical or algorithmic errors. In particular, the tone-mapping and automatic exposure implementations are approximations and may not accurately reflect the intended results of the original research or realistic photometric behavior.

## Features
- **Spectral Rendering**: 40 bands (380-780nm) light transport.
- **Atmosphere**: Rayleigh and Mie scattering with spectral ray marching.
- **Ephemerides**: Calculation of Sun, Moon, and Planet positions (Mercury, Venus, Mars, Jupiter, Saturn).
- **Moon Phase**: Dynamic lunar phase calculation and shaded disk rendering with earthshine.
- **Star Catalog**: Renders ~9000 stars from the Yale Bright Star Catalog (YBS).
- **Night Appearance**: Simulates the Purkinje effect (blue shift) and scotopic vision traits.
- **Stellar Bloom**: 5x5 Gaussian glare effect for realistic point source (star/planet) appearance.
- **Auto-Exposure**: Reinhard tone mapping with log-average luminance control to handle everything from deep night to twilight.
- **Ground Plane**: Includes a basic ground occlusion and horizon definition.

## Building

```bash
make
```

## Running

```bash
./knight [options]
```

### Options:
- `--lat <deg>`: Observer latitude (default: 45.0).
- `--lon <deg>`: Observer longitude (default: 0.0).
- `--date <YYYY-MM-DD>`: Simulation date (default: 2026-02-17).
- `--time <hour>`: UTC decimal hour (default: 18.25 / 18:15).
- `--alt <deg>`: Viewer altitude above horizon (default: 10.0).
- `--az <deg>`: Viewer azimuth (0=N, 90=E, 180=S, 270=W, default: 270.0).
- `--fov <deg>`: Field of view in degrees (default: 60.0).
- `--width <px>`: Image width (default: 640).
- `--height <px>`: Image height (default: 480).
- `--no-moon`: Disable Moon rendering and its atmospheric scattering contribution.
- `--help`: Show usage information.

### Examples:
**Twilight in Los Angeles:**
```bash
./knight --lat 34.05 --lon -118.24 --time 12.0 --date 2026-06-21 --alt 45 --az 180 --fov 90 --width 1280 --height 720
```

**Deep Night (New Moon):**
```bash
./knight --date 2026-02-17 --time 22.0 --alt 45 --az 180
```

## Output
The program generates `output.pfm`, a Portable Float Map (HDR) image. You can view this file with tools like Photoshop, GIMP, or `display` (ImageMagick).

## Structure
- `src/main.c`: Primary entry point, argument parsing, and render loop.
- `src/atmosphere.h/c`: Atmospheric scattering models and ray marching.
- `src/ephemerides.h/c`: Sun, Moon, and Planet positioning logic.
- `src/stars.h/c`: Yale Bright Star Catalog parsing.
- `src/tonemap.h/c`: Auto-exposure, Reinhard tone mapping, blue shift, and Gaussian glare.
- `src/core.h/c`: Spectral math, vector utilities, and PFM I/O.