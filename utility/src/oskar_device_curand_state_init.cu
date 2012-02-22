/*
 * Copyright (c) 2011, The University of Oxford
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

#include "utility/oskar_device_curand_state_init.h"
#include "utility/cudak/oskar_cudak_curand_state_init.h"
#include <curand_kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

int oskar_device_curand_state_init(curandState* d_states, int num_states,
        int seed, int offset, int use_device_offset)
{
    if (d_states == NULL)
        return OSKAR_ERR_INVALID_ARGUMENT;

    int error = OSKAR_SUCCESS;
    int num_threads = 128;
    int num_blocks = (num_states + num_threads - 1) / num_threads;

    // Note: device_offset allocates different states from same seed to span
    // multiple GPUs.
    int device_offset = 0;
    if (use_device_offset)
    {
        int device_id = 0;
        cudaGetDevice(&device_id);
        device_offset = device_id * num_states;
    }

    oskar_cudak_curand_state_init
        OSKAR_CUDAK_CONF(num_blocks, num_threads)
        (d_states, seed, offset, device_offset);
    error = cudaPeekAtLastError();

    return error;
}

#ifdef __cplusplus
}
#endif
