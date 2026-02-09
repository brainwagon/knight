#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "cuda_host.h"
#include "core.h"
#include "atmosphere.h"
#include "zodiacal.h"

// Math implementations
#include "atmosphere_math.h"
#include "zodiacal_math.h"

// Constant memory for CIE tables
__constant__ float c_CIE_X[40];
__constant__ float c_CIE_Y[40];
__constant__ float c_CIE_Z[40];
__constant__ float c_CIE_V[40];

// Host copies of CIE tables for initialization
static const float h_CIE_X[40] = {
    0.0014, 0.0042, 0.0143, 0.0435, 0.1344, 0.2839, 0.3483, 0.3362, 0.2908, 0.1954,
    0.0956, 0.0320, 0.0049, 0.0093, 0.0633, 0.1655, 0.2904, 0.4334, 0.5945, 0.7621,
    0.9163, 1.0263, 1.0622, 1.0026, 0.8544, 0.6424, 0.4479, 0.2835, 0.1649, 0.0874,
    0.0468, 0.0227, 0.0114, 0.0058, 0.0029, 0.0014, 0.0007, 0.0003, 0.0002, 0.0001
};
static const float h_CIE_Y[40] = {
    0.0000, 0.0001, 0.0004, 0.0012, 0.0040, 0.0116, 0.0230, 0.0380, 0.0600, 0.0910,
    0.1390, 0.2080, 0.3230, 0.5030, 0.7100, 0.8620, 0.9540, 0.9950, 0.9950, 0.9520,
    0.8700, 0.7570, 0.6310, 0.5030, 0.3810, 0.2650, 0.1750, 0.1070, 0.0610, 0.0320,
    0.0170, 0.0082, 0.0041, 0.0021, 0.0010, 0.0005, 0.0002, 0.0001, 0.0001, 0.0000
};
static const float h_CIE_Z[40] = {
    0.0065, 0.0201, 0.0679, 0.2074, 0.6456, 1.3856, 1.7471, 1.7721, 1.6692, 1.2876,
    0.8130, 0.4652, 0.2720, 0.1582, 0.0782, 0.0422, 0.0203, 0.0087, 0.0039, 0.0021,
    0.0017, 0.0011, 0.0008, 0.0003, 0.0002, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000,
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000
};
static const float h_CIE_V[40] = {
    0.0006, 0.0022, 0.0093, 0.0348, 0.1084, 0.2525, 0.4571, 0.6756, 0.8524, 0.9632,
    0.9939, 0.9398, 0.8110, 0.6496, 0.4812, 0.3283, 0.2076, 0.1212, 0.0665, 0.0346,
    0.0173, 0.0083, 0.0039, 0.0018, 0.0008, 0.0004, 0.0002, 0.0001, 0.0000, 0.0000,
    0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000, 0.0000
};

extern "C" void cuda_init() {
    cudaMemcpyToSymbol(c_CIE_X, h_CIE_X, 40 * sizeof(float));
    cudaMemcpyToSymbol(c_CIE_Y, h_CIE_Y, 40 * sizeof(float));
    cudaMemcpyToSymbol(c_CIE_Z, h_CIE_Z, 40 * sizeof(float));
    cudaMemcpyToSymbol(c_CIE_V, h_CIE_V, 40 * sizeof(float));
}

XYZV* d_pixels = NULL;
Star* d_stars = NULL;
int d_num_stars = 0;
cudaTextureObject_t moon_tex_obj = 0;
cudaArray* moon_tex_array = NULL;

extern "C" void cuda_cleanup() {
    if (d_pixels) {
        cudaFree(d_pixels);
        d_pixels = NULL;
    }
    if (d_stars) {
        cudaFree(d_stars);
        d_stars = NULL;
        d_num_stars = 0;
    }
    if (moon_tex_obj) {
        cudaDestroyTextureObject(moon_tex_obj);
        moon_tex_obj = 0;
    }
    if (moon_tex_array) {
        cudaFreeArray(moon_tex_array);
        moon_tex_array = NULL;
    }
}

