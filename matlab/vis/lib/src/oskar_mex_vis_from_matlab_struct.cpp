/*
 * Copyright (c) 2012-2013, The University of Oxford
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

#include "matlab/vis/lib/oskar_mex_vis_from_matlab_struct.h"

#include <oskar_get_error_string.h>

#include <cstdlib>
#include <cstring>

#include <matrix.h>

static void mex_vis_error_(const char* msg)
{
    mexErrMsgIdAndTxt("OSKAR:ERROR", "Invalid input vis. structure (%s).\n",
            msg);
}


static void mex_vis_error_field_(const char* msg)
{
    mexErrMsgIdAndTxt("OSKAR:ERROR", "Invalid input vis. structure "
            "(missing field: %s).\n", msg);
}


oskar_Vis* oskar_mex_vis_from_matlab_struct(const mxArray* v_in)
{
    int err = 0;
    oskar_Vis* v_out = 0;

    if (v_in == NULL)
        mexErrMsgTxt("ERROR: Invalid inputs.\n");

    /* Check input mxArray is a structure! */
    if (!mxIsStruct(v_in))
        mexErrMsgTxt("ERROR: Invalid input vis structure.\n");

    /* Read structure fields into local mxArrays */
    int num_fields = mxGetNumberOfFields(v_in);
    int num_fields_polarised = 32;
    int num_fields_scalar = 25;
    if (!(num_fields == num_fields_polarised || num_fields == num_fields_scalar))
    {
        mexPrintf("\nERROR: input vis structure has %i fields, "
                "expecting %i or %i.\n", num_fields, num_fields_polarised,
                num_fields_scalar);
        mex_vis_error_("incorrect number of fields");
    }

    int num_pols = (num_fields == num_fields_polarised) ? 4 : 1;
    mxArray* settings_path_ = mxGetField(v_in, 0, "settings_path");
    if (!settings_path_) mex_vis_error_field_("settings_path");
    mxArray* num_channels_ = mxGetField(v_in, 0, "num_channels");
    if (!num_channels_) mex_vis_error_field_("num_channels");
    mxArray* num_times_ = mxGetField(v_in, 0, "num_times");
    if (!num_times_) mex_vis_error_field_("num_times");
    mxArray* num_stations_ = mxGetField(v_in, 0, "num_stations");
    if (!num_stations_) mex_vis_error_field_("num_stations");
    mxArray* num_baselines_ = mxGetField(v_in, 0, "num_baselines");
    if (!num_baselines_) mex_vis_error_field_("num_baselines");
    mxArray* freq_start_hz_ = mxGetField(v_in, 0, "freq_start_hz");
    if (!freq_start_hz_) mex_vis_error_field_("freq_start_hz");
    mxArray* freq_inc_hz_ = mxGetField(v_in, 0, "freq_inc_hz");
    if (!freq_inc_hz_) mex_vis_error_field_("freq_inc_hz");
    mxArray* channel_bandwidth_hz_ = mxGetField(v_in, 0, "channel_bandwidth_hz");
    if (!channel_bandwidth_hz_) mex_vis_error_field_("channel_bandwidth_hz");
    mxArray* time_start_mjd_utc_ = mxGetField(v_in, 0, "time_start_mjd_utc");
    if (!time_start_mjd_utc_) mex_vis_error_field_("time_start_mjd_utc");
    mxArray* time_inc_sec_ = mxGetField(v_in, 0, "time_inc_seconds");
    if (!time_inc_sec_) mex_vis_error_field_("time_inc_seconds");
    mxArray* phase_centre_ra_deg_ = mxGetField(v_in, 0, "phase_centre_ra_deg");
    if (!phase_centre_ra_deg_) mex_vis_error_field_("phase_centre_ra_deg");
    mxArray* phase_centre_dec_deg_ = mxGetField(v_in, 0, "phase_centre_dec_deg");
    if (!phase_centre_dec_deg_) mex_vis_error_field_("phase_centre_dec_deg");
    mxArray* freq_ = mxGetField(v_in, 0, "frequency");
    if (!freq_) mex_vis_error_field_("frequency");
    mxArray* time_ = mxGetField(v_in, 0, "time");
    if (!time_) mex_vis_error_field_("time");
    mxArray* coord_units_ = mxGetField(v_in, 0, "coord_units");
    if (!coord_units_) mex_vis_error_field_("coord_units");
    mxArray* x_ = mxGetField(v_in, 0, "station_x");
    if (!x_) mex_vis_error_field_("station_x");
    mxArray* y_ = mxGetField(v_in, 0, "station_y");
    if (!y_) mex_vis_error_field_("station_y");
    mxArray* z_ = mxGetField(v_in, 0, "station_z");
    if (!z_) mex_vis_error_field_("station_z");
    mxArray* uu_ = mxGetField(v_in, 0, "uu");
    if (!uu_) mex_vis_error_field_("uu");
    mxArray* vv_ = mxGetField(v_in, 0, "vv");
    if (!vv_) mex_vis_error_field_("vv");
    mxArray* ww_ = mxGetField(v_in, 0, "ww");
    if (!ww_) mex_vis_error_field_("ww");
    mxArray* axis_order_ = mxGetField(v_in, 0, "axis_order");
    if (!axis_order_) mex_vis_error_field_("axis_order");
    mxArray* xx_ = mxGetField(v_in, 0, "xx");
    if (!xx_) mex_vis_error_field_("xx");
    mxArray *xy_ = NULL, *yx_ = NULL, *yy_ = NULL;
    if (num_pols == 4)
    {
        xy_ = mxGetField(v_in, 0, "xy");
        if (!xy_) mex_vis_error_field_("xy");
        yx_ = mxGetField(v_in, 0, "yx");
        if (!yx_) mex_vis_error_field_("yx");
        yy_ = mxGetField(v_in, 0, "yy");
        if (!yy_) mex_vis_error_field_("yy");
    }

    /* Get dimensions */
    int num_stations  = (int)mxGetScalar(num_stations_);
    int num_baselines = (int)mxGetScalar(num_baselines_);
    int num_times     = (int)mxGetScalar(num_times_);
    int num_channels  = (int)mxGetScalar(num_channels_);

    /* Get the data type */
    int type = 0;
    if (mxIsDouble(uu_)) type = OSKAR_DOUBLE;
    else if (mxIsSingle(uu_)) type = OSKAR_SINGLE;
    else mexErrMsgTxt("ERROR: Invalid input vis structure.\n");

    /* Initialise oskar_Visibility structure */
    int location = OSKAR_LOCATION_CPU;
    v_out = oskar_vis_create(type | OSKAR_COMPLEX | OSKAR_MATRIX,
            location, num_channels, num_times, num_stations, &err);
    if (err)
    {
        mexErrMsgIdAndTxt("OSKAR:ERROR", "oskar_vis_init() "
            "failed with code %i: %s.\n", err, oskar_get_error_string(err));
    }

    /* Set meta-data */
    int length = mxGetN(settings_path_);
    size_t buflen = (length * sizeof(mxChar)) + 1;;
    char* buffer = (char*)mxMalloc(buflen);
    mxGetString(settings_path_, buffer, (mwSize)buflen);
    oskar_mem_realloc(oskar_vis_settings_path(v_out), length + 1, &err);
    if (err)
    {
        mexErrMsgIdAndTxt("OSKAR:ERROR", "oskar_mem_init() "
                " failed with code %i: %s.\n", err,
                oskar_get_error_string(err));
    }
    strcpy(oskar_mem_char(oskar_vis_settings_path(v_out)), buffer);
    mxFree(buffer);

    oskar_vis_set_freq_start_hz(v_out, mxGetScalar(freq_start_hz_));
    oskar_vis_set_freq_inc_hz(v_out, mxGetScalar(freq_inc_hz_));
    oskar_vis_set_time_start_mjd_utc(v_out, mxGetScalar(time_start_mjd_utc_));
    oskar_vis_set_time_inc_seconds(v_out, mxGetScalar(time_inc_sec_));
    oskar_vis_set_channel_bandwidth_hz(v_out, mxGetScalar(channel_bandwidth_hz_));
    oskar_vis_set_phase_centre(v_out, mxGetScalar(phase_centre_ra_deg_),
            mxGetScalar(phase_centre_dec_deg_));

    /* check coordinates units field */
    char str_coord_units[20];
    mxGetString(coord_units_, str_coord_units, 20);
    if (strcmp(str_coord_units, "metres")) mex_vis_error_("coord units != metres");

    /* Check dimensions of coordinate fields */
    int num_dims = mxGetNumberOfDimensions(uu_);
    if (num_dims > 3) mex_vis_error_("_dimension mismatch");
    const mwSize* dims = mxGetDimensions(uu_);
    if ((int)dims[0] != num_baselines) mex_vis_error_("dimension mismatch");

    /* Set coordinate fields */
    size_t coord_size = num_times * num_baselines;
    coord_size *= (type == OSKAR_DOUBLE) ? sizeof(double) : sizeof(float);
    memcpy(oskar_mem_void(oskar_vis_baseline_uu_metres(v_out)), mxGetData(uu_),
            coord_size);
    memcpy(oskar_mem_void(oskar_vis_baseline_vv_metres(v_out)), mxGetData(vv_),
            coord_size);
    memcpy(oskar_mem_void(oskar_vis_baseline_ww_metres(v_out)), mxGetData(ww_),
            coord_size);

    size_t mem_size = num_stations;
    mem_size *= (type == OSKAR_DOUBLE) ? sizeof(double) : sizeof(float);
    memcpy(oskar_mem_void(oskar_vis_station_x_metres(v_out)), mxGetData(x_),
            mem_size);
    memcpy(oskar_mem_void(oskar_vis_station_y_metres(v_out)), mxGetData(y_),
            mem_size);
    memcpy(oskar_mem_void(oskar_vis_station_z_metres(v_out)), mxGetData(z_),
            mem_size);

    /* Set amplitude fields */
    if (num_pols == 4)
    {
        if (type == OSKAR_DOUBLE)
        {
            double* xx_r = mxGetPr(xx_); double* xx_i = mxGetPi(xx_);
            double* xy_r = mxGetPr(xy_); double* xy_i = mxGetPi(xy_);
            double* yx_r = mxGetPr(yx_); double* yx_i = mxGetPi(yx_);
            double* yy_r = mxGetPr(yy_); double* yy_i = mxGetPi(yy_);
            double4c* amp_ = oskar_mem_double4c(oskar_vis_amplitude(v_out),
                    &err);
            for (int c = 0; c < num_channels; ++c)
            {
                for (int t = 0; t < num_times; ++t)
                {
                    for (int b = 0; b < num_baselines; ++b)
                    {
                        int idx = num_baselines * (c*num_times + t) + b;
                        amp_[idx].a.x = xx_r[idx];
                        amp_[idx].a.y = xx_i[idx];
                        amp_[idx].b.x = xy_r[idx];
                        amp_[idx].b.y = xy_i[idx];
                        amp_[idx].c.x = yx_r[idx];
                        amp_[idx].c.y = yx_i[idx];
                        amp_[idx].d.x = yy_r[idx];
                        amp_[idx].d.y = yy_i[idx];
                    }
                }
            }
        }
        else
        {
            float* xx_r = (float*)mxGetPr(xx_); float* xx_i = (float*)mxGetPi(xx_);
            float* xy_r = (float*)mxGetPr(xy_); float* xy_i = (float*)mxGetPi(xy_);
            float* yx_r = (float*)mxGetPr(yx_); float* yx_i = (float*)mxGetPi(yx_);
            float* yy_r = (float*)mxGetPr(yy_); float* yy_i = (float*)mxGetPi(yy_);
            float4c* amp_ = oskar_mem_float4c(oskar_vis_amplitude(v_out), &err);
            for (int c = 0; c < num_channels; ++c)
            {
                for (int t = 0; t < num_times; ++t)
                {
                    for (int b = 0; b < num_baselines; ++b)
                    {
                        int idx = num_baselines * (c*num_times + t) + b;
                        amp_[idx].a.x = xx_r[idx];
                        amp_[idx].a.y = xx_i[idx];
                        amp_[idx].b.x = xy_r[idx];
                        amp_[idx].b.y = xy_i[idx];
                        amp_[idx].c.x = yx_r[idx];
                        amp_[idx].c.y = yx_i[idx];
                        amp_[idx].d.x = yy_r[idx];
                        amp_[idx].d.y = yy_i[idx];
                    }
                }
            }
        }
    }
    else /* num_pols == 1 */
    {
        if (type == OSKAR_DOUBLE)
        {
            double* xx_r = mxGetPr(xx_); double* xx_i = mxGetPi(xx_);
            double2* amp_ = oskar_mem_double2(oskar_vis_amplitude(v_out), &err);
            for (int c = 0; c < num_channels; ++c)
            {
                for (int t = 0; t < num_times; ++t)
                {
                    for (int b = 0; b < num_baselines; ++b)
                    {
                        int idx = num_baselines * (c*num_times + t) + b;
                        amp_[idx].x = xx_r[idx];
                        amp_[idx].y = xx_i[idx];
                    }
                }
            }
        }
        else
        {
            float* xx_r = (float*)mxGetPr(xx_); float* xx_i = (float*)mxGetPi(xx_);
            float2* amp_ = oskar_mem_float2(oskar_vis_amplitude(v_out), &err);
            for (int c = 0; c < num_channels; ++c)
            {
                for (int t = 0; t < num_times; ++t)
                {
                    for (int b = 0; b < num_baselines; ++b)
                    {
                        int idx = num_baselines * (c*num_times + t) + b;
                        amp_[idx].x = xx_r[idx];
                        amp_[idx].y = xx_i[idx];
                    }
                }
            }
        }
    }

#if 0
    mexPrintf("settings path     = '%s'\n", (char*)v_out->settings_path.data);
    mexPrintf("num_channels      = %i\n", v_out->num_channels);
    mexPrintf("num_time          = %i\n", v_out->num_times);
    mexPrintf("num_stations      = %i\n", v_out->num_stations);
    mexPrintf("num_baselines     = %i\n", v_out->num_baselines);
    mexPrintf("start freq.       = %f\n", v_out->freq_start_hz);
    mexPrintf("freq inc          = %f\n", v_out->freq_inc_hz);
    mexPrintf("channel bandwidth = %f\n", v_out->channel_bandwidth_hz);
    mexPrintf("start time        = %f\n", v_out->time_start_mjd_utc);
    mexPrintf("time inc          = %f\n", v_out->time_inc_seconds);
    mexPrintf("ra0               = %f\n", v_out->phase_centre_ra_deg);
    mexPrintf("dec0              = %f\n", v_out->phase_centre_dec_deg);
#endif

    return v_out;
}