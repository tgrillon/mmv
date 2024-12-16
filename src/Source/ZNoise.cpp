#include "ZNoise.h"

#include "Utils.h"

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

        PNG2D noise(inter_func == 0 ? lerp : inter_func == 1 ? cosine
                                                             : smoothstep);

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

                if (value > maxNoiseVal)
                    maxNoiseVal = value;
                elevations[z * w + x] = value;
            }
        }

        for (float &e : elevations)
            e /= maxNoiseVal;

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

namespace znoise
{
    std::vector<float> generate_perlin(const std::string &filename, float scale, int width, int height)
    {
        Perlin perlin;
        perlin.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int r = 0; r < height; ++r)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = perlin.Get({(float)i, (float)r}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[r * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(r * width + i) * 3 + 0] = value;
                image.pixels[(r * width + i) * 3 + 1] = value;
                image.pixels[(r * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_perlin] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_perlin_3dslice(const std::string &filename, float scale, int width, int height)
    {
        Perlin perlin;
        perlin.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int r = 0; r < height; ++r)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = perlin.Get({(float)i, (float)r, 0.0f}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[r * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(r * width + i) * 3 + 0] = value;
                image.pixels[(r * width + i) * 3 + 1] = value;
                image.pixels[(r * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_perlin_3dslice] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_perlin_4dslice(const std::string &filename, float scale, int width, int height)
    {
        Perlin perlin;
        perlin.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int r = 0; r < height; ++r)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = perlin.Get({(float)i, (float)r, 0.0f, 1.0f}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[r * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(r * width + i) * 3 + 0] = value;
                image.pixels[(r * width + i) * 3 + 1] = value;
                image.pixels[(r * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_perlin_4dslice] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_simplex(const std::string &filename, float scale, int width, int height)
    {
        Simplex simplex;
        simplex.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = simplex.Get({(float)i, (float)j}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[j * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(j * width + i) * 3 + 0] = value;
                image.pixels[(j * width + i) * 3 + 1] = value;
                image.pixels[(j * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_simplex] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_simplex_3dslice(const std::string &filename, float scale, int width, int height)
    {
        Simplex simplex;
        simplex.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = simplex.Get({(float)i, (float)j, 1.0f}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[j * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(j * width + i) * 3 + 0] = value;
                image.pixels[(j * width + i) * 3 + 1] = value;
                image.pixels[(j * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_simplex_3dslice] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_simplex_4dslice(const std::string &filename, float scale, int width, int height)
    {
        Simplex simplex;
        simplex.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = simplex.Get({(float)i, (float)j, 1.0f, 2.0f}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[j * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(j * width + i) * 3 + 0] = value;
                image.pixels[(j * width + i) * 3 + 1] = value;
                image.pixels[(j * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_simplex_4dslice] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_worley(const std::string &filename, float scale, int width, int height, WorleyFunction worleyFunc)
    {
        Worley worley;
        worley.Shuffle(10);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = worley.Get({(float)i, (float)j}, 0.01f);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[j * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(j * width + i) * 3 + 0] = value;
                image.pixels[(j * width + i) * 3 + 1] = value;
                image.pixels[(j * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_worley] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_hmf(const std::string &filename, float scale, int width, int height, float hurst, float lacunarity, float baseScale, int x_offset, int y_offset, unsigned int seed)
    {
        Simplex simplex;
        simplex.SetSeed(seed);
        simplex.Shuffle(10);

        HybridMultiFractal hmf(simplex);
        hmf.SetParameters(hurst, lacunarity, 5.f);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = (hmf.Get({(float)i + x_offset, (float)j + y_offset}, baseScale) + 1.0f) * 0.5f;
                auto value = static_cast<unsigned char>(h * 255.f);

                elevations[j * width + i] = h * scale;

                image.pixels[(j * width + i) * 3 + 0] = value;
                image.pixels[(j * width + i) * 3 + 1] = value;
                image.pixels[(j * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_hmf] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

    std::vector<float> generate_fbm(const std::string &filename, float scale, int width, int height, float hurst, float lacunarity, float baseScale, int x_offset, int y_offset, unsigned int seed)
    {
        Simplex simplex;
        simplex.SetSeed(seed);
        simplex.Shuffle(10);

        FBM fbm(simplex);
        fbm.SetParameters(hurst, lacunarity, 5.f);

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::vector<float> elevations(width * height);
        ImageData image(width, height, 3);

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                float h = fbm.Get({(float)i + x_offset, (float)j + y_offset}, baseScale);
                auto value = static_cast<unsigned char>((h + 1.f) * 0.5f * 255.f);

                elevations[j * width + i] = (h + 1.f) * 0.5f * scale;

                image.pixels[(j * width + i) * 3 + 0] = value;
                image.pixels[(j * width + i) * 3 + 1] = value;
                image.pixels[(j * width + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[generate_fbm] Image ", filename, " successfully saved in ./data/output");

        return elevations;
    }

} // namespace znoise
