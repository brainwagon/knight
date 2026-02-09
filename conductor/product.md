# Initial Concept
Knight is a physically-based night sky rendering engine written in vanilla C. It implements spectral rendering, atmospheric scattering, accurate ephemerides for celestial bodies, and simulates human night-time perception traits.

# Product Definition

## Target Audience
- **Graphics Researchers and Enthusiasts**: Individuals interested in the physics of atmospheric scattering, spectral light transport, and night-sky modeling.
- **Game Developers**: Developers seeking a physically-accurate skybox generator or a reference for integrating realistic night skies into their engines.
- **Amateur Astronomers**: Users who want to visualize celestial events and the appearance of the sky from specific locations and times on Earth.

## Core Goals
- **Physical Accuracy**: Prioritize the scientific accuracy of spectral rendering and atmospheric models to reflect realistic photometric behavior.
- **Performance and Efficiency**: Provide a high-performance renderer, utilizing CUDA where available, capable of generating high-resolution HDR environment maps quickly.
- **User-Friendly Interface**: Maintain and improve a robust CLI that allows for easy configuration of simulation parameters (time, location, field of view).

## Key Features
- **Advanced Atmospheric Simulation**: Realistic Rayleigh and Mie scattering models with spectral ray marching through a spherical atmosphere.
- **Accurate Celestial Positioning**: High-precision ephemerides for the Sun, Moon, and major planets, integrated with the Yale Bright Star Catalog.
- **Celestial Body Labeling**: Option to label the Sun, Moon, and major planets with their names in configurable colors for easy identification.
- **Night-Time Perception**: Simulation of scotopic vision effects, including the Purkinje shift, rod saturation, and stellar bloom.
- **Physically-Based Star Rendering**: Analytic Point Spread Function (PSF) based on the diffraction limit of the observer's aperture, ensuring resolution-independent brightness and realistic spectral spread.
- **Constellation Outlines & Labels**: Ability to render accurate IAU constellation boundaries and labels (3-letter abbreviations) based on J2000 coordinate data, with full support for both standard and environment map projections including proper meridian wrap-around.
- **Flexible Output**: Support for HDR output (PFM) and conversion to standard formats (PNG) for various use cases, including environment mapping.
