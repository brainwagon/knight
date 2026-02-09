# Specification - Constellation Labeling

## Overview
This track adds text labels to constellations when the `--outline` flag is enabled. Each constellation will be labeled with its 3-letter abbreviation (e.g., "Ori") at its geometric centroid.

## Functional Requirements
- **Automatic Activation**: Labels are rendered whenever the `--outline` flag is present.
- **Centroid Calculation**: For each constellation, calculate the geometric centroid by averaging the RA/DEC coordinates of all its boundary vertices.
- **Coordinate Transformation**: Convert the centroid coordinates to Horizontal (Az/Alt) and then to screen coordinates.
- **Visibility**: Labels are only drawn if their centroid is above the horizon and within the camera's FOV.
- **Font Rendering**: Use a hardcoded 8x8 bitmap font.
- **Label Alignment**: The 3-letter abbreviation should be centered around the calculated screen position.

## Non-Functional Requirements
- **No External Dependencies**: The font data must be hardcoded within the source code (8x8 bitmap).
- **Legibility**: Labels should be clear and rendered in a subtle color (e.g., matching the outlines) to avoid overwhelming the view.

## Acceptance Criteria
- Running `./knight --outline` displays 3-letter labels at the center of visible constellations.
- Labels are correctly centered at the geometric centroid of the boundary polygons.
- Labels are only rendered when their centroid is above the horizon.
