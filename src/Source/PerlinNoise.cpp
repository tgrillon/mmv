#include "PerlinNoise.h"

namespace mmv
{
    float lerp(float a, float b, float t)
    {
        assert(t >= 0.f && t <= 1.f);
        return a * (1 - t) + b * t;
    }

    float smoothstep(float a, float b, float t)
    {
        assert(t >= 0.f && t <= 1.f);
        float tRemapSmoothstep = t * t * (3.f - 2.f * t);
        return lerp(a, b, tRemapSmoothstep);
    }

    float cosine(float a, float b, float t)
    {
        assert(t >= 0.f && t <= 1.f);
        float tRemapCosine = (1.f - cos(t * M_PI)) * 0.5f;
        return lerp(a, b, tRemapCosine);
    }

    std::vector<float> generate_height_map(int w, int h, int n_octaves, float amplitude, float frequency, unsigned char inter_func)
    {
        assert(w > 0 && h > 0);

        std::vector<float> elevations(w * h, 0.f);

        PNG2D noise(inter_func == 0 ? lerp : inter_func == 1 ? cosine : smoothstep); 

        float maxNoiseVal = 0.f;
        for (int z = 0; z < h; ++z)
        {
            for (int x = 0; x < w; ++x)
            {
                float value = 0.f;
                float ak = amplitude;
                float fk = frequency;
                for (int k = 0; k < n_octaves; ++k)
                {
                    value += ak * noise.eval(x * fk, z * fk);
                    ak *= 0.5f;
                    fk *= 2.0f;
                }

                if (value > maxNoiseVal) maxNoiseVal = value;
                elevations[z * w + x] = value;
            }
        }

        for (float& e : elevations) e /= maxNoiseVal;  

        return elevations;
    }

    PerlinNoiseGenerator1D::PerlinNoiseGenerator1D(const std::function<float(float, float, float)> &inter_func) : m_InterpolationFunc(inter_func)
    {
        for (int i = 0; i < s_MaxVertices; ++i)
        {
            m_RandomValues[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }
    }

    float PerlinNoiseGenerator1D::eval(float x) const
    {
        int ix = static_cast<int>(floor(x));
        int xMin = ix & s_MaxVerticesMask;
        int xMax = (xMin + 1) & s_MaxVerticesMask;

        float t = x - floor(x);
        return m_InterpolationFunc(m_RandomValues[xMin], m_RandomValues[xMax], t);
    }

    PerlinNoiseGenerator2D::PerlinNoiseGenerator2D(const std::function<float(float, float, float)> &inter_func) : m_InterpolationFunc(inter_func)
    {
        std::random_device hwseed;
        std::default_random_engine rng(hwseed());
        std::uniform_real_distribution<float> distFloat(0.f, 1.f);

        for (int i = 0; i < s_MaxVertices * s_MaxVertices; ++i)
        {
            m_RandomValues[i] = distFloat(rng);
        }
    }

    float PerlinNoiseGenerator2D::eval(const vec2 &p) const
    {
        return eval(p.x, p.y);
    }

    float PerlinNoiseGenerator2D::eval(float x, float y) const
    {
        int ix = static_cast<int>(floor(x));
        int iy = static_cast<int>(floor(y));

        float tx = x - ix;
        float ty = y - iy;

        int rx0 = ix & s_MaxVerticesMask;
        int rx1 = (rx0 + 1) & s_MaxVerticesMask;
        int ry0 = iy & s_MaxVerticesMask;
        int ry1 = (ry0 + 1) & s_MaxVerticesMask;

        // random values at the corners of the cell using the permutation table
        float c00 = m_RandomValues[ry0 * s_MaxVertices + rx0];
        float c10 = m_RandomValues[ry0 * s_MaxVertices + rx1];
        float c01 = m_RandomValues[ry1 * s_MaxVertices + rx0];
        float c11 = m_RandomValues[ry1 * s_MaxVertices + rx1];

        float nx0 = m_InterpolationFunc(c00, c10, tx); 
        float nx1 = m_InterpolationFunc(c01, c11, tx); 

        return m_InterpolationFunc(nx0, nx1, ty);
    }

} // namespace mmv
