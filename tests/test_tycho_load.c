#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include "stars.h"

const char* mock_dir = "mock_tycho";
const char* mock_file = "mock_tycho/tyc2.dat.00";

void create_mock_tycho() {
    mkdir(mock_dir, 0777);
    FILE* f = fopen(mock_file, "w");
    if (!f) return;
    // 1-4 TYC1, 6-10 TYC2, 12 TYC3
    // RAmdeg: 16-27 (F12.8)
    // DEmdeg: 29-40 (F12.8)
    // BTmag: 111-116 (F6.3)
    // VTmag: 124-129 (F6.3)
    // Star 1: RA=10.0, Dec=20.0, BT=5.0, VT=4.5
    // 0001 00001 1| | 10.00000000| 20.00000000| ... (many pipes) ... |  5.000| |  4.500|
    
    char line[208];
    memset(line, ' ', 206);
    line[206] = '\n';
    line[207] = '\0';
    
    // Fill in values for Star 1
    memcpy(line + 0, "0001 00001 1", 12);
    memcpy(line + 15, " 10.00000000", 12);
    memcpy(line + 28, " 20.00000000", 12);
    memcpy(line + 110, "  5.000", 7);
    memcpy(line + 123, "  4.500", 7);
    line[206] = '\n'; // Ensure newline is still there
    fprintf(f, "%s", line);

    // Star 2: RA=15.0, Dec=-10.0, BT=12.0, VT=11.5 (Should be filtered if limit=10)
    memset(line, ' ', 206);
    line[206] = '\n';
    line[207] = '\0';
    memcpy(line + 0, "0001 00002 1", 12);
    memcpy(line + 15, " 15.00000000", 12);
    memcpy(line + 28, "-10.00000000", 12);
    memcpy(line + 110, " 12.000", 7);
    memcpy(line + 123, " 11.500", 7);
    line[206] = '\n';
    fprintf(f, "%s", line);

    fclose(f);
}

void test_tycho_parsing() {
    create_mock_tycho();
    Star* stars = NULL;
    
    // Limit 10.0: Should load 1 star
    int num = load_stars_tycho(mock_dir, 10.0f, &stars);
    printf("Loaded %d Tycho stars (expected 1)\n", num);
    assert(num == 1);
    
    if (num > 0) {
        // V = VT - 0.090*(BT-VT) = 4.5 - 0.090*(0.5) = 4.5 - 0.045 = 4.455
        // B-V = 0.850*(BT-VT) = 0.850*(0.5) = 0.425
        printf("Star 0: RA=%.2f, Dec=%.2f, Mag=%.3f, BV=%.3f\n", 
               stars[0].ra * RAD2DEG, stars[0].dec * RAD2DEG, stars[0].vmag, stars[0].bv);
        assert(fabs(stars[0].ra * RAD2DEG - 10.0) < 0.0001);
        assert(fabs(stars[0].dec * RAD2DEG - 20.0) < 0.0001);
        assert(fabs(stars[0].vmag - 4.455) < 0.001);
        assert(fabs(stars[0].bv - 0.425) < 0.001);
    }
    
    if (stars) free(stars);
    remove(mock_file);
    rmdir(mock_dir);
    printf("test_tycho_parsing passed\n");
}

int main() {
    test_tycho_parsing();
    return 0;
}