# Specification: GPU-Accelerated Star Rendering

## Overview
This track aims to significantly improve the performance of star rendering by migrating the computation to the GPU using CUDA. The current CPU-based implementation is a bottleneck, especially when rendering millions of stars from the Tycho-2 catalog. We will also simplify the Point Spread Function (PSF) calculation by using a fixed wavelength (550nm) for the Gaussian size, ensuring parity between CPU and GPU implementations.

## Functional Requirements

### 1. GPU Star Rendering Kernel
- Implement a CUDA kernel `render_stars_kernel` in `src/render_cuda.cu`.
- The kernel should take an array of `Star` data, camera parameters, and the output HDR buffer.
- Each thread (or block) should process a subset of stars or pixels. Given the sparse nature of stars, a thread-per-star approach rasterizing onto the buffer using atomic adds is likely most efficient, or a hybrid tile-based approach. **Decision:** We will start with a thread-per-star approach using `atomicAdd` for the accumulation buffer, as it's simpler to implement and effective for sparse points.
- **Parity:** The GPU implementation must match the visual output of the CPU implementation (subject to floating-point differences).

### 2. Star Data Management
- Update `cuda_render_frame` (or add a new function `cuda_render_stars`) to accept the `Star` array.
- Allocate device memory for star data and copy the host `Star` array to the device. This upload should happen once or only when the star list changes (which is static in this app).

### 3. PSF Simplification (CPU & GPU)
- Refactor the existing `render_stars` in `src/stars.c` to use a single Gaussian size based on a fixed wavelength of 550nm for all spectral bands.
- Remove the per-band loop for `sigma_px` calculation.
- Apply this same simplified logic in the CUDA kernel.

### 4. Integration
- Update `src/main.c` to call the GPU star renderer when `--mode gpu` is active.
- Ensure the stars are rendered *additively* into the existing HDR buffer (which already contains the atmosphere).

## Non-Functional Requirements
- **Performance**: The GPU implementation should be orders of magnitude faster than the CPU version for large star counts.
- **Memory**: 2.5 million stars take ~80MB. This fits comfortably in VRAM.
- **Atomic Safety**: Use `atomicAdd` to prevent race conditions when multiple stars overlap the same pixel.

## Acceptance Criteria
- [ ] `render_stars` on CPU uses a fixed 550nm wavelength for PSF size.
- [ ] A new `render_stars_kernel` exists and functions correctly.
- [ ] The application successfully renders stars on the GPU when `--mode gpu` is selected.
- [ ] Visual output between CPU and GPU modes is consistent.
- [ ] Performance is demonstrably improved in GPU mode.

## Out of Scope
- Moving bloom/glare to GPU.
- Dynamic streaming of star data (upload once is sufficient).
