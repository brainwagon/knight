# Knight - Night Sky Renderer

## Project Overview
**Knight** is a physically-based night sky rendering engine written in vanilla C. It implements the key concepts from the paper "A Physically-Based Night Sky Model" (Jensen et al.), focusing on spectral rendering, atmospheric scattering, and night-time perception effects.

### Key Features
*   **Spectral Rendering**: Simulates light transport across 40 spectral bands (380-780nm).
*   **Atmosphere**: Implements Rayleigh and Mie scattering using ray marching through a spherical atmosphere.
*   **Ephemerides**: Calculates accurate positions for the Sun and Moon, and transforms star coordinates from the Yale Bright Star Catalog (YBS).
*   **Night Appearance**: Simulates scotopic vision traits, including the Purkinje effect (blue shift) and rod saturation.
*   **Moon Phase**: Dynamic calculation of lunar phase and brightness based on Sun-Moon-Earth geometry.

## Architecture
The project is structured into modular components within the `src/` directory:
*   `main.c`: The primary entry point, configuration (time/location), and render loop.
*   `atmosphere.c`: Handles spectral optical depth integration and scattering phase functions.
*   `ephemerides.c`: Computes astronomical positions for celestial bodies.
*   `stars.c`: Parses and manages star data from the YBS catalog.
*   `tonemap.c`: Post-processing for night vision simulation (blue shift, tone mapping).
*   `core.c`: Math utilities, spectrum manipulation, and PFM image output.

## Building and Running

### Prerequisites
*   GCC or compatible C compiler.
*   `make`.
*   Standard C math libraries (`-lm`).
*   `libjpeg` development libraries (`-ljpeg`).

### Build Command
To compile the project:
```bash
make
```

### Run Command
To execute the renderer:
```bash
./knight
```

**Options:**
*   `--no-moon`: Disables the Moon rendering and its atmospheric scattering contribution (useful for isolating starlight).

**Output:**
The program generates a `output.pfm` file in the root directory. This is a Portable Float Map (HDR) image that can be viewed with compatible tools (e.g., Photoshop, GIMP, ImageMagick).

## Development Conventions
*   **Language**: Vanilla C (C99).
*   **Data**: External data (star catalogs) resides in the `data/` directory.
*   **Configuration**: Currently, simulation parameters like Date, Time, Location (Lat/Lon), and Resolution are hardcoded in `src/main.c` and require recompilation to change.
*   **Math**: Uses custom `Vec3` and `Spectrum` structs defined in `src/core.h`.
*   **Output**: Produces High Dynamic Range (HDR) output to preserve physical radiance values before tone mapping.
