/*
 * Copyright (c) 2016-2017, The University of Oxford
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

#ifdef OSKAR_HAVE_CUDA
#include <cufft.h>
#endif

#include "imager/private_imager.h"
#include "imager/oskar_imager.h"

#include "imager/private_imager_init_fft.h"
#include "imager/private_imager_free_fft.h"

#include "math/oskar_cmath.h"
#include "imager/oskar_fftpack_cfft.h"
#include "imager/oskar_fftpack_cfft_f.h"
#include "imager/oskar_grid_functions_spheroidal.h"
#include "imager/oskar_grid_functions_pillbox.h"

#ifdef __cplusplus
extern "C" {
#endif

void oskar_imager_init_fft(oskar_Imager* h, int* status)
{
    int grid_size;
    oskar_imager_free_fft(h, status);
    if (*status) return;

    /* Generate the convolution function. */
    grid_size = oskar_imager_plane_size(h);
    h->conv_func = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU,
            h->oversample * (h->support + 1), status);
    h->corr_func = oskar_mem_create(OSKAR_DOUBLE, OSKAR_CPU, grid_size, status);
    if (h->kernel_type == 'S')
    {
        oskar_grid_convolution_function_spheroidal(h->support, h->oversample,
                oskar_mem_double(h->conv_func, status));
        oskar_grid_correction_function_spheroidal(grid_size, 0,
                oskar_mem_double(h->corr_func, status));
    }
    else if (h->kernel_type == 'P')
    {
        oskar_grid_convolution_function_pillbox(h->support, h->oversample,
                oskar_mem_double(h->conv_func, status));
        oskar_grid_correction_function_pillbox(grid_size,
                oskar_mem_double(h->corr_func, status));
    }

    /* Set up the FFT. */
    if (h->fft_on_gpu)
    {
#ifdef OSKAR_HAVE_CUDA
        /* Generate FFT plan. */
        if (h->imager_prec == OSKAR_DOUBLE)
            cufftPlan2d(&h->cufft_plan, grid_size, grid_size, CUFFT_Z2Z);
        else
            cufftPlan2d(&h->cufft_plan, grid_size, grid_size, CUFFT_C2C);
#else
        *status = OSKAR_ERR_CUDA_NOT_AVAILABLE;
#endif
    }
    else
    {
        /* Initialise workspaces for CPU FFT algorithm. */
        int len_save = 4 * grid_size +
                2 * (int)(log((double)grid_size) / log(2.0)) + 8;
        h->fftpack_wsave = oskar_mem_create(h->imager_prec, OSKAR_CPU,
                len_save, status);
        h->fftpack_work = oskar_mem_create(h->imager_prec, OSKAR_CPU,
                2 * grid_size * grid_size, status);
        if (h->imager_prec == OSKAR_DOUBLE)
            oskar_fftpack_cfft2i(grid_size, grid_size,
                    oskar_mem_double(h->fftpack_wsave, status));
        else
            oskar_fftpack_cfft2i_f(grid_size, grid_size,
                    oskar_mem_float(h->fftpack_wsave, status));
    }
}

#ifdef __cplusplus
}
#endif