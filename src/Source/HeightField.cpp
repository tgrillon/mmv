#include "HeightField.h"

#include "vecext.h"

namespace mmv
{
    /***********************************************************/
    /************************* CLASS GRID **********************/
    Grid::Grid(const std::vector<float> &elements, const vec2 &a, const vec2 &b, size_t nx, size_t ny) : m_Nx(nx), m_Ny(ny), m_A(a), m_B(b), m_Elements(elements)
    {
        assert(nx * ny == elements.size());
    }

    float &Grid::operator()(size_t i, size_t j)
    {
        return m_Elements[j * m_Nx + i];
    }

    float Grid::At(size_t i, size_t j) const
    {
        assert(i >= 0 && i < m_Nx);
        assert(j >= 0 && j < m_Ny);

        return m_Elements[j * m_Nx + i];
    }

    size_t Grid::Nx() const
    {
        return m_Nx;
    }

    size_t Grid::Ny() const
    {
        return m_Ny;
    }

    vec2 Grid::A() const
    {
        return m_A;
    }

    vec2 Grid::B() const
    {
        return m_B;
    }

    float Grid::Min() const
    {
        return *std::min_element(m_Elements.begin(), m_Elements.end());
    }

    float Grid::Max() const
    {
        return *std::max_element(m_Elements.begin(), m_Elements.end());
    }

    vec2 Grid::Diagonal() const
    {
        assert(m_Nx - 1 > 0);
        assert(m_Ny - 1 > 0);
        return {(m_B.x - m_A.x) / (m_Nx - 1), (m_B.y - m_A.y) / (m_Ny - 1)};
    }

    /***********************************************************/
    /********************* CLASS SCALAR_FIELD ******************/

    ScalarField::ScalarField() : Grid()
    {
        m_Diag = Diagonal();
    }

    ScalarField::ScalarField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny) : Grid(heights, a, b, nx, ny)
    {
        assert(nx * ny == heights.size());
        m_Diag = Diagonal();
    }

    Ref<SF> ScalarField::Create(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny)
    {
        return create_ref<SF>(heights, a, b, nx, ny);
    }

    Point ScalarField::Point3D(size_t i, size_t j) const
    {
        return {m_A.x + m_Diag.x * i, m_A.y + m_Diag.y * j, Height(i, j)};
    }

    vec2 ScalarField::Gradient(size_t i, size_t j) const
    {
        assert(i > 0 && i < m_Nx - 1);
        assert(j > 0 && j < m_Ny - 1);

        return {(Height(i + 1, j) - Height(i - 1, j)) * 0.5f, (Height(i, j + 1) - Height(i, j - 1)) * 0.5f};
    }

    vec2 ScalarField::Laplacian(size_t i, size_t j) const
    {
        assert(i > 0 && i < m_Nx - 1);
        assert(j > 0 && j < m_Ny - 1);

        return {(Height(i + 1, j) - 2.f * Height(i, j) + Height(i - 1, j)), (Height(i, j + 1) - 2.f * Height(i, j) + Height(i, j - 1)) * 0.5f};
    }

    float ScalarField::Height(size_t i, size_t j) const
    {
        return (*this).At(i, j);
    }

    float ScalarField::Height(const vec2 &point) const
    {
        float fi = (point.x - m_A.x) / m_Diag.x;
        size_t i = size_t(fi);

        float fj = (point.y - m_A.y) / m_Diag.y;
        size_t j = size_t(fj);

        float u = fi - i;
        float v = fj - j;

        //! Bilinear Interpolation
        return (1 - u) * (1 - v) * Height(i, j) + (1 - u) * v * Height(i, j + 1) + u * (1 - v) * Height(i + 1, j) + u * v * Height(i + 1, j + 1);
    }

    void save_scalar_field(const std::string &filename, const Ref<SF> &sf, size_t nx, size_t ny)
    {
        if (nx < 0)
        {
            nx = sf->Nx();
        }

        if (ny < 0)
        {
            ny = sf->Ny();
        }

        ImageData image(nx, ny, 1);
        float min = std::abs(sf->Min()), max = std::abs(sf->Max());

        float s = 255 / max;
        for (size_t j = 0; j < ny; ++j)
        {
            float njx = j / ny * sf->Ny();
            for (size_t i = 0; i < nx; ++i)
            {
                float nix = i / nx * sf->Nx();
                float h = sf->Height({nix, njx});
                image.pixels[j * nx + i] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (h + min) * s)));
            }
        }

        write_image_data(image, filename.c_str());
    }

    // Point sin_noise(const Point &p)
    // {
    //     return Point();
    // }

    std::vector<float> surface_points(size_t n, const std::function<double(double, double)> &f)
    {
        std::vector<float> points(n * n);
        float step = 1.0 / (n - 1);

        for (size_t i = 0; i < n; ++i)
        {
            float u = i * step;
            for (size_t j = 0; j < n; ++j)
            {
                float v = j * step;
                points.emplace_back(static_cast<float>(f(u, v)));
            }
        }

        return points;
    }

    HeightField::HeightField() : ScalarField()
    {
    }

    HeightField::HeightField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny) : ScalarField(heights, a, b, nx, ny)
    {
    }

    Vector HeightField::Normal(size_t i, size_t j) const
    {
        vec2 grad = -Gradient(i, j);
        return normalize({grad.x, grad.y, 1.f});
    }

    float HeightField::Slope(size_t i, size_t j) const
    {
        return length(Gradient(i, j));
    }
} // namespace mmv
