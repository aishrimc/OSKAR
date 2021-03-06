/*
 * Copyright (c) 2012-2015, The University of Oxford
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

#include "vis/private_vis.h"
#include "vis/oskar_vis.h"

#ifdef __cplusplus
extern "C" {
#endif

void oskar_vis_free(oskar_Vis* vis, int* status)
{
    if (!vis) return;

    /* Free memory. */
    oskar_mem_free(vis->settings_path, status);
    oskar_mem_free(vis->telescope_path, status);
    oskar_mem_free(vis->settings, status);
    oskar_mem_free(vis->station_x_offset_ecef_metres, status);
    oskar_mem_free(vis->station_y_offset_ecef_metres, status);
    oskar_mem_free(vis->station_z_offset_ecef_metres, status);
    oskar_mem_free(vis->baseline_uu_metres, status);
    oskar_mem_free(vis->baseline_vv_metres, status);
    oskar_mem_free(vis->baseline_ww_metres, status);
    oskar_mem_free(vis->amplitude, status);

    /* Free the structure itself. */
    free(vis);
}

#ifdef __cplusplus
}
#endif
