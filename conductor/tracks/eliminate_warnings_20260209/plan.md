# Implementation Plan - Eliminate Compiler Warnings

This plan outlines the steps to resolve all compiler warnings from `gcc` and `nvcc` to achieve a clean build environment.

## Phase 1: GCC Warning Resolution
- [x] Task: Fix string truncation warning in `src/constellation.c` by replacing `strncpy` with `snprintf`. 0076366
- [~] Task: Remove or mark unused variables in `src/ephemerides.c`.
- [ ] Task: Resolve missing initializer warning in `src/main.c` for `ConstellationBoundary`.
- [ ] Task: Handle unused parameters in `src/zodiacal_math.h`.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: GCC Warning Resolution' (Protocol in workflow.md)

## Phase 2: CUDA & Build System Modernization
- [ ] Task: Replace deprecated `cudaMemcpyToArray` with `cudaMemcpy2DToArray` in `src/render_cuda.cu`.
- [ ] Task: Update `Makefile` to target CUDA compute capability `sm_75` (or higher) and document the flag.
- [ ] Task: Update `README.md` to document the required CUDA compute capability.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: CUDA & Build System Modernization' (Protocol in workflow.md)

## Phase 3: Final Verification
- [ ] Task: Run a full `make clean && make` and verify zero warnings.
- [ ] Task: Execute all existing tests to ensure no regressions were introduced.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Final Verification' (Protocol in workflow.md)
