#include "zodiacal.h"
#include "zodiacal_math.h"
#include <math.h>

void horizon_to_ecliptic(float alt, float az, float lat_deg, float lmst, float* ecl_lon, float* ecl_lat) {
    horizon_to_ecliptic_math(alt, az, lat_deg, lmst, ecl_lon, ecl_lat);
}

Spectrum compute_zodiacal_light(Vec3 view_dir, Vec3 sun_dir, float sun_ecliptic_lon_deg, float cam_lat, float lmst) {
    return compute_zodiacal_light_math(view_dir, sun_dir, sun_ecliptic_lon_deg, cam_lat, lmst);
}
