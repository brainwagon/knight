# Specification: Eliminate Compiler Warnings

## Overview
This track addresses and eliminates all compiler warnings generated during the build process (`make clean && make`). This includes warnings from `gcc` regarding string safety, unused variables, and missing initializers, as well as `nvcc` warnings regarding deprecated functions and architecture support.

## Functional Requirements

### 1. String Safety and Truncation
- In `src/constellation.c`, replace `strncpy` with `snprintf` or equivalent safe logic to ensure null-termination and eliminate truncation warnings.

### 2. Unused Variables and Parameters
- Review and remove redundant unused variables in `src/ephemerides.c` and `src/zodiacal_math.h`.
- For unused parameters required by function signatures (e.g., in `src/zodiacal_math.h`), mark them explicitly using `(void)param;` or appropriate attributes to suppress warnings while maintaining the interface.

### 3. Missing Initializers
- In `src/main.c`, fix the missing initializer warning for the `ConstellationBoundary` struct by providing a complete or explicit zero-initialization.

### 4. CUDA Modernization
- In `src/render_cuda.cu`, replace the deprecated `cudaMemcpyToArray` function with a modern equivalent (e.g., `cudaMemcpy2DToArray` or `cudaMemcpy3D`).
- Update the `Makefile` to target a more modern CUDA compute capability (e.g., `sm_75` or higher) to eliminate the architecture support warnings.
- Document the new compute capability flag in the `Makefile` and `README.md`.

## Non-Functional Requirements
- **Build Hygiene**: The primary goal is a "clean build" with zero warnings under `-Wall -Wextra`.
- **Maintainability**: The code should remain readable and idiomatic after the changes.

## Acceptance Criteria
- [ ] Running `make clean && make` produces no warnings from either `gcc` or `nvcc`.
- [ ] The `Makefile` and `README.md` are updated with documentation regarding the targeted CUDA architecture.
- [ ] The application remains fully functional and passes all existing tests.
