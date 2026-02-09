# Specification - Celestial Body Labeling

## Overview
This feature adds text labels for major celestial bodies (Sun, Moon, and Planets) to the rendered output. This aids in identification, particularly for planets that appear as points similar to stars.

## Functional Requirements
- **CLI Arguments**:
    - `--label-bodies` (or `-j`): Enable labeling for the Sun, Moon, and five planets (Mercury, Venus, Mars, Jupiter, Saturn).
    - `--label-color <hex>`: Set the color for these labels (default: `FF0000` / Bright Red).
- **Label Content**: The label should display the name of the body in plain text.
- **Positioning**: 
    - The label should start exactly 8 pixels to the right of the body's computed screen center.
    - The text should be vertically centered relative to the body's center point (consistent with the 4-pixel offset used in `draw_label_centered`).
- **Rendering**:
    - Use the existing `draw_char` / `draw_label` infrastructure.
    - Support both standard pinhole projection and the cylindrical environment map (`--env`).
    - Labels should be rendered in the post-processing phase, after tone mapping but before output.
- **Visibility**:
    - Labels are only drawn if the body is above the horizon.
    - Labels should be clipped to the image boundaries.

## Acceptance Criteria
- Running `./knight --label-bodies` renders labels for visible planets/sun/moon in red.
- Running `./knight --label-bodies --label-color 00FFFF` renders labels in cyan.
- Labels remain correctly positioned 8px to the right when switching between `--env` and standard views.
- Labels for bodies below the horizon are not rendered.
