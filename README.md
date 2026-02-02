# Knight - Night Sky Renderer

An implementation of "A Physically-Based Night Sky Model" in vanilla C.

## Features
- **Spectral Rendering**: 40 bands (380-780nm) light transport.
- **Atmosphere**: Rayleigh and Mie scattering with spectral ray marching.
- **Ephemerides**: Calculation of Sun, Moon, and Planet positions (Mercury, Venus, Mars, Jupiter, Saturn).
- **Moon Phase**: Dynamic lunar phase and disk shading.
- **Star Catalog**: Renders stars from the Yale Bright Star Catalog (YBS).
- **Night Appearance**: Simulates the Purkinje effect (blue shift) and scotopic vision.
- **Stellar Bloom**: 5x5 Gaussian glare effect for realistic point source appearance.
- **Auto-Exposure**: Reinhard tone mapping with log-average luminance control.

## Building

```bash
make
```

## Running

```bash
./knight
```

**Options:**
- `--no-moon`: Disable Moon rendering and its atmospheric scattering contribution.

This will produce `output.pfm`, a Portable Float Map HDR image.

## Configuration

The current configuration is hardcoded in `src/main.c`:
- Date: 2026-02-17 18:15 UTC (Civil Twilight)
- Location: 45°N, 0°E
- Camera: Facing West at 10° altitude.
- Resolution: 1280x960

## Structure

- `src/core.h/c`: Math, Spectrum, PFM I/O.
- `src/atmosphere.h/c`: Atmospheric scattering models.
- `src/ephemerides.h/c`: Sun/Moon/Planet positioning.
- `src/stars.h/c`: Star catalog parsing.
- `src/tonemap.h/c`: Night vision post-processing and glare.
- `src/main.c`: Main render loop and scene setup.
