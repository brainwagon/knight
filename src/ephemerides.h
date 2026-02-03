#ifndef EPHEMERIDES_H
#define EPHEMERIDES_H

#include "core.h"
#include "stars.h"

// Computes Julian Day from date
double get_julian_day(int year, int month, int day, double hour);

// Computes Sun and Moon direction (normalized) in local horizon coordinates (North=Z?, usually Y=Up, Z=North, X=East in standard LH, or similar)
// We will assume a coordinate system: Y is Up, Z is North, X is East (Right Handed).
// lat, lon in degrees.
void sun_moon_position(double jd, double lat, double lon, Vec3* sun_dir, Vec3* moon_dir);

// Transforms star coordinates from Equatorial (RA/Dec) to Horizon (Alt/Az) and Cartesian direction.
// Updates the az, alt, and direction fields of the stars.
void star_equ_to_horizon(double jd, double lat, double lon, Star* catalog, int n);

typedef struct {
    const char* name;
    float ra, dec;
    float alt, az;
    float vmag;
    Vec3 direction;
} Planet;

// Computes positions and magnitudes for Mercury, Venus, Mars, Jupiter, Saturn
void planets_position(double jd, double lat, double lon, Planet* planets);

// Returns the Sun's ecliptic longitude in degrees for a given JD
double get_sun_ecliptic_longitude(double jd);

// Returns an English description of the moon phase
const char* get_moon_phase_name(double jd);

// Computes sunrise, sunset, and astronomical dawn/dusk times for a given JD and location.
// Times are in UTC hours [0, 24].
void sun_rise_set(double jd, double lat, double lon, 
                  double* sunrise, double* sunset, 
                  double* astro_dawn, double* astro_dusk);

// Greenwich Mean Sidereal Time in radians
double greenwich_mean_sidereal_time(double jd);

// Local Mean Sidereal Time in radians
double local_mean_sidereal_time(double gmst, double lon_deg);

#endif