void update_moon_texture(unsigned char* data, int w, int h) {
    if (!data) return;
    
    // Check if re-creation is needed (simplification: just destroy and recreate if exists)
    // For a static texture, checking if already created is enough.
    if (moon_tex_obj != 0) return; 

    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(8, 0, 0, 0, cudaChannelFormatKindUnsigned);
    cudaMallocArray(&moon_tex_array, &channelDesc, w, h);
    
    cudaMemcpy2DToArray(moon_tex_array, 0, 0, data, w * sizeof(unsigned char), w * sizeof(unsigned char), h, cudaMemcpyHostToDevice);
    
    struct cudaResourceDesc resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.resType = cudaResourceTypeArray;
    resDesc.res.array.array = moon_tex_array;
    
    struct cudaTextureDesc texDesc;
    memset(&texDesc, 0, sizeof(texDesc));
    texDesc.addressMode[0] = cudaAddressModeWrap;
    texDesc.addressMode[1] = cudaAddressModeClamp;
    texDesc.filterMode = cudaFilterModeLinear;
    texDesc.readMode = cudaReadModeNormalizedFloat;
    texDesc.normalizedCoords = 1;
    
    cudaCreateTextureObject(&moon_tex_obj, &resDesc, &texDesc, NULL);
}

__device__ XYZV dev_spectrum_to_xyzv(const Spectrum* s) {
    XYZV res = {0, 0, 0, 0};
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        res.X += s->s[i] * c_CIE_X[i];
        res.Y += s->s[i] * c_CIE_Y[i];
        res.Z += s->s[i] * c_CIE_Z[i];
        res.V += s->s[i] * c_CIE_V[i];
    }
    float dLambda = LAMBDA_STEP;
    res.X *= dLambda;
    res.Y *= dLambda;
    res.Z *= dLambda;
    res.V *= dLambda;
    return res;
}

