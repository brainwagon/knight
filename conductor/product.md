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
- **Night-Time Perception**: Simulation of scotopic vision effects, including the Purkinje shift, rod saturation, and stellar bloom.
- **Constellation Outlines**: Ability to render accurate IAU constellation boundaries based on J2000 coordinate data.
- **Flexible Output**: Support for HDR output (PFM) and conversion to standard formats (PNG) for various use cases, including environment mapping.
