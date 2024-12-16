#pragma once

#include "pch.h"

namespace mmv
{

    float cosine(float a, float b, float t);
    float smoothstep(float a, float b, float t);
    float lerp(float a, float b, float t);

    //! Computes perlin noise height map
    std::vector<float> generate_height_map(int w, int h, int n_octaves = 4, float amplitude = 1.f, float frequency = 1.f, unsigned char inter_func = 1);

    //! Thanks to https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/procedural-patterns-noise-part-1/creating-simple-2D-noise.html

    //! 1-dimensional Perlin noise generator
    class PerlinNoiseGenerator1D
    {
    public:
        PerlinNoiseGenerator1D(const std::function<float(float, float, float)> &inter_func);

        float eval(float x) const;

        inline float operator()(int i) const { return m_RandomValues.at(i); }

    public:
        static const int s_MaxVertices = 256;
        static const int s_MaxVerticesMask = s_MaxVertices - 1;

    private:
        std::function<float(float, float, float)> m_InterpolationFunc;
        std::array<float, s_MaxVertices> m_RandomValues;
    } typedef PNG1D;

    //! 2-dimensional Perlin noise generator
    class PerlinNoiseGenerator2D
    {
    public:
        PerlinNoiseGenerator2D(const std::function<float(float, float, float)> &inter_func);

        float eval(const vec2 &p) const;
        float eval(float x, float y) const;

    public:
        static const int s_MaxVertices = 256;
        static const int s_MaxVerticesMask = s_MaxVertices - 1;

    private:
        std::function<float(float, float, float)> m_InterpolationFunc;
        std::array<float, s_MaxVertices * s_MaxVertices> m_RandomValues;
    } typedef PNG2D;

} // namespace mmv

//! Znoise interface
namespace znoise
{
    std::vector<float> generate_perlin(const std::string &filename, float scale, int width, int height);

    std::vector<float> generate_perlin_3dslice(const std::string &filename, float scale, int width, int height);
    
    std::vector<float> generate_perlin_4dslice(const std::string &filename, float scale, int width, int height);
    
    std::vector<float> generate_simplex(const std::string &filename, float scale, int width, int height);
    
    std::vector<float> generate_simplex_3dslice(const std::string &filename, float scale, int width, int height);
    
    std::vector<float> generate_simplex_4dslice(const std::string &filename, float scale, int width, int height);
    
    std::vector<float> generate_worley(const std::string &filename, float scale, int width, int height, WorleyFunction worleyFunc);
    
    std::vector<float> generate_hmf(const std::string &filename, float scale, int width, int height, float hurst, float lacunarity, float baseScale, int x_offset = 0, int y_offset = 0, unsigned int seed = 0);
    
    std::vector<float> generate_fbm(const std::string &filename, float scale, int width, int height, float hurst, float lacunarity, float baseScale, int x_offset = 0, int y_offset = 0, unsigned int seed = 0);
} // namespace znoise
