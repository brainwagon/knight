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

Ensure you have `libjpeg` development headers installed (e.g., `libjpeg-dev` on Ubuntu).

```bash
make
```

## Running

```bash
./knight [options]
```

### Options:
- `-l, --lat <deg>`: Observer latitude (default: 45.0).
- `-L, --lon <deg>`: Observer longitude (default: 0.0).
- `-d, --date <YYYY-MM-DD>`: Simulation date (default: 2026-02-17).
- `-t, --time <hour>`: UTC decimal hour (default: 18.25).
- `-a, --alt <deg>`: Viewer altitude above horizon (default: 10.0).
- `-z, --az <deg>`: Viewer azimuth (0=N, 90=E, 180=S, 270=W, default: 270.0).
- `-f, --fov <deg>`: Field of view in degrees (default: 60.0).
- `-w, --width <px>`: Image width (default: 640).
- `-h, --height <px>`: Image height (default: 480).
- `-o, --output <file>`: Output filename (default: output.pfm).
- `-e, --exposure <val>`: Exposure boost in f-stops (default: 0.0). Positive values brighten the image, negative values darken it.
- `-E, --env`: Generate a cylindrical (equirectangular) environment map of the complete sky (360° azimuth, 180° altitude).
- `-n, --no-moon`: Disable Moon rendering and its atmospheric scattering contribution.
- `--help`: Show usage information.

### Examples:
**Boost exposure by 2 stops:**
```bash
./knight -e 2.0 -o brighter.pfm
```

**Equirectangular Environment Map:**
```bash
./knight --env --width 2048 --height 1024 -o sky_env.pfm
```

**Twilight in Los Angeles:**
```bash
./knight -l 34.05 -L -118.24 -t 12.0 -d 2026-06-21 -a 45 -z 180 -f 90 -w 1280 -h 720 -o sunset.pfm
```

**Deep Night (New Moon):**
```bash
./knight -d 2026-02-17 -t 22.0 -a 45 -z 180
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