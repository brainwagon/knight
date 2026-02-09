# Specification: Consistent Star Rendering via Spectral PSF

## 1. Overview
The current star rendering implementation exhibits inconsistent brightness and apparent size when changing output resolutions. Bright stars appear as large circles at high resolutions (1080p) but diminish significantly at lower resolutions (540p). This track aims to implement a physically based rendering approach for stars using an analytic Point Spread Function (PSF) based on the diffraction limit of an observer's aperture (Airy disk).

## 2. Functional Requirements

### 2.1 Analytic Gaussian Convolution
- **Core Logic**: Treat stars as point sources modulated by a Gaussian PSF.
- **Integration**: Instead of simple point sampling or rasterization, calculate the exact amount of light a star contributes to a pixel by analytically integrating the 2D Gaussian over the pixel's rectangular area.
- **Energy Conservation**: Ensure that the total energy (flux) of a star remains constant regardless of the output resolution or pixel density.

### 2.2 Spectral PSF
- **Wavelength Dependency**: Calculate the Gaussian width ($\sigma$) dynamically for each of the 40 spectral bands.
- **Formula**: Use the Airy disk approximation $	heta \approx 1.22 \frac{\lambda}{D}$ to determine the angular spread, where $\lambda$ is the wavelength and $D$ is the aperture diameter.
- **Visual Outcome**: Stars may exhibit subtle chromatic effects where longer wavelengths (red) have slightly larger spreads than shorter wavelengths (blue).

### 2.3 User Configuration
- **New CLI Argument**: Add `--aperture` (or `-a`) to specify the observer's aperture diameter in millimeters.
- **Default Value**: 6.0 mm (approximating a human eye in low light).
- **Validation**: Ensure non-negative values are handled gracefully.

## 3. Non-Functional Requirements
- **Performance**: The analytic integration (likely involving error functions `erf`) must be optimized to avoid significant slowdowns in the rendering loop, especially given the large number of stars in the catalog.
- **Accuracy**: The solution must maintain the high-dynamic-range (HDR) nature of the pipeline before tone mapping.

## 4. Acceptance Criteria
1.  **Resolution Independence**: Rendering the same scene at 1920x1080 and 960x540 results in stars that appear visually consistent in terms of total brightness and relative size (bloom).
2.  **Aperture Control**: Running with a larger aperture (e.g., `--aperture 50` for a binocular view) results in smaller, sharper stars (smaller Airy disk), while a smaller aperture (e.g., `--aperture 2`) results in larger, fuzzier stars.
3.  **Spectral Correctness**: The PSF calculation correctly accounts for wavelength, ensuring physical validity across the spectrum.
