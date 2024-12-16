#pragma once 

#include "pch.h"

#include "Type.h"

namespace kernel 
{
    const scalar_t gauss[9] = {};
} // namespace kernel 

void convolve(const std::vector<scalar_t>& input, std::vector<scalar_t>& output, int nx, int ny, const float* kernel);
