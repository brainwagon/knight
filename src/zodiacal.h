#ifndef ZODIACAL_H
#define ZODIACAL_H

#include "core.h"

/**
 * Computes the zodiacal light contribution for a given view direction.
 * 
 * @param view_dir Normalized view direction in world coordinates (Y=up).
 * @param sun_dir Normalized sun direction in world coordinates.
 * @param sun_ecliptic_lon Sun's ecliptic longitude in degrees.
 * @param cam_lat Observer's latitude in degrees.
 * @param lmst Local Mean Sidereal Time in radians.
 * @return Spectrum containing the zodiacal light radiance.
 */
Spectrum compute_zodiacal_light(Vec3 view_dir, Vec3 sun_dir, float sun_ecliptic_lon, float cam_lat, float lmst);

/**
 * Helper to convert Horizon coordinates to Ecliptic coordinates.
 * 
 * @param alt Altitude in radians.
 * @param az Azimuth in radians.
 * @param lat Observer latitude in radians.
 * @param lmst Local Mean Sidereal Time in radians.
 * @param ecl_lon [out] Ecliptic longitude in radians.
 * @param ecl_lat [out] Ecliptic latitude in radians.
 */
void horizon_to_ecliptic(float alt, float az, float lat, float lmst, float* ecl_lon, float* ecl_lat);

#endif
