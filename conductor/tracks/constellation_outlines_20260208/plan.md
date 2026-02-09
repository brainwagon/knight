# Implementation Plan - Constellation Outlines Rendering

## Phase 1: Data Infrastructure
- [ ] Task: Create data structures to hold constellation boundary vertices.
- [ ] Task: Implement a parser for `data/bound_in_20.txt`.
- [ ] Task: Verify parsing by printing the number of vertices and constellations loaded.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Data Infrastructure' (Protocol in workflow.md)

## Phase 2: Coordinate Transformation & Logic
- [ ] Task: Implement J2000 to Horizontal coordinate transformation for boundary vertices.
- [ ] Task: Add logic to determine visibility (above horizon, within FOV) for line segments.
- [ ] Task: Add the `--outline` flag to argument parsing in `main.c`.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Coordinate Transformation & Logic' (Protocol in workflow.md)

## Phase 3: Rendering Implementation
- [ ] Task: Implement a simple line-drawing algorithm (e.g., Bresenham or simple sampling) for the image buffer.
- [ ] Task: Integrate boundary rendering into the post-processing pipeline (in `tonemap.c` or after it).
- [ ] Task: Ensure outlines are drawn after tone mapping as requested.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Rendering Implementation' (Protocol in workflow.md)

## Phase 4: Verification & Refinement
- [ ] Task: Test with various FOVs and orientations to ensure correct alignment with stars.
- [ ] Task: Verify that the horizon clipping works correctly.
- [ ] Task: Conductor - User Manual Verification 'Phase 4: Verification & Refinement' (Protocol in workflow.md)
