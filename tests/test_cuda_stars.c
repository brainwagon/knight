#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cuda_host.h"
#include "stars.h"

int main() {
#ifndef CUDA_ENABLED
    printf("CUDA not enabled, skipping test.\n");
    return 0;
#endif

    cuda_init();

    int num_stars = 100;
    Star* stars = (Star*)malloc(sizeof(Star) * num_stars);
    for (int i = 0; i < num_stars; i++) {
        stars[i].id = i;
        stars[i].vmag = (float)i / 10.0f;
    }

    bool ok = cuda_upload_stars(stars, num_stars);
    printf("cuda_upload_stars status: %d\n", ok);
    assert(ok);

    // Upload again with more stars to test reallocation
    num_stars = 200;
    stars = (Star*)realloc(stars, sizeof(Star) * num_stars);
    for (int i = 100; i < num_stars; i++) {
        stars[i].id = i;
    }
    ok = cuda_upload_stars(stars, num_stars);
    printf("cuda_upload_stars (realloc) status: %d\n", ok);
    assert(ok);

    cuda_cleanup();
    free(stars);

    printf("test_cuda_stars passed\n");
    return 0;
}