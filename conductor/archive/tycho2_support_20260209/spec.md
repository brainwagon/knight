# Specification: Tycho2 Star Catalog Integration

## Overview
This track implements support for the Tycho-2 star catalog in the Knight rendering engine. It introduces a configurable star catalog selection, a universal magnitude threshold for filtering stars, and a specific parser for the Tycho-2 data format.

## Functional Requirements

### 1. Tycho-2 Catalog Support
- Implement a parser for the Tycho-2 `tyc2.dat.XX` file format (pipe-delimited, fixed-byte positions).
- Support loading stars from the `tycho/` directory (or a user-specified path).
- The parser must combine data from all 20 segments (`tyc2.dat.00` through `tyc2.dat.19`).
- Perform coordinate conversion from J2000 (ICRS) to the internal `Star` representation.
- Calculate B-V color index from Tycho-2 `BTmag` and `VTmag` using the standard transformation: `B-V = 0.850 * (BT - VT)`.
- Derive visual magnitude `V` from `VT` and `BT`: `V = VT - 0.090 * (BT - VT)`.

### 2. Universal Magnitude Filtering
- Implement a new configuration parameter `star_mag_limit` (default: 6.0).
- Apply this filter during the loading phase for both Yale Bright Star Catalog (YBSC5) and Tycho-2 to optimize memory usage.

### 3. CLI Configuration
- Add `--tycho` flag to switch the active catalog to Tycho-2.
- Add `--tycho-dir <path>` to specify the location of Tycho-2 data files (defaults to `./tycho`).
- Add `--mag-limit <float>` to set the visual magnitude threshold for stars.

## Non-Functional Requirements
- **Memory Efficiency**: Stars should be filtered during file I/O to avoid allocating memory for stars below the visibility threshold.
- **Performance**: The Tycho-2 parser should efficiently handle up to 2.5 million records.
- **Robustness**: Gracefully handle missing Tycho-2 segments or malformed lines.

## Acceptance Criteria
- [ ] Users can toggle between YBSC5 (default) and Tycho-2 using CLI flags.
- [ ] The `--mag-limit` flag correctly filters stars for both catalogs.
- [ ] Stars loaded from Tycho-2 appear correctly in the rendered output (properly positioned and colored).
- [ ] The engine does not crash when loading the full Tycho-2 dataset within the 110MB memory expectation.

## Out of Scope
- Automatic decompression of `.gz` files (user is responsible for decompression).
- Support for Tycho-2 supplement files (`suppl_1.dat`, `suppl_2.dat`).
- Dynamic catalog switching during runtime (handled at initialization).
