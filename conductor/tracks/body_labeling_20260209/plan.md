# Implementation Plan - Celestial Body Labeling

## Phase 1: Configuration & Infrastructure [checkpoint: 4346f66]
- [x] Task: Update `Config` struct in `src/config.h` to include `label_bodies` and `label_color`. bc371f9
- [x] Task: Update `parse_args` in `src/config.c` to handle `--label-bodies` (`-j`) and `--label-color`. bc371f9
- [x] Task: Initialize defaults for new configuration options in `src/main.c`. bc371f9
- [x] Task: Update `print_help` in `src/config.c` to include the new flags. bc371f9
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Configuration & Infrastructure' (Protocol in workflow.md)

## Phase 2: Core Rendering Logic
- [x] Task: Implement `draw_body_label` helper function to handle 8px offset and clipping. f35bf98
- [x] Task: Write unit tests in `tests/test_labels.c` for label positioning and clipping logic. f35bf98
- [x] Task: Integrate planet labeling into the main loop in `src/main.c`. f35bf98
- [x] Task: Integrate Sun and Moon labeling into the main loop in `src/main.c`. f35bf98
- [x] Task: Ensure labels are rendered after tone mapping. f35bf98
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Core Rendering Logic' (Protocol in workflow.md)

## Phase 3: Environment Map Support & Verification
- [x] Task: Implement label projection for cylindrical environment maps. f35bf98
- [x] Task: Verify label alignment in both pinhole and environment map views. f35bf98
- [x] Task: Perform final manual verification with various times/locations to ensure correct visibility. f35bf98
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Environment Map Support & Verification' (Protocol in workflow.md)
