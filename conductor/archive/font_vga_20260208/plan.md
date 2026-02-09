# Implementation Plan - Font Rendering Fix and IBM VGA Replacement

## Phase 1: Font Substitution & Diagnostic
- [x] Task: Locate and convert IBM VGA 8x8 bitmap data into a static C array format. 59634e5
- [x] Task: Replace the content of `src/font8x8.h` with the new VGA data (ASCII 32-126). 59634e5
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Font Substitution & Diagnostic' (Protocol in workflow.md) [checkpoint: 59634e5]

## Phase 2: Rendering Logic Correction
- [x] Task: Refactor `draw_char` in `src/constellation.c` to use MSB-first bit indexing (e.g., `0x80 >> col`). bdcdb5a
- [x] Task: Verify character orientation by rendering a simple test string (e.g., "Abc123") in a diagnostic pass. bdcdb5a
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Rendering Logic Correction' (Protocol in workflow.md) [checkpoint: bdcdb5a]

## Phase 3: Integration & Final Verification
- [x] Task: Re-compile the main renderer and generate an output with `--outline`. bdcdb5a
- [x] Task: Confirm that all 88 constellation labels are legible and correctly oriented. bdcdb5a
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Integration & Final Verification' (Protocol in workflow.md) [checkpoint: bdcdb5a]