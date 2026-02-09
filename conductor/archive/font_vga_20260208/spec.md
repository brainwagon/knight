# Specification - Font Rendering Fix and IBM VGA Replacement

## Overview
This track addresses the left-to-right character reversal bug in the constellation labels and replaces the current minimal font with a complete, high-quality IBM VGA 8x8 bitmap font.

## Functional Requirements
- **Font Replacement**: Substitute the existing font in `src/font8x8.h` with the standard IBM VGA 8x8 bitmap font.
- **Extended Character Set**: Support the full printable ASCII range (32-126), including uppercase, lowercase, numbers, and common punctuation.
- **Bit-Order Fix**: Correct the bit-indexing logic in `draw_char` to resolve the horizontal reversal. The current implementation likely treats bits as Least-Significant-Bit (LSB) first, whereas standard bitmap fonts (like VGA) are typically Most-Significant-Bit (MSB) first.
- **Verification**: Use the existing constellation label rendering path to confirm that abbreviations (e.g., "Ori", "UMa") are legible and correctly oriented.

## Non-Functional Requirements
- **No External Dependencies**: The IBM VGA font data must be hardcoded as a static array in `src/font8x8.h`.
- **Maintain Performance**: Ensure the bit-shifting logic remains efficient for real-time post-processing.

## Acceptance Criteria
- Constellation labels display correctly without character reversal.
- All characters (A-Z, a-z, 0-9) are rendered using the IBM VGA 8x8 glyphs.
- The `--outline` flag correctly triggers the display of the new font in the renderer output.