__global__ void render_kernel(
    int width, int height,
    Atmosphere atm,
    Vec3 cam_pos, Vec3 cam_forward, Vec3 cam_right, Vec3 cam_up,
    float tan_half_fov, float aspect,
    Vec3 sun_dir, Spectrum sun_intensity,
    Vec3 moon_dir, Spectrum moon_intensity,
    float sun_ecl_lon, float cam_lat, float lmst,
    bool env_map,
    cudaTextureObject_t moon_tex,
    XYZV* out_pixels
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= width || y >= height) return;
    
    Vec3 dir;
    if (env_map) {
        float az_rad = (float)x / width * TWO_PI;
        float alt_rad = (0.5f - (float)y / height) * PI;
        dir.x = cosf(alt_rad) * sinf(az_rad);
        dir.y = sinf(alt_rad);
        dir.z = cosf(alt_rad) * cosf(az_rad);
    } else {
        float u = (2.0f * (x + 0.5f) / width - 1.0f) * aspect * tan_half_fov;
        float v = (1.0f - 2.0f * (y + 0.5f) / height) * tan_half_fov;
        dir = vec3_add(cam_forward, vec3_add(vec3_mul(cam_right, u), vec3_mul(cam_up, v)));
        dir = vec3_normalize(dir);
    }
    
    float alpha_atm = 1.0f;
    Spectrum L = atmosphere_render_radiance(&atm, cam_pos, dir, sun_dir, &sun_intensity, moon_dir, &moon_intensity, &alpha_atm);
    
    float t_e0, t_e1;
    if (ray_sphere_intersect_math(cam_pos, dir, EARTH_RADIUS, &t_e0, &t_e1)) {
        // Ground Intersection
        Vec3 p_hit = vec3_add(cam_pos, vec3_mul(dir, t_e0));
        Vec3 N = vec3_normalize(p_hit);
        
        Spectrum ground_irradiance;
        spectrum_zero(&ground_irradiance);
        
        // Direct Sun
        float ndotl_sun = vec3_dot(N, sun_dir);
        if (ndotl_sun > 0) {
            Spectrum t_sun = atmosphere_compute_transmittance(&atm, p_hit, sun_dir);
            Spectrum direct_sun = sun_intensity;
            spectrum_mul_spec(&direct_sun, &t_sun);
            spectrum_mul(&direct_sun, ndotl_sun);
            spectrum_add(&ground_irradiance, &direct_sun);
        }
        
        // Direct Moon
        float ndotl_moon = vec3_dot(N, moon_dir);
        if (ndotl_moon > 0) {
            Spectrum t_moon = atmosphere_compute_transmittance(&atm, p_hit, moon_dir);
            Spectrum direct_moon = moon_intensity;
            spectrum_mul_spec(&direct_moon, &t_moon);
            spectrum_mul(&direct_moon, ndotl_moon);
            spectrum_add(&ground_irradiance, &direct_moon);
        }
        
        // Ambient
        Spectrum ambient = sun_intensity; 
        spectrum_mul(&ambient, 0.0005f * (sun_dir.y > 0 ? sun_dir.y : 0));
        Spectrum moon_amb = moon_intensity;
        spectrum_mul(&moon_amb, 0.0005f * (moon_dir.y > 0 ? moon_dir.y : 0));
        spectrum_add(&ambient, &moon_amb);
        Spectrum base_amb; spectrum_set(&base_amb, 2.0e-7f);
        spectrum_add(&ambient, &base_amb);
        
        spectrum_add(&ground_irradiance, &ambient);
        
        Spectrum ground_rad = ground_irradiance;
        spectrum_mul(&ground_rad, 0.1f / PI);
        spectrum_mul(&ground_rad, alpha_atm);
        
        spectrum_add(&L, &ground_rad);
        alpha_atm = 0.0f; 
    } else {
        // Sky view: Add Zodiacal
        if (alpha_atm > 0.0f) {
            Spectrum zod = compute_zodiacal_light_math(dir, sun_dir, sun_ecl_lon, cam_lat, lmst);
            spectrum_mul(&zod, alpha_atm);
            spectrum_add(&L, &zod);
        }
    }
    
    // Moon Disk
    float cos_theta_moon = vec3_dot(dir, moon_dir);
    if (cos_theta_moon > 0.99999f && moon_dir.y > 0) {
        Vec3 m_up_vec = {0, 1, 0};
        if (fabsf(moon_dir.y) > 0.99f) m_up_vec = (Vec3){0, 0, 1};
        Vec3 m_right = vec3_normalize(vec3_cross(m_up_vec, moon_dir));
        Vec3 m_actual_up = vec3_cross(moon_dir, m_right);
        
        float dx = vec3_dot(dir, m_right);
        float dy = vec3_dot(dir, m_actual_up);
        float dist = sqrtf(dx*dx + dy*dy) / 0.0045f;
        
        if (dist <= 1.0f) {
            float dz = sqrtf(1.0f - dist*dist);
            Vec3 N = vec3_add(vec3_add(vec3_mul(m_right, dx/0.0045f), vec3_mul(m_actual_up, dy/0.0045f)), vec3_mul(moon_dir, -dz));
            N = vec3_normalize(N);
            
            float albedo = 0.12f;
            
            if (moon_tex) {
                float nx_local = dx / 0.0045f;
                float ny_local = dy / 0.0045f;
                float u = (atan2f(nx_local, dz) + PI) / TWO_PI;
                float v = acosf(ny_local) / PI;
                albedo = tex2D<float>(moon_tex, u, v) * 0.2f; // Scale to match visual brightness
            }
            
            float ndotl = vec3_dot(N, sun_dir);
            if (ndotl < 0) ndotl = 0;
            
            Spectrum moon_disk = sun_intensity;
            // Add a small amount of earthshine (0.005) to the shadow side
            spectrum_mul(&moon_disk, albedo * (ndotl + 0.005f) * alpha_atm);
            spectrum_add(&L, &moon_disk);
        }
    }
    
    // Sun Disk
    float cos_theta_sun = vec3_dot(dir, sun_dir);
    if (cos_theta_sun > 0.99999f && sun_dir.y > -0.02f) {
        Spectrum sun_disk = sun_intensity;
        spectrum_mul(&sun_disk, alpha_atm);
        spectrum_add(&L, &sun_disk);
    }
    
    out_pixels[y * width + x] = dev_spectrum_to_xyzv(&L);
}

extern "C" bool cuda_upload_stars(const Star* stars, int num_stars) {
    if (num_stars <= 0) return true;
    
    if (d_stars != NULL && d_num_stars < num_stars) {
        cudaFree(d_stars);
        d_stars = NULL;
    }
    
    if (d_stars == NULL) {
        cudaError_t err = cudaMalloc((void**)&d_stars, num_stars * sizeof(Star));
        if (err != cudaSuccess) {
            printf("CUDA Error: Failed to allocate star buffer: %s\n", cudaGetErrorString(err));
            return false;
        }
        d_num_stars = num_stars;
    }
    
    cudaError_t err = cudaMemcpy(d_stars, stars, num_stars * sizeof(Star), cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        printf("CUDA Error: Failed to copy stars to GPU: %s\n", cudaGetErrorString(err));
        return false;
    }
    
    return true;
}

