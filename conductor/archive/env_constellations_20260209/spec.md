# Specification - Constellation Outlines in Environment Maps

## Overview
Currently, constellation outlines and labels are only supported in standard pinhole camera views. This feature extends support to the cylindrical environment map mode (`-E`), ensuring accurate projections, proper azimuth wrap-around, and correct curvature for boundary lines.

## Functional Requirements
- **Projection Support**: Update `draw_constellation_outlines` and `draw_constellation_labels` to use the equirectangular/cylindrical projection mapping when `env_map` is enabled.
- **Line Subdivision (Tessellation)**:
    - Boundary segments between vertices can span large angular distances. 
    - These must be subdivided into smaller segments (e.g., every 1-2 degrees) before projection to ensure they accurately follow the curved geometry of the environment map.
- **Azimuth Wrap-around**:
    - Lines crossing the 360° -> 0° transition must be split into two distinct pixel-space segments: one ending at the right edge and one beginning at the left edge.
    - Labels (3-letter abbreviations) must also support wrapping; if a label's character block extends past the image width, the overflowing pixels must appear on the opposite side.
- **Horizon Clipping**:
    - Maintain existing horizon clipping logic ($alt = 0$) to ensure outlines do not appear below the ground in the environment map.
- **Integration**:
    - Remove the `!cfg.env_map` restriction in `src/main.c` that currently prevents these overlays in environment mode.

## Non-Functional Requirements
- **Performance**: Line subdivision should be balanced to provide smooth curves without excessive overhead during the overlay phase.
- **Accuracy**: Labels must remain centered at the correct astronomical coordinates relative to the stars in the environment map.

## Acceptance Criteria
- Running `./knight -E --outline` produces an HDR/PNG image where constellation boundaries are visible.
- Constellations spanning the 0°/360° meridian (e.g., Pisces or Pegasus) are drawn as continuous lines wrapping across the image edges.
- Labels near the meridian wrap correctly without being cut off.
- Outlines follow the natural curvature of the cylindrical projection.
