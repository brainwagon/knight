# Implementation Plan - Constellation Labeling

## Phase 1: Font & Basic Text Rendering
- [x] Task: Hardcode an 8x8 bitmap font for alphanumeric characters. ede39f9
- [x] Task: Implement a function to render a single character into the image buffer. 67d1bf2
- [x] Task: Implement a function to render a 3-letter string centered at a specific pixel. 67d1bf2
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Font & Basic Text Rendering' (Protocol in workflow.md) [checkpoint: 67d1bf2]

## Phase 2: Centroid Calculation & Logic
- [x] Task: Calculate geometric centroids (average RA/DEC) for all 88 constellations during data loading. 4fc6e0c
- [x] Task: Update the `ConstellationBoundary` structure to store these centroids. 4fc6e0c
- [x] Task: Implement coordinate transformation (Equatorial to Horizontal) for the centroids. 4fc6e0c
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Centroid Calculation & Logic' (Protocol in workflow.md) [checkpoint: 4fc6e0c]

## Phase 3: Integration & Rendering
- [x] Task: Integrate label rendering into the post-processing pipeline (after outlines). 7085c4d
- [x] Task: Implement visibility and projection logic for the centroids. 7085c4d
- [x] Task: Ensure labels are centered correctly and rendered in a subtle color. 7085c4d
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Integration & Rendering' (Protocol in workflow.md) [checkpoint: 7085c4d]

## Phase 4: Verification & Refinement
- [~] Task: Verify that labels appear correctly for various constellations and orientations.
- [ ] Task: Refine label positioning if necessary to ensure legibility.
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Verification & Refinement' (Protocol in workflow.md)
