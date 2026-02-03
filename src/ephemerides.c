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

double get_sun_ecliptic_longitude(double jd) {
    double n = jd - 2451545.0;
    double L = 280.460 + 0.9856474 * n; // Mean longitude
    double g = 357.528 + 0.9856003 * n; // Mean anomaly
    
    L = fmod(L, 360.0);
    g = fmod(g, 360.0);
    if (L < 0) L += 360.0;
    if (g < 0) g += 360.0;
    
    double lambda = L + 1.915 * sin(g * DEG2RAD) + 0.020 * sin(2 * g * DEG2RAD);
    return fmod(lambda, 360.0);
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

// Simplified Orbital Elements (J2000)
// a: semi-major axis (AU), e: eccentricity, i: inclination (deg), 
// L: mean longitude (deg), w: longitude of perihelion (deg), N: longitude of ascending node (deg)
typedef struct {
    double a, e, i, L, w, N;
    double da, de, di, dL, dw, dN; // rates per century
} OrbitalElements;

static const OrbitalElements PLANET_ELEMENTS[] = {
    {0.387098, 0.205630, 7.0049, 252.2509, 77.4577, 48.3308, 0, 0, -0.0059, 149472.6746, 0.1591, -0.1254}, // Mercury
    {0.723329, 0.006772, 3.3946, 181.9798, 131.5637, 76.6799, 0, -0.000047, -0.0008, 58517.8156, 0.0050, -0.2781}, // Venus
    {1.000002, 0.016708, 0.0000, 100.4664, 102.9373, 0.0, 0, -0.000042, -0.0130, 35999.3730, 0.3225, 0.0}, // Earth (for heliocentric)
    {1.523679, 0.093400, 1.8497, 355.4465, 336.0602, 49.5581, 0, 0.000090, -0.0081, 19140.2993, 0.4439, -0.2925}, // Mars
    {5.202603, 0.048497, 1.3032, 34.3515, 14.3312, 100.4644, 0.000002, 0.000163, -0.0054, 3034.9056, 0.2155, -0.1989}, // Jupiter
    {9.554909, 0.055508, 2.4888, 50.0774, 93.0567, 113.6655, -0.000002, -0.000346, -0.0037, 1222.1137, 0.5517, -0.2886} // Saturn
};

void planets_position(double jd, double lat, double lon, Planet* planets) {
    double T = (jd - 2451545.0) / 36525.0;
    double gmst = greenwich_mean_sidereal_time(jd);
    double lmst = local_mean_sidereal_time(gmst, lon);
    double eps = 23.439 * DEG2RAD;

    // Earth Position
    const OrbitalElements* ee = &PLANET_ELEMENTS[2];
    double e_L = fmod(ee->L + ee->dL * T, 360.0) * DEG2RAD;
    double e_e = ee->e + ee->de * T;
    double e_a = ee->a + ee->da * T;
    double e_w = ee->w + ee->dw * T * DEG2RAD;
    double e_M = e_L - e_w;
    double e_E = e_M + e_e * sin(e_M) * (1.0 + e_e * cos(e_M));
    double e_xv = e_a * (cos(e_E) - e_e);
    double e_yv = e_a * (sqrt(1.0 - e_e * e_e) * sin(e_E));
    double xe = e_xv * cos(e_w) - e_yv * sin(e_w);
    double ye = e_xv * sin(e_w) + e_yv * cos(e_w);
    double ze = 0;

    const char* names[] = {"Mercury", "Venus", "Mars", "Jupiter", "Saturn"};
    int idx_map[] = {0, 1, 3, 4, 5};

    for (int i = 0; i < 5; i++) {
        const OrbitalElements* oe = &PLANET_ELEMENTS[idx_map[i]];
        double a = oe->a + oe->da * T;
        double e = oe->e + oe->de * T;
        double incl = (oe->i + oe->di * T) * DEG2RAD;
        double L = fmod(oe->L + oe->dL * T, 360.0) * DEG2RAD;
        double w = (oe->w + oe->dw * T) * DEG2RAD;
        double N = (oe->N + oe->dN * T) * DEG2RAD;

        double M = L - w;
        double E = M + e * sin(M) * (1.0 + e * cos(M));
        double xv = a * (cos(E) - e);
        double yv = a * (sqrt(1.0 - e * e) * sin(E));

        double xh = xv * (cos(N) * cos(w - N) - sin(N) * sin(w - N) * cos(incl)) - yv * (cos(N) * sin(w - N) + sin(N) * cos(w - N) * cos(incl));
        double yh = xv * (sin(N) * cos(w - N) + cos(N) * sin(w - N) * cos(incl)) - yv * (sin(N) * sin(w - N) - cos(N) * cos(w - N) * cos(incl));
        double zh = xv * (sin(w - N) * sin(incl)) + yv * (cos(w - N) * sin(incl));

        // Geocentric
        double x = xh - xe;
        double y = yh - ye;
        double z = zh - ze;

        double RA = atan2(y * cos(eps) - z * sin(eps), x);
        double Dec = atan2(z * cos(eps) + y * sin(eps), sqrt(x * x + (y * cos(eps) - z * sin(eps)) * (y * cos(eps) - z * sin(eps))));

        planets[i].name = names[i];
        planets[i].ra = (float)RA;
        planets[i].dec = (float)Dec;

        equatorial_to_horizon(planets[i].ra, planets[i].dec, lmst, lat, &planets[i].alt, &planets[i].az);

        planets[i].direction.x = cosf(planets[i].alt) * sinf(planets[i].az);
        planets[i].direction.y = sinf(planets[i].alt);
        planets[i].direction.z = cosf(planets[i].alt) * cosf(planets[i].az);

        // Simple Magnitudes (approximate)
        float base_mag[] = {-0.42f, -4.40f, -1.52f, -2.59f, 0.67f};
        planets[i].vmag = base_mag[i];
    }
}

const char* get_moon_phase_name(double jd) {
    double T = (jd - 2451545.0) / 36525.0;
    // Mean elongation of moon (D) in degrees
    double D = 297.8501921 + 445267.1114034 * T;
    D = fmod(D, 360.0);
    if (D < 0) D += 360.0;

    // D = 0 is New Moon
    // D = 90 is First Quarter
    // D = 180 is Full Moon
    // D = 270 is Last Quarter

    if (D < 6.0 || D > 354.0) return "New Moon";
    if (D < 84.0) return "Waxing Crescent";
    if (D < 96.0) return "First Quarter";
    if (D < 174.0) return "Waxing Gibbous";
    if (D < 186.0) return "Full Moon";
    if (D < 264.0) return "Waning Gibbous";
    if (D < 276.0) return "Last Quarter";
    return "Waning Crescent";
}

void sun_rise_set(double jd, double lat, double lon, 
                  double* sunrise, double* sunset, 
                  double* astro_dawn, double* astro_dusk) {
    // Start at midnight of the current day
    double jd_start = floor(jd - 0.5) + 0.5;
    
    *sunrise = -1.0; *sunset = -1.0;
    *astro_dawn = -1.0; *astro_dusk = -1.0;

    float prev_alt = -100.0f;
    
    // Scan the day in 10-minute steps to find crossings
    for (int i = 0; i <= 144; i++) {
        double current_jd = jd_start + (i / 144.0);
        double gmst = greenwich_mean_sidereal_time(current_jd);
        double lmst = local_mean_sidereal_time(gmst, lon);
        
        float s_ra, s_dec;
        get_sun_equatorial(current_jd, &s_ra, &s_dec);
        
        float alt, az;
        equatorial_to_horizon(s_ra, s_dec, lmst, lat, &alt, &az);
        float alt_deg = alt * RAD2DEG;

        if (i > 0) {
            // Sunrise/Sunset (~ -0.833 deg for refraction/disk, but -0.5 is close enough for this model)
            float horizon = -0.833f;
            if (prev_alt < horizon && alt_deg >= horizon) *sunrise = (i / 144.0) * 24.0;
            if (prev_alt > horizon && alt_deg <= horizon) *sunset = (i / 144.0) * 24.0;

            // Astronomical Twilight (-18 deg)
            float astro_limit = -18.0f;
            if (prev_alt < astro_limit && alt_deg >= astro_limit) *astro_dawn = (i / 144.0) * 24.0;
            if (prev_alt > astro_limit && alt_deg <= astro_limit) *astro_dusk = (i / 144.0) * 24.0;
        }
        prev_alt = alt_deg;
    }
}
