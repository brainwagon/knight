# Implementation Plan - Celestial Body Labeling

## Phase 1: Configuration & Infrastructure
- [x] Task: Update `Config` struct in `src/config.h` to include `label_bodies` and `label_color`. bc371f9
- [x] Task: Update `parse_args` in `src/config.c` to handle `--label-bodies` (`-j`) and `--label-color`. bc371f9
- [x] Task: Initialize defaults for new configuration options in `src/main.c`. bc371f9
- [x] Task: Update `print_help` in `src/config.c` to include the new flags. bc371f9
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Configuration & Infrastructure' (Protocol in workflow.md)

## Phase 2: Core Rendering Logic
- [ ] Task: Implement `draw_body_label` helper function to handle 8px offset and clipping.
- [ ] Task: Write unit tests in `tests/test_labels.c` for label positioning and clipping logic.
- [ ] Task: Integrate planet labeling into the main loop in `src/main.c`.
- [ ] Task: Integrate Sun and Moon labeling into the main loop in `src/main.c`.
- [ ] Task: Ensure labels are rendered after tone mapping.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Core Rendering Logic' (Protocol in workflow.md)

## Phase 3: Environment Map Support & Verification
- [ ] Task: Implement label projection for cylindrical environment maps.
- [ ] Task: Verify label alignment in both pinhole and environment map views.
- [ ] Task: Perform final manual verification with various times/locations to ensure correct visibility.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Environment Map Support & Verification' (Protocol in workflow.md)
