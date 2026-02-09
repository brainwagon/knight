# Implementation Plan - Constellation Outlines in Environment Maps

## Phase 1: Projection & Subdivision Math
- [x] Task: Implement `project_vertex_env` or update `project_vertex` to support cylindrical mapping. 7087796
- [x] Task: Implement a spherical subdivision utility to split long boundary segments into smaller points. 7087796
- [x] Task: Write unit tests in `tests/test_env_proj.c` to verify subdivision accuracy and projection logic. 7087796
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Projection & Subdivision Math' (Protocol in workflow.md)

## Phase 2: Outlines & Wrap-around Logic
- [x] Task: Update `draw_constellation_outlines` to use subdivided segments in environment map mode. 7087796
- [x] Task: Implement line-splitting logic for segments crossing the 0°/360° (meridian) boundary. 7087796
- [x] Task: Write failing unit tests for meridian wrap-around line segments. 7087796
- [x] Task: Implement the wrap-around logic to pass the tests. 7087796
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Outlines & Wrap-around Logic' (Protocol in workflow.md)

## Phase 3: Labels & Character Wrapping
- [x] Task: Update `draw_char` to support horizontal wrap-around (modulo width) in pixel space. 7087796
- [x] Task: Update `draw_constellation_labels` to support environment map projection. 7087796
- [x] Task: Write unit tests for label wrapping near the image edges. 7087796
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Labels & Character Wrapping' (Protocol in workflow.md)

## Phase 4: Integration & Final Verification
- [x] Task: Remove `!cfg.env_map` restriction in `src/main.c` for constellation overlays. 7087796
- [x] Task: Perform manual verification renders of constellations known to cross the meridian (e.g., Pisces). 7087796
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Integration & Final Verification' (Protocol in workflow.md)
