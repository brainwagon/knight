# Technology Stack

## Core Language and Runtime
- **C (C99)**: The primary programming language used for the rendering engine, chosen for its performance and low-level control.
- **CUDA**: Utilized for GPU acceleration of the ray marching and spectral integration processes. The codebase supports both CPU (fallback) and GPU execution paths.

## Graphics and Imaging
- **libjpeg**: Used for encoding rendered images into JPEG format for easy viewing and previewing.
- **PFM (Portable Float Map)**: The primary output format for high-dynamic-range (HDR) radiance data, preserving spectral accuracy before tone mapping.
- **ImageMagick (Optional)**: Recommended for converting PFM files to other standard formats (like PNG) via the CLI.

## Mathematical and Scientific Libraries
- **Standard C Math Library (math.h)**: Used for all fundamental astronomical and atmospheric calculations.
- **Custom Math Utilities**: The project uses internal implementations for vector math (`Vec3`) and spectral manipulation (`Spectrum`) to maintain physical consistency.

## Build and Development Tools
- **GNU Make**: The build system used to manage compilation of C and CUDA source files.
- **GCC / NVCC**: Compilers used for building the CPU and GPU components respectively.

## External Data Sources
- **Yale Bright Star Catalog (YBSC5)**: Provides the fundamental data (coordinates, magnitudes, spectral types) for rendering the night sky's stars.
- **IAU Constellation Boundaries**: Uses `data/bound_in_20.txt` for defining the vertices of constellation polygons in J2000 coordinates.
