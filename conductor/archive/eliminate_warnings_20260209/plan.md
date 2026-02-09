# Implementation Plan - Eliminate Compiler Warnings

This plan outlines the steps to resolve all compiler warnings from `gcc` and `nvcc` to achieve a clean build environment.

## Phase 1: GCC Warning Resolution [checkpoint: 70fe207]
- [x] Task: Fix string truncation warning in `src/constellation.c` by replacing `strncpy` with `snprintf`. 0076366
- [x] Task: Remove or mark unused variables in `src/ephemerides.c`. 1952093
- [x] Task: Resolve missing initializer warning in `src/main.c` for `ConstellationBoundary`. c61db6d
- [x] Task: Handle unused parameters in `src/zodiacal_math.h`. 70fe207
- [x] Task: Conductor - User Manual Verification 'Phase 1: GCC Warning Resolution' (Protocol in workflow.md) 70fe207

## Phase 2: CUDA & Build System Modernization [checkpoint: 70fe207]
- [x] Task: Replace deprecated `cudaMemcpyToArray` with `cudaMemcpy2DToArray` in `src/render_cuda.cu`. 70fe207
- [x] Task: Update `Makefile` to target CUDA compute capability `sm_75` (or higher) and document the flag. 70fe207
- [x] Task: Update `README.md` to document the required CUDA compute capability. 70fe207
- [x] Task: Conductor - User Manual Verification 'Phase 2: CUDA & Build System Modernization' (Protocol in workflow.md) 70fe207

## Phase 3: Final Verification [checkpoint: b051e12]
- [x] Task: Run a full `make clean && make` and verify zero warnings. b051e12
- [x] Task: Execute all existing tests to ensure no regressions were introduced. b051e12
- [x] Task: Conductor - User Manual Verification 'Phase 3: Final Verification' (Protocol in workflow.md) b051e12
