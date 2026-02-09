# Product Guidelines

## Documentation and Communication
- **Technical Precision**: Use precise mathematical and physical terminology in documentation and code comments. Favor rigorous descriptions of algorithms and physical models over simplified analogies.
- **Clarity and Context**: Comments should explain the *why* behind complex physical formulas or astronomical calculations, referencing the underlying theory or research paper where applicable.

## Rendering and Visual Standards
- **Physically-Based Accuracy**: Adhere strictly to the principles of Physically-Based Rendering (PBR). Ensure energy conservation in scattering models and use accurate spectral data for light sources.
- **HDR Pipeline**: Maintain a High Dynamic Range (HDR) workflow. All internal calculations must preserve physical radiance values. Tone mapping, exposure control, and perceptual effects (like the Purkinje shift) should only be applied as a post-processing step.
- **Scotopic Realism**: Prioritize the visual experience of a human observer under low-light conditions. This includes accurate simulation of rod-based vision, stellar bloom, and appropriate color shifts in the dark.

## Mathematical and Physical Consistency
- **Standardized SI Units**: Use International System of Units (SI) consistently throughout the codebase (e.g., meters, seconds, Watts/sr/m²/nm). This ensures compatibility between different modules (atmosphere, ephemerides, etc.).
- **Consistent Coordinate Systems**: Use clearly defined and documented coordinate systems (e.g., Horizontal, Equatorial) and ensure transformations between them are explicit and well-tested.
