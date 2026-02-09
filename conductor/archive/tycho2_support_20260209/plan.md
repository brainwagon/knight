# Implementation Plan - Tycho2 Star Catalog Support

This plan covers the integration of the Tycho-2 star catalog into the Knight rendering engine, including CLI configuration, universal magnitude filtering, and the implementation of the Tycho-2 data parser.

## Phase 1: Configuration and CLI Updates [checkpoint: 895e39d]
- [x] Task: Update `Config` struct in `src/config.h` to include `use_tycho` (bool), `tycho_dir` (char*), and `star_mag_limit` (float). [61a15b9]
- [x] Task: Update `parse_args` and `long_options` in `src/config.c` to support `--tycho`, `--tycho-dir`, and `--mag-limit` (with `-m` as short flag). [61a15b9]
- [x] Task: Update `print_help` in `src/config.c` to document the new options. [61a15b9]
- [x] Task: Update `main.c` to initialize default values for the new configuration parameters. [61a15b9]
- [x] Task: Write failing tests in `tests/test_config.c` to verify the new flags are parsed correctly. [61a15b9]
- [x] Task: Implement changes in `config.c` to pass the tests. [61a15b9]
- [x] Task: Conductor - User Manual Verification 'Phase 1: Configuration and CLI Updates' (Protocol in workflow.md) [895e39d]

## Phase 2: Magnitude Filtering Refactor [checkpoint: d031d5d]
- [x] Task: Update `load_stars` signature in `src/stars.h` to accept `float mag_limit`. [3ddd305]
- [x] Task: Update `load_stars` implementation in `src/stars.c` to filter stars based on `mag_limit` during parsing. [3ddd305]
- [x] Task: Update `main.c` to pass `cfg.star_mag_limit` to `load_stars`. [3ddd305]
- [x] Task: Write a test in a new file `tests/test_mag_filter.c` that verifies only stars brighter than the limit are loaded from a sample YBSC5 file. [3ddd305]
- [x] Task: Conductor - User Manual Verification 'Phase 2: Magnitude Filtering Refactor' (Protocol in workflow.md) [d031d5d]

## Phase 3: Tycho-2 Parser Implementation
- [x] Task: Declare `load_stars_tycho(const char* dirpath, float mag_limit, Star** stars)` in `src/stars.h`.
- [x] Task: Implement `load_stars_tycho` in `src/stars.c` to iterate through `tyc2.dat.00` to `tyc2.dat.19`.
- [x] Task: Implement the Tycho-2 line parser using fixed-byte offsets for RA, Dec, BTmag, and VTmag.
- [x] Task: Implement the magnitude and color index transformations in the parser.
- [~] Task: Write unit tests in `tests/test_tycho_load.c` using a small mock Tycho-2 data file to verify parsing accuracy.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Tycho-2 Parser Implementation' (Protocol in workflow.md)

## Phase 4: Integration and Final Verification [checkpoint: 7efc496]
- [x] Task: Update `main.c` to call `load_stars_tycho` if `cfg.use_tycho` is true, otherwise call `load_stars`. [f52816a]
- [x] Task: Update `main.c` to pass `cfg.tycho_dir` when loading Tycho-2. [f52816a]
- [x] Task: Perform manual verification by rendering a scene with `--tycho` and comparing it to the default YBSC5 output. [f52816a]
- [x] Task: Verify that the `--mag-limit` flag works as expected with the Tycho-2 catalog. [f52816a]
- [x] Task: Conductor - User Manual Verification 'Phase 4: Integration and Final Verification' (Protocol in workflow.md) [7efc496]
