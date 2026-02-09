#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "constellation.h"

void test_vertex_structure() {
    ConstellationVertex v;
    v.ra = 1.0f;
    v.dec = 0.5f;
    strcpy(v.abbr, "Ori");
    
    assert(v.ra == 1.0f);
    assert(v.dec == 0.5f);
    assert(strcmp(v.abbr, "Ori") == 0);
    printf("test_vertex_structure passed\n");
}

void test_load_boundaries() {

    ConstellationBoundary boundary;

    int result = load_constellation_boundaries("../data/bound_in_20.txt", &boundary);

    assert(result == 0);

    assert(boundary.count > 0);

    

    // Count unique constellations

    char list[200][4];

    int unique_count = 0;

    for (int i = 0; i < boundary.count; i++) {

        int found = 0;

        for (int j = 0; j < unique_count; j++) {

            if (strcmp(boundary.vertices[i].abbr, list[j]) == 0) {

                found = 1;

                break;

            }

        }

        if (!found && unique_count < 200) {

            strcpy(list[unique_count], boundary.vertices[i].abbr);

            unique_count++;

        }

    }

    

    // There should be 88 constellations

    printf("test_load_boundaries passed: loaded %d vertices, %d constellations\n", boundary.count, unique_count);

    assert(unique_count == 88);

    

    free_constellation_boundaries(&boundary);

}





void test_equ_to_horizon() {





    ConstellationBoundary boundary;





    load_constellation_boundaries("../data/bound_in_20.txt", &boundary);





    





    double jd = get_julian_day(2026, 2, 8, 10.0);





    double lat = 45.0;





    double lon = 0.0;





    





    constellation_equ_to_horizon(jd, lat, lon, &boundary);





    





    // Check if altitude/azimuth were computed





    for (int i = 0; i < 10; i++) {





        assert(!isnan(boundary.vertices[i].alt));





        assert(!isnan(boundary.vertices[i].az));





    }





    





    printf("test_equ_to_horizon passed\n");





    free_constellation_boundaries(&boundary);





}











void test_draw_char() {











    Image img;











    img.width = 16;











    img.height = 16;











    img.channels = 3;











    img.data = (unsigned char*)calloc(img.width * img.height * img.channels, 1);











    











    // Draw 'A' at 0,0











    draw_char(&img, 0, 0, 'A', 255, 255, 255);











    











    // Check top of 'A' (0x18 = 00011000)











    // Row 0, Col 3 and 4 should be white











    int idx3 = (0 * 16 + 3) * 3;











    int idx4 = (0 * 16 + 4) * 3;











    assert(img.data[idx3] == 255);











    assert(img.data[idx4] == 255);











    











    free(img.data);











    printf("test_draw_char passed\n");











}























int main() {











    test_vertex_structure();











    test_load_boundaries();











    test_equ_to_horizon();











    test_draw_char();











    printf("All constellation tests passed!\n");











    return 0;











}


