extern "C" bool cuda_render_frame(
    int width, int height,
    const Atmosphere* atm, 
    Vec3 cam_pos, Vec3 cam_forward, Vec3 cam_right, Vec3 cam_up,
    float fov, float aspect,
    Vec3 sun_dir, const Spectrum* sun_intensity,
    Vec3 moon_dir, const Spectrum* moon_intensity,
    float sun_ecl_lon, float cam_lat, float lmst,
    bool env_map,
    unsigned char* moon_tex_data, int moon_tex_w, int moon_tex_h,
    XYZV* out_pixels
) {
    size_t size = width * height * sizeof(XYZV);
    if (d_pixels == NULL) {
        cudaMalloc((void**)&d_pixels, size); 
    }
    
    update_moon_texture(moon_tex_data, moon_tex_w, moon_tex_h);
    
    float tan_half_fov = tanf(fov * 0.5f * DEG2RAD);
    
    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);
    
    render_kernel<<<grid, block>>>(
        width, height,
        *atm, // Pass by value
        cam_pos, cam_forward, cam_right, cam_up,
        tan_half_fov, aspect,
        sun_dir, *sun_intensity,
        moon_dir, *moon_intensity,
        sun_ecl_lon, cam_lat, lmst,
        env_map,
        moon_tex_obj,
        d_pixels
    );
    
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Error: %s\n", cudaGetErrorString(err));
        return false;
    }
    
    cudaMemcpy(out_pixels, d_pixels, size, cudaMemcpyDeviceToHost);
    return true;
}

__global__ void render_stars_kernel(
    Star* stars, int num_stars,
    int width, int height,
    Vec3 cam_forward, Vec3 cam_right, Vec3 cam_up,
    float tan_half_fov, float aspect,
    bool env_map,
    float sigma_ang,
    float pinhole_f_px,
    XYZV* out_pixels
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_stars) return;

    Star s = stars[idx];
    if (s.direction.y <= 0) return;

    float px, py;
    if (env_map) {
        float s_az = atan2f(s.direction.x, s.direction.z) * RAD2DEG;
        if (s_az < 0) s_az += 360.0f;
        px = (s_az / 360.0f) * width;
        py = (90.0f - asinf(s.direction.y) * RAD2DEG) / 180.0f * height;
    } else {
        float dz = vec3_dot(s.direction, cam_forward);
        if (dz <= 0) return;
        px = (vec3_dot(s.direction, cam_right) / dz / (aspect * tan_half_fov) + 1.0f) * 0.5f * width;
        py = (1.0f - vec3_dot(s.direction, cam_up) / dz / tan_half_fov) * 0.5f * height;
    }

    if (px < -20 || px >= width + 20 || py < -20 || py >= height + 20) return;

    // Spectrum calculation (simplified blackbody)
    // We can't easily call host functions, so we reimplement or use device functions.
    // Assuming bv_to_temp and blackbody_spectrum logic is needed.
    // For performance, we can approximate or use pre-computed color.
    // But the requirement says "parity". So we implement it.
    
    float term1 = 1.0f / (0.92f * s.bv + 1.7f);
    float term2 = 1.0f / (0.92f * s.bv + 0.62f);
    float tempK = 4600.0f * (term1 + term2);

    Spectrum spec;
    // Blackbody inline
    double c1 = 1.191e-16; 
    double c2 = 1.4388e-2;
    for (int i = 0; i < SPECTRUM_BANDS; i++) {
        float lambda_nm = LAMBDA_START + i * LAMBDA_STEP;
        double lambda_m = lambda_nm * 1e-9;
        double power_term = pow(lambda_m, 5.0);
        double exp_term = exp(c2 / (lambda_m * tempK)) - 1.0;
        spec.s[i] = (float)(c1 / (power_term * exp_term));
    }

    XYZV star_xyzv = dev_spectrum_to_xyzv(&spec); // Uses device constant memory
    
    // Normalize flux (CPU logic: flux / xyz_bb.V)
    // Note: dev_spectrum_to_xyzv includes dLambda scaling, CPU spectrum_to_xyzv does too.
    float flux = powf(10.0f, -0.4f * s.vmag) * 2.0e-5f;
    float norm = star_xyzv.V + 1e-20f;
    float scale = flux / norm;
    
    star_xyzv.X *= scale;
    star_xyzv.Y *= scale;
    star_xyzv.Z *= scale;
    star_xyzv.V *= scale;

    float T = expf(-0.1f / (s.direction.y + 0.01f));
    star_xyzv.X *= T;
    star_xyzv.Y *= T;
    star_xyzv.Z *= T;
    star_xyzv.V *= T;

    float sigma_px;
    float solid_angle;
    
    if (env_map) {
        sigma_px = sigma_ang * width / 6.283185f;
        solid_angle = (6.283185f / width) * (3.14159f / height) * cosf(asinf(s.direction.y));
    } else {
        sigma_px = sigma_ang * pinhole_f_px;
        solid_angle = (4.0f * tan_half_fov * tan_half_fov * aspect) / (width * height);
    }

    if (sigma_px < 0.5f) sigma_px = 0.5f;

    int radius = (int)(sigma_px * 4.0f) + 1;
    int x_start = (int)px - radius;
    int x_end = (int)px + radius;
    int y_start = (int)py - radius;
    int y_end = (int)py + radius;

    if (x_start < 0) x_start = 0;
    if (x_end >= width) x_end = width - 1;
    if (y_start < 0) y_start = 0;
    if (y_end >= height) y_end = height - 1;

    float rad_factor = 1.0f / (solid_angle + 1e-15f);
    float inv_sigma_sqrt2 = 1.0f / (sigma_px * sqrtf(2.0f));

    for (int iy = y_start; iy <= y_end; iy++) {
        float y0 = (float)iy - py;
        float y1 = (float)iy + 1.0f - py;
        float weight_y = 0.5f * (erff(y1 * inv_sigma_sqrt2) - erff(y0 * inv_sigma_sqrt2));
        
        for (int ix = x_start; ix <= x_end; ix++) {
            float x0 = (float)ix - px;
            float x1 = (float)ix + 1.0f - px;
            float weight_x = 0.5f * (erff(x1 * inv_sigma_sqrt2) - erff(x0 * inv_sigma_sqrt2));
            
            float f = weight_x * weight_y * rad_factor;
            
            int p_idx = iy * width + ix;
            atomicAdd(&out_pixels[p_idx].X, star_xyzv.X * f);
            atomicAdd(&out_pixels[p_idx].Y, star_xyzv.Y * f);
            atomicAdd(&out_pixels[p_idx].Z, star_xyzv.Z * f);
            atomicAdd(&out_pixels[p_idx].V, star_xyzv.V * f);
        }
    }
}

extern "C" bool cuda_render_stars(
    int width, int height,
    const RenderCamera* cam,
    float aperture,
    XYZV* out_pixels
) {
    if (d_stars == NULL || d_num_stars == 0) return true;
    if (d_pixels == NULL) return false; // Should be allocated by render_frame

    // 550nm constants matching CPU
    float lambda_550nm = 550.0f;
    float theta_550nm = 1.22f * (lambda_550nm * 1e-9f) / (aperture * 1e-3f);
    float sigma_ang = theta_550nm * 0.42f;

    float pinhole_f_px = 0;
    if (!cam->env_map) {
        pinhole_f_px = height / (2.0f * cam->tan_half_fov);
    }

    int blockSize = 256;
    int numBlocks = (d_num_stars + blockSize - 1) / blockSize;

    render_stars_kernel<<<numBlocks, blockSize>>>(
        d_stars, d_num_stars,
        width, height,
        cam->forward, cam->right, cam->up,
        cam->tan_half_fov, cam->aspect,
        cam->env_map,
        sigma_ang,
        pinhole_f_px,
        d_pixels
    );

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Error (render_stars): %s\n", cudaGetErrorString(err));
        return false;
    }

    size_t size = width * height * sizeof(XYZV);
    cudaMemcpy(out_pixels, d_pixels, size, cudaMemcpyDeviceToHost);
    return true;
}
