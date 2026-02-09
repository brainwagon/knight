# Implementation Plan - Consistent Star Rendering

## Phase 1: Math Foundation & Configuration [checkpoint: 8d167aa]
- [x] Task: Add `--aperture` argument to CLI configuration. 30bf817
    - [x] Sub-task: Update `config` struct in `main.c` to hold aperture size (default 6.0mm).
    - [x] Sub-task: Parse `-a` / `--aperture` flag.
    - [x] Sub-task: TDD - Write test to verify argument parsing.
- [x] Task: Implement analytic Gaussian integration function. 5e623c2
    - [x] Sub-task: Create a math utility function `integrate_gaussian_2d` in `core.c` (or similar) that calculates the integral of a Gaussian over a rectangular pixel area.
    - [x] Sub-task: TDD - Write unit tests to verify integration over infinite bounds equals 1.0 (normalized), symmetry, and correct scaling with sigma.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Math Foundation & Configuration' (Protocol in workflow.md)

## Phase 2: Spectral PSF Rendering Logic
- [x] Task: Refactor Star Rendering to use PSF. 1e34523
    - [x] Sub-task: Update `stars.c` to use the new physics-based approach.
    - [x] Sub-task: Calculate angular size per spectral band: `theta = 1.22 * lambda / aperture`.
    - [x] Sub-task: Convert angular size to pixel coordinates (sigma in pixels) based on current resolution/FOV.
    - [x] Sub-task: Apply `integrate_gaussian_2d` for each pixel in the star's bounding box.
    - [x] Sub-task: TDD - Write a test that simulates rendering a star at two different resolutions (e.g., 100x100 vs 200x200), summing the total pixel values. The total flux should be consistent.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Spectral PSF Rendering Logic' (Protocol in workflow.md)

## Phase 3: Integration & Verification
- [ ] Task: Integrate new rendering logic into the main render loop.
    - [ ] Sub-task: Replace the old point/circle drawing code in `stars.c` with the new PSF logic.
    - [ ] Sub-task: Ensure performance is acceptable (optimize `erf` calls if necessary).
- [ ] Task: Verify Resolution Independence.
    - [ ] Sub-task: Generate test outputs at 1920x1080 and 960x540.
    - [ ] Sub-task: Manual visual inspection to confirm stars look consistent in brightness and size relative to the frame.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Integration & Verification' (Protocol in workflow.md)
