#include "stars.h"

static float parse_float(const char* line, int start, int len) {
    char buf[32];
    if (len >= 31) len = 31;
    memcpy(buf, line + start, len);
    buf[len] = '\0';
    // Check if empty
    int empty = 1;
    for (int i = 0; i < len; i++) {
        if (buf[i] != ' ') { empty = 0; break; }
    }
    if (empty) return NAN;
    return strtof(buf, NULL);
}

static int parse_int(const char* line, int start, int len) {
    char buf[32];
    if (len >= 31) len = 31;
    memcpy(buf, line + start, len);
    buf[len] = '\0';
    int empty = 1;
    for (int i = 0; i < len; i++) {
        if (buf[i] != ' ') { empty = 0; break; }
    }
    if (empty) return 0; // Or -1? Usually 0 for RA/Dec parts if missing is risky.
    return atoi(buf);
}

int load_stars(const char* filepath, Star** stars) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;
    
    // Allocate max stars
    int max_stars = 10000;
    *stars = (Star*)malloc(sizeof(Star) * max_stars);
    int count = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) < 114) continue; // Too short
        
        // Indices 0-based
        // RAh: 75 (2)
        // RAm: 77 (2)
        // RAs: 79 (4)
        // DE-: 83 (1)
        // DEd: 84 (2)
        // DEm: 86 (2)
        // DEs: 88 (2)
        
        // Vmag: 102 (5)
        // B-V: 109 (5)
        
        float vmag = parse_float(line, 102, 5);
        if (isnan(vmag)) continue; // Skip if no mag
        
        float bv = parse_float(line, 109, 5);
        if (isnan(bv)) bv = 0.0f; // Default if missing?
        
        float rah = (float)parse_int(line, 75, 2);
        float ram = (float)parse_int(line, 77, 2);
        float ras = parse_float(line, 79, 4);
        if (isnan(ras)) ras = 0.0f;
        
        char de_sign = line[83];
        float ded = (float)parse_int(line, 84, 2);
        float dem = (float)parse_int(line, 86, 2);
        float des = (float)parse_int(line, 88, 2);
        
        // Check validity of position. If RAh is missing (blank), it's likely invalid.
        // We can check if RAh was actually parsed.
        // But parse_float/int returns 0/NAN.
        // Let's assume valid if Vmag exists, usually. 
        // But some stars might be removed.
        
        // Compute RA/Dec in radians
        float ra_deg = (rah + ram / 60.0f + ras / 3600.0f) * 15.0f;
        float dec_deg = ded + dem / 60.0f + des / 3600.0f;
        if (de_sign == '-') dec_deg = -dec_deg;
        
        Star* s = &(*stars)[count];
        s->id = count; // or HR number from bytes 0-4
        s->vmag = vmag;
        s->bv = bv;
        s->ra = ra_deg * DEG2RAD;
        s->dec = dec_deg * DEG2RAD;
        
        count++;
        if (count >= max_stars) break;
    }
    
    fclose(f);
    return count;
}
