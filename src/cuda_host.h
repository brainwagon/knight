#ifndef CUDA_HOST_H
#define CUDA_HOST_H

#include <stdbool.h>
#include "core.h"
#include "atmosphere.h"

#ifdef CUDA_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

void cuda_init();
void cuda_cleanup();

// Returns true if success
bool cuda_render_frame(
    int width, int height,
    const Atmosphere* atm,
    Vec3 cam_pos, Vec3 cam_forward, Vec3 cam_right, Vec3 cam_up,
    float fov, float aspect,
    Vec3 sun_dir, const Spectrum* sun_intensity,
    Vec3 moon_dir, const Spectrum* moon_intensity,
    float sun_ecl_lon, float cam_lat, float lmst,
    bool env_map,
    XYZV* out_pixels // buffer on host to copy results to
);

#ifdef __cplusplus
}
#endif

#endif
#endif
