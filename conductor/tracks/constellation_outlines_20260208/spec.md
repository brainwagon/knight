# Specification - Constellation Outlines Rendering

## Overview
This track implements the rendering of IAU constellation boundaries. The boundaries are defined as polygons in spherical coordinates (RA/DEC J2000) stored in `data/bound_in_20.txt`.

## Functional Requirements
- **CLI Argument**: Add a new flag `--outline` (or `-O`) to the `knight` executable.
- **Data Parsing**: Parse `data/bound_in_20.txt`. Each line contains RA (decimal hours), DEC (decimal degrees), and a 3-letter constellation abbreviation.
- **Coordinate Transformation**: Convert J2000 RA/DEC coordinates to the current observer's horizontal coordinates (Azimuth/Altitude) based on the simulation time and location.
- **Rendering**: 
    - Draw lines between consecutive vertices of the same constellation polygon.
    - The outlines should be superimposed on the image.
    - Since the request specifies "after tone mapping is complete", the drawing should occur in the post-processing/tonemapping phase or as a separate overlay step on the final image buffer.
- **Clipping**: Correctly handle lines that cross the horizon or the edges of the field of view.

## Non-Functional Requirements
- **Performance**: Parsing the boundary file should be efficient and ideally done once.
- **Visuals**: Outlines should be clear but not overwhelm the celestial bodies. A subtle, fixed intensity or color (e.g., dim red or grey) is likely appropriate.

## Acceptance Criteria
- Running `./knight --outline` produces an image with visible constellation boundaries.
- Boundaries correctly align with the rendered stars.
- Boundaries are not rendered below the horizon.
