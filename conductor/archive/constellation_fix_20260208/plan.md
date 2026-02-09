# Implementation Plan - Constellation Rendering Fixes

## Phase 1: Diagnostic & Projection Audit
- [x] Task: Create a diagnostic test case with a fixed orientation to dump projected coordinates of a known constellation (e.g., Ursa Major). 59634e5
- [x] Task: Audit `project_vertex` in `src/constellation.c` for mathematical errors in screen coordinate mapping. 59634e5
- [x] Task: Audit `constellation_equ_to_horizon` for potential issues with sidereal time or hour angle calculations affecting boundary vertices. 59634e5
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Diagnostic & Projection Audit' (Protocol in workflow.md) [checkpoint: 59634e5]

## Phase 2: Line Drawing & Visibility Fix
- [x] Task: Refactor `draw_line` in `src/constellation.c` to ensure continuous segments and fix the "dashed" appearance. bdcdb5a
- [x] Task: Implement sub-pixel rounding for line endpoints. bdcdb5a
- [x] Task: Fix visibility logic to ensure lines are not prematurely clipped by vertical coordinate bounds. bdcdb5a
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Line Drawing & Visibility Fix' (Protocol in workflow.md) [checkpoint: bdcdb5a]

## Phase 3: Label Rendering Fix
- [x] Task: Audit `draw_constellation_labels` for visibility and projection errors. bdcdb5a
- [x] Task: Debug `draw_char` and `font8x8_basic` indexing to ensure bitmap data is correctly accessed and rendered. bdcdb5a
- [x] Task: Verify centroid calculation accuracy for complex/concave constellations. bdcdb5a
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Label Rendering Fix' (Protocol in workflow.md) [checkpoint: bdcdb5a]

## Phase 4: Final Verification
- [x] Task: Verify continuous outlines and visible labels across full image at various Alt/Az. bdcdb5a
- [x] Task: Ensure correct alignment with stars in high-FOV shots. bdcdb5a
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Final Verification' (Protocol in workflow.md) [checkpoint: bdcdb5a]