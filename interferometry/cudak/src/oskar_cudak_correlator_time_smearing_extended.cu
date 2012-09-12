/*
 * Copyright (c) 2012, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "interferometry/cudak/oskar_cudak_correlator_extended.h"
#include "math/cudak/oskar_cudaf_mul_mat2c_mat2c.h"
#include "math/cudak/oskar_cudaf_mul_mat2c_mat2h.h"
#include "math/cudak/oskar_cudaf_mul_mat2c_mat2c_conj_trans.h"
#include "math/cudak/oskar_cudaf_sinc.h"
#include <math.h>

#define ONE_OVER_2PI  0.159154943091895335768884   // 1 / (2 * pi)
#define ONE_OVER_2PIf 0.159154943091895335768884f  // 1 / (2 * pi)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif

// Indices into the visibility/baseline matrix.
#define SI blockIdx.x // Column index.
#define SJ blockIdx.y // Row index.

extern __shared__ float4c  smem_f[];
extern __shared__ double4c smem_d[];

// Single precision.
__global__
void oskar_cudak_correlator_time_smearing_extended_f(const int num_sources,
        const int num_stations, const float4c* jones, const float* source_I,
        const float* source_Q, const float* source_U, const float* source_V,
        const float* u, const float* v, const float* x, const float* y,
        const float* z, const float* l, const float* m, const float freq,
        const float bandwidth, const float time_interval, const float gha0,
        const float dec0, const float* a, const float* b, const float* c,
        float4c* vis)
{
    // Return immediately if we're in the lower triangular half of the
    // visibility matrix.
    if (SJ >= SI) return;

    // Common things per thread block.
    __device__ __shared__ float uu, vv, uu2, vv2, uuvv;
    if (threadIdx.x == 0)
    {
        // Baseline UV-distance, in wavelengths.
        uu   = (u[SI] - u[SJ]) * ONE_OVER_2PIf;
        vv   = (v[SI] - v[SJ]) * ONE_OVER_2PIf;

        // Quantities needed for evaluating source with gaussian term.
        uu2  = uu * uu;
        vv2  = vv * vv;
        uuvv = 2.0f * uu * vv;

        // Modify the baseline UV-distance to include the common components
        // of the bandwidth smearing term.
        uu *= M_PIf * bandwidth / freq;
        vv *= M_PIf * bandwidth / freq;
    }
    __syncthreads();

    // Get pointers to both source vectors for station i and j.
    const float4c* station_i = &jones[num_sources * SI];
    const float4c* station_j = &jones[num_sources * SJ];

    // Each thread loops over a subset of the sources.
    {
        float4c sum; // Partial sum per thread.
        sum.a = make_float2(0.0f, 0.0f);
        sum.b = make_float2(0.0f, 0.0f);
        sum.c = make_float2(0.0f, 0.0f);
        sum.d = make_float2(0.0f, 0.0f);
        for (int t = threadIdx.x; t < num_sources; t += blockDim.x)
        {
            // Compute bandwidth-smearing term first (register optimisation).
            float rb = oskar_cudaf_sinc_f(uu * source_l[t] + vv * source_m[t]);

            // Evaluate gaussian source width term.
            float f = expf(-(source_a[t] * uu2 +
                    source_b[t] * uuvv + source_c[t] * vv2));

            rb *= f;

            // Construct source brightness matrix.
            float4c c_b;
            {
                float s_I = source_I[t];
                float s_Q = source_Q[t];
                c_b.b.x = source_U[t];
                c_b.b.y = source_V[t];
                c_b.a.x = s_I + s_Q;
                c_b.d.x = s_I - s_Q;
            }

            // Multiply first Jones matrix with source coherency matrix.
            float4c c_a = station_i[t];
            oskar_cudaf_mul_mat2c_mat2h_f(c_a, c_b);

            // Multiply result with second (Hermitian transposed) Jones matrix.
            c_b = station_j[t];
            oskar_cudaf_mul_mat2c_mat2c_conj_trans_f(c_a, c_b);

            // Multiply result by bandwidth-smearing term.
            sum.a.x += c_a.a.x * rb;
            sum.a.y += c_a.a.y * rb;
            sum.b.x += c_a.b.x * rb;
            sum.b.y += c_a.b.y * rb;
            sum.c.x += c_a.c.x * rb;
            sum.c.y += c_a.c.y * rb;
            sum.d.x += c_a.d.x * rb;
            sum.d.y += c_a.d.y * rb;
        }
        smem_f[threadIdx.x] = sum;
    }
    __syncthreads();

    // Accumulate contents of shared memory.
    if (threadIdx.x == 0)
    {
        // Sum over all sources for this baseline.
        float4c sum;
        sum.a = make_float2(0.0f, 0.0f);
        sum.b = make_float2(0.0f, 0.0f);
        sum.c = make_float2(0.0f, 0.0f);
        sum.d = make_float2(0.0f, 0.0f);
        for (int i = 0; i < blockDim.x; ++i)
        {
            sum.a.x += smem_f[i].a.x;
            sum.a.y += smem_f[i].a.y;
            sum.b.x += smem_f[i].b.x;
            sum.b.y += smem_f[i].b.y;
            sum.c.x += smem_f[i].c.x;
            sum.c.y += smem_f[i].c.y;
            sum.d.x += smem_f[i].d.x;
            sum.d.y += smem_f[i].d.y;
        }

        // Determine 1D index.
        int idx = SJ*(num_stations-1) - (SJ-1)*SJ/2 + SI - SJ - 1;

        // Modify existing visibility.
        vis[idx].a.x += sum.a.x;
        vis[idx].a.y += sum.a.y;
        vis[idx].b.x += sum.b.x;
        vis[idx].b.y += sum.b.y;
        vis[idx].c.x += sum.c.x;
        vis[idx].c.y += sum.c.y;
        vis[idx].d.x += sum.d.x;
        vis[idx].d.y += sum.d.y;
    }
}

// Double precision.
__global__
void oskar_cudak_correlator_time_smearing_extended_d(const int num_sources,
        const int num_stations, const double4c* jones, const double* source_I,
        const double* source_Q, const double* source_U, const double* source_V,
        const double* u, const double* v, const double* x, const double* y,
        const double* z, const double* l, const double* m, const double freq,
        const double bandwidth, const double time_interval, const double gha0,
        const double dec0, const double* a, const double* b, const double* c,
        double4c* vis)
{
    // Return immediately if we're in the lower triangular half of the
    // visibility matrix.
    if (SJ >= SI) return;

    // Common things per thread block.
    __device__ __shared__ double uu, vv, uu2, vv2, uuvv;
    if (threadIdx.x == 0)
    {
        // Baseline UV-distance, in wavelengths.
        uu   = (u[SI] - u[SJ]) * ONE_OVER_2PI;
        vv   = (v[SI] - v[SJ]) * ONE_OVER_2PI;

        // Quantities needed for evaluating source with gaussian term.
        uu2  = uu * uu;
        vv2  = vv * vv;
        uuvv = 2.0 * uu * vv;

        // Modify the baseline UV-distance to include the common components
        // of the bandwidth smearing term.
        uu *= M_PI * bandwidth / freq;
        vv *= M_PI * bandwidth / freq;
    }
    __syncthreads();

    // Get pointers to both source vectors for station i and j.
    const double4c* station_i = &jones[num_sources * SI];
    const double4c* station_j = &jones[num_sources * SJ];

    // Each thread loops over a subset of the sources.
    {
        double4c sum; // Partial sum per thread.
        sum.a = make_double2(0.0, 0.0);
        sum.b = make_double2(0.0, 0.0);
        sum.c = make_double2(0.0, 0.0);
        sum.d = make_double2(0.0, 0.0);
        for (int t = threadIdx.x; t < num_sources; t += blockDim.x)
        {
            // Compute bandwidth-smearing term first (register optimisation).
            double rb = oskar_cudaf_sinc_d(uu * l[t] + vv * m[t]);

            // Evaluate gaussian source width term.
            double f = exp(-(a[t] * uu2 + b[t] * uuvv + c[t] * vv2));

            rb *= f;

            // Construct source brightness matrix.
            double4c c_b;
            {
                double s_I = source_I[t];
                double s_Q = source_Q[t];
                c_b.b.x = source_U[t];
                c_b.b.y = source_V[t];
                c_b.a.x = s_I + s_Q;
                c_b.d.x = s_I - s_Q;
            }

            // Multiply first Jones matrix with source coherency matrix.
            double4c c_a = station_i[t];
            oskar_cudaf_mul_mat2c_mat2h_d(c_a, c_b);

            // Multiply result with second (Hermitian transposed) Jones matrix.
            c_b = station_j[t];
            oskar_cudaf_mul_mat2c_mat2c_conj_trans_d(c_a, c_b);

            // Multiply result by bandwidth-smearing term.
            sum.a.x += c_a.a.x * rb;
            sum.a.y += c_a.a.y * rb;
            sum.b.x += c_a.b.x * rb;
            sum.b.y += c_a.b.y * rb;
            sum.c.x += c_a.c.x * rb;
            sum.c.y += c_a.c.y * rb;
            sum.d.x += c_a.d.x * rb;
            sum.d.y += c_a.d.y * rb;
        }
        smem_d[threadIdx.x] = sum;
    }
    __syncthreads();

    // Accumulate contents of shared memory.
    if (threadIdx.x == 0)
    {
        // Sum over all sources for this baseline.
        double4c sum;
        sum.a = make_double2(0.0, 0.0);
        sum.b = make_double2(0.0, 0.0);
        sum.c = make_double2(0.0, 0.0);
        sum.d = make_double2(0.0, 0.0);
        for (int i = 0; i < blockDim.x; ++i)
        {
            sum.a.x += smem_d[i].a.x;
            sum.a.y += smem_d[i].a.y;
            sum.b.x += smem_d[i].b.x;
            sum.b.y += smem_d[i].b.y;
            sum.c.x += smem_d[i].c.x;
            sum.c.y += smem_d[i].c.y;
            sum.d.x += smem_d[i].d.x;
            sum.d.y += smem_d[i].d.y;
        }

        // Determine 1D index.
        int idx = SJ*(num_stations-1) - (SJ-1)*SJ/2 + SI - SJ - 1;

        // Modify existing visibility.
        vis[idx].a.x += sum.a.x;
        vis[idx].a.y += sum.a.y;
        vis[idx].b.x += sum.b.x;
        vis[idx].b.y += sum.b.y;
        vis[idx].c.x += sum.c.x;
        vis[idx].c.y += sum.c.y;
        vis[idx].d.x += sum.d.x;
        vis[idx].d.y += sum.d.y;
    }
}