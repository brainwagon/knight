#include "ephemerides.h"

// Julian Day calculation
double get_julian_day(int year, int month, int day, double hour) {
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    int A = year / 100;
    int B = 2 - A + A / 4;
    return (int)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) + day + B - 1524.5 + hour / 24.0;
}

double greenwich_mean_sidereal_time(double jd) {
    double T = (jd - 2451545.0) / 36525.0;
    double gmst = 280.46061837 + 360.98564736629 * (jd - 2451545.0) + T*T * (0.000387933 - T / 38710000.0);
    // Normalize to 0-360
    gmst = fmod(gmst, 360.0);
    if (gmst < 0) gmst += 360.0;
    return gmst * DEG2RAD;
}

double local_mean_sidereal_time(double gmst, double lon_deg) {
    double lmst = gmst + lon_deg * DEG2RAD;
    return fmod(lmst, TWO_PI);
}

// Low precision sun position
static void get_sun_equatorial(double jd, float* ra, float* dec) {
    double n = jd - 2451545.0;
    double L = 280.460 + 0.9856474 * n; // Mean longitude
    double g = 357.528 + 0.9856003 * n; // Mean anomaly
    
    L = fmod(L, 360.0);
    g = fmod(g, 360.0);
    if (L < 0) L += 360.0;
    if (g < 0) g += 360.0;
    
    double lambda = L + 1.915 * sin(g * DEG2RAD) + 0.020 * sin(2 * g * DEG2RAD); // Ecliptic longitude
    double epsilon = 23.439 - 0.0000004 * n; // Obliquity of ecliptic
    
    double l_rad = lambda * DEG2RAD;
    double e_rad = epsilon * DEG2RAD;
    
    // Convert to RA/Dec
    // tan(RA) = cos(epsilon) * tan(lambda)
    // sin(Dec) = sin(epsilon) * sin(lambda)
    
    double alpha = atan2(cos(e_rad) * sin(l_rad), cos(l_rad));
    double delta = asin(sin(e_rad) * sin(l_rad));
    
    *ra = (float)alpha;
    *dec = (float)delta;
}

// Low precision moon position
static void get_moon_equatorial(double jd, float* ra, float* dec) {
    // Very simplified, just to get it in the sky roughly
    // Use full Meeus if high accuracy needed, but this is a stub
    // Approximating from fundamental arguments
    
    double T = (jd - 2451545.0) / 36525.0;
    
    // Moon mean longitude
    double Lp = 218.3164477 + 481267.88123421 * T;
    // Mean elongation of moon
    double D = 297.8501921 + 445267.1114034 * T;
    // Sun mean anomaly
    double M = 357.5291092 + 35999.0502909 * T;
    // Moon mean anomaly
    double Mp = 134.9633964 + 477198.8675055 * T;
    // Moon argument of latitude
    double F = 93.2720950 + 483202.0175233 * T;
    
    double l_rad = Lp * DEG2RAD; // Very rough ecliptic longitude
    // Add some major perturbations (Eviction, Variation, Annual Eq)
    // Ignore for this "night rendering" prototype to keep it simple and vanilla C
    // Assume Moon is on ecliptic roughly for now? No, assume inclination 5 deg.
    
    // Let's use a slightly better approx:
    double lambda = Lp + 6.289 * sin(Mp * DEG2RAD);
    double beta = 5.128 * sin(F * DEG2RAD);
    
    double epsilon = 23.439 * DEG2RAD;
    double lam_rad = lambda * DEG2RAD;
    double bet_rad = beta * DEG2RAD;
    
    // Ecliptic to Equatorial
    // sin(dec) = sin(beta)cos(eps) + cos(beta)sin(eps)sin(lambda)
    // cos(dec)cos(ra) = cos(beta)cos(lambda)
    // cos(dec)sin(ra) = -sin(beta)sin(eps) + cos(beta)cos(eps)sin(lambda)
    
    double s_dec = sin(bet_rad)*cos(epsilon) + cos(bet_rad)*sin(epsilon)*sin(lam_rad);
    double c_dec_c_ra = cos(bet_rad)*cos(lam_rad);
    double c_dec_s_ra = -sin(bet_rad)*sin(epsilon) + cos(bet_rad)*cos(epsilon)*sin(lam_rad);
    
    *dec = asinf(s_dec);
    *ra = atan2f(c_dec_s_ra, c_dec_c_ra);
}

static void equatorial_to_horizon(float ra, float dec, double lmst, double lat, float* alt, float* az) {
    double ha = lmst - ra; // Hour Angle
    double lat_rad = lat * DEG2RAD;
    
    double sin_alt = sin(dec) * sin(lat_rad) + cos(dec) * cos(lat_rad) * cos(ha);
    *alt = asinf(sin_alt);
    
    double cos_alt = cos(*alt);
    
    // cos(Az)cos(Alt) = sin(Dec)cos(Lat) - cos(Dec)sin(Lat)cos(HA) -> From North?
    // Let's use standard formula:
    // tan(Az) = sin(HA) / (cos(HA)sin(lat) - tan(dec)cos(lat)) 
    // Wait, let's use:
    // sin(Az)cos(Alt) = -cos(Dec)sin(HA)
    // cos(Az)cos(Alt) = sin(Dec)cos(Lat) - cos(Dec)sin(Lat)cos(HA)
    
    double sin_az_c_alt = -cos(dec) * sin(ha);
    double cos_az_c_alt = sin(dec) * cos(lat_rad) - cos(dec) * sin(lat_rad) * cos(ha);
    
    *az = atan2f(sin_az_c_alt, cos_az_c_alt);
}

void sun_moon_position(double jd, double lat, double lon, Vec3* sun_dir, Vec3* moon_dir) {
    double gmst = greenwich_mean_sidereal_time(jd);
    double lmst = local_mean_sidereal_time(gmst, lon);
    
    float s_ra, s_dec;
    get_sun_equatorial(jd, &s_ra, &s_dec);
    
    float s_alt, s_az;
    equatorial_to_horizon(s_ra, s_dec, lmst, lat, &s_alt, &s_az);
    
    sun_dir->x = cosf(s_alt) * sinf(s_az);
    sun_dir->y = sinf(s_alt);
    sun_dir->z = cosf(s_alt) * cosf(s_az);
    *sun_dir = vec3_normalize(*sun_dir);
    
    float m_ra, m_dec;
    get_moon_equatorial(jd, &m_ra, &m_dec);
    
    float m_alt, m_az;
    equatorial_to_horizon(m_ra, m_dec, lmst, lat, &m_alt, &m_az);
    
    moon_dir->x = cosf(m_alt) * sinf(m_az);
    moon_dir->y = sinf(m_alt);
    moon_dir->z = cosf(m_alt) * cosf(m_az);
    *moon_dir = vec3_normalize(*moon_dir);
}

void star_equ_to_horizon(double jd, double lat, double lon, Star* catalog, int n) {
    double gmst = greenwich_mean_sidereal_time(jd);
    double lmst = local_mean_sidereal_time(gmst, lon);
    
    for (int i = 0; i < n; i++) {
        equatorial_to_horizon(catalog[i].ra, catalog[i].dec, lmst, lat, &catalog[i].alt, &catalog[i].az);
        
        catalog[i].direction.x = cosf(catalog[i].alt) * sinf(catalog[i].az);
        catalog[i].direction.y = sinf(catalog[i].alt);
        catalog[i].direction.z = cosf(catalog[i].alt) * cosf(catalog[i].az);
    }
}
