# Implementation Plan - Constellation Outlines Rendering

## Phase 1: Data Infrastructure
- [x] Task: Create data structures to hold constellation boundary vertices. 8ca281d
- [x] Task: Implement a parser for `data/bound_in_20.txt`. 3fefb07
- [x] Task: Verify parsing by printing the number of vertices and constellations loaded. 91083ec
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Data Infrastructure' (Protocol in workflow.md) [checkpoint: 91083ec]

## Phase 2: Coordinate Transformation & Logic
- [x] Task: Implement J2000 to Horizontal coordinate transformation for boundary vertices. 1049ba0
- [x] Task: Add logic to determine visibility (above horizon, within FOV) for line segments. 86634dd
- [x] Task: Add the `--outline` flag to argument parsing in `main.c`. 86634dd
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Coordinate Transformation & Logic' (Protocol in workflow.md) [checkpoint: 86634dd]

## Phase 3: Rendering Implementation
- [x] Task: Implement a simple line-drawing algorithm (e.g., Bresenham or simple sampling) for the image buffer. 86634dd
- [x] Task: Integrate boundary rendering into the post-processing pipeline (in `tonemap.c` or after it). 85d144c
- [x] Task: Ensure outlines are drawn after tone mapping as requested. 85d144c
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Rendering Implementation' (Protocol in workflow.md) [checkpoint: 85d144c]

## Phase 4: Verification & Refinement
- [x] Task: Test with various FOVs and orientations to ensure correct alignment with stars. 29d472e
- [x] Task: Verify that the horizon clipping works correctly. 29d472e
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Verification & Refinement' (Protocol in workflow.md) [checkpoint: 29d472e]
