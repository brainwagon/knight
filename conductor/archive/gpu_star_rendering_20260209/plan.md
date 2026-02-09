# Implementation Plan - GPU-Accelerated Star Rendering

This plan outlines the steps to migrate star rendering to the GPU using CUDA, simplifying the PSF calculation for performance and maintaining parity between CPU and GPU modes.

## Phase 1: PSF Simplification & CPU Refactor [checkpoint: a45ee98]
- [x] Task: Update `render_stars` in `src/stars.c` to use a fixed 550nm wavelength for PSF Gaussian size calculation instead of looping over spectral bands for the sigma. [b3e5ed8]
- [x] Task: Update `tests/test_psf.c` to verify the simplified PSF logic on the CPU. [b3e5ed8]
- [x] Task: Conductor - User Manual Verification 'Phase 1: PSF Simplification & CPU Refactor' (Protocol in workflow.md) [a45ee98]

## Phase 2: CUDA Star Data Management [checkpoint: 56c4395]
- [x] Task: Add `Star` device buffer management (allocation, copying, and cleanup) to `src/render_cuda.cu`. [d573aa3]
- [x] Task: Update `src/cuda_host.h` to include declarations for `cuda_upload_stars` and `cuda_render_stars`. [d573aa3]
- [x] Task: Implement `cuda_upload_stars` in `src/render_cuda.cu` to transfer star data to the GPU. [d573aa3]
- [x] Task: Write a test in `tests/test_cuda_stars.c` to verify successful star data transfer to the GPU. [d573aa3]
- [x] Task: Conductor - User Manual Verification 'Phase 2: CUDA Star Data Management' (Protocol in workflow.md) [56c4395]

## Phase 3: GPU Star Rendering Kernel [checkpoint: e1e52c8]
- [x] Task: Implement `render_stars_kernel` in `src/render_cuda.cu` using the simplified 550nm PSF logic and `atomicAdd` for accumulation into the HDR buffer. [475ea37]
- [x] Task: Implement the `cuda_render_stars` host function to configure and launch the star rendering kernel. [475ea37]
- [x] Task: Create a parity test in `tests/test_gpu_stars.c` that renders a small set of stars in both CPU and GPU modes and asserts that the resulting HDR values are consistent. [475ea37]
- [x] Task: Conductor - User Manual Verification 'Phase 3: GPU Star Rendering Kernel Implementation' (Protocol in workflow.md) [e1e52c8]

## Phase 4: Full Integration & Optimization [checkpoint: 5488ea7]
- [x] Task: Update `src/main.c` to invoke `cuda_upload_stars` and `cuda_render_stars` when the renderer is in GPU mode. [0502cd5]
- [x] Task: Refactor `src/main.c` to ensure star coordinates are converted to horizon space before being passed to the GPU renderer. [0502cd5]
- [x] Task: Perform manual verification by rendering a Tycho-2 scene with thousands of stars and comparing CPU vs. GPU performance and visual parity. [0502cd5]
- [x] Task: Conductor - User Manual Verification 'Phase 4: Full Integration & Optimization' (Protocol in workflow.md) [5488ea7]
