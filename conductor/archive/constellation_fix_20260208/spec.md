# Specification - Constellation Rendering Fixes

## Overview
This track addresses two major rendering issues with constellations:
1. **Outline Clipping & Artifacts**: Constellation outlines only appear in the top 1/4 of the image and have a "dashed" appearance.
2. **Missing Labels**: Constellation labels (3-letter abbreviations) are entirely missing from the output.

## Functional Requirements
- **Projection Logic Audit**: Investigate and fix the coordinate transformation and projection logic in `src/constellation.c`. The fact that clipping is constant regardless of camera orientation suggests an error in how screen coordinates (`px`, `py`) are derived or validated.
- **Improved Line Drawing**: 
    - Fix the dashed appearance by auditing the Bresenham implementation in `draw_line`.
    - Implement sub-pixel sampling or ensure robust float-to-int rounding to ensure continuous segments.
- **Label Visibility & Rendering Fix**:
    - Identify why labels are missing. Check centroid calculation, visibility logic (`l->alt < 0`), and the `draw_char` bitmap indexing.
    - Ensure labels are rendered at the correct projected screen coordinates.

## Non-Functional Requirements
- **Performance**: Maintain efficient rendering as these post-processing steps occur after the main HDR pass.
- **Accuracy**: Outlines and labels must align perfectly with the rendered stars.

## Acceptance Criteria
- Constellation outlines are rendered across the entire valid field of view, not just the top.
- Outlines appear as continuous lines rather than dashed segments.
- 3-letter labels appear centered at the geometric centroids of visible constellations.
- Alignment is verified to be correct across various camera altitudes and azimuths.
