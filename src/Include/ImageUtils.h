#pragma once

#include "pch.h"

#include "Type.h"

namespace kernel
{
    const int smooth_size = 3;
    const float ds = 1. / 16.;
    const float smooth[9] = {
        ds, 2. * ds, ds,
        2. * ds, 4. * ds, 2. * ds,
        ds, 2. * ds, ds};

    const int blur_size = 3;
    const float db = 1. / 9.;
    const float blur[9] = {
        db, db, db,
        db, db, db,
        db, db, db};

    const int gauss_size = 3;
    const float dg = 1. / 256.;
    const float gauss[25] = {
        dg * 1., dg * 4., dg * 6., dg * 4., dg * 1.,
        dg * 4., dg * 16., dg * 24., dg * 16., dg * 4.,
        dg * 6., dg * 24., dg * 36., dg * 24., dg * 6.,
        dg * 4., dg * 16., dg * 24., dg * 16., dg * 4.,
        dg * 1., dg * 4., dg * 6., dg * 4., dg * 1.};
} // namespace kernel

void convolve(const std::vector<scalar_t> &input, std::vector<scalar_t> &output, const int nx, const int ny, const float *kernel, const int nk = 3);
