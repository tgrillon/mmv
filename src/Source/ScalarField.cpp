#include "ScalarField.h"

namespace mmv
{
    /***********************************************************/
    /************************* CLASS GRID **********************/
    Grid::Grid(int nx, int ny, const vec2 &a, const vec2 &b, const std::vector<float> &elements) : m_Nx(nx), m_Ny(ny), m_A(a), m_B(b), m_Elements(elements)
    {
        assert(nx * ny == elements.size());
    }

    float &Grid::operator()(int i, int j)
    {
        return m_Elements[j * m_Nx + i];
    }

    float Grid::At(int i, int j) const
    {
        return m_Elements[j * m_Nx + i];
    }

    float Grid::Nx() const
    {
        return m_Nx;
    }

    float Grid::Ny() const
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

    ScalarField::ScalarField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, int nx, int ny) : Grid(nx, ny, a, b, heights)
    {
        assert(nx * ny == heights.size());
        m_Diag = Diagonal();
    }

    Ref<SF> ScalarField::Create(const std::vector<float> &heights, const vec2 &a, const vec2 &b, int nx, int ny)
    {
        return create_ref<SF>(heights, a, b, nx, ny);
    }

    Point ScalarField::Point(int i, int j) const
    {
        return {m_A.x + m_Diag.x * i, m_A.y + m_Diag.y * j, Height(i, j)};
    }

    vec2 ScalarField::Grad(int i, int j) const
    {
        assert(i > 0 && i < m_Nx - 1);
        assert(j > 0 && j < m_Ny - 1);

        return {(Height(i + 1, j) - Height(i - 1, j)) * 0.5, (Height(i, j + 1) - Height(i, j - 1)) * 0.5};
    }

    float ScalarField::Height(int i, int j) const
    {
        return (*this).At(i, j);
    }

    float ScalarField::Height(const vec2 &point) const
    {
        float fi = (point.x - m_A.x) / m_Diag.x;
        int i = int(fi);

        float fj = (point.y - m_A.y) / m_Diag.y;
        int j = int(fj);

        float u = fi - i;
        float v = fj - j;

        //! Bilinear Interpolation
        return (1 - u) * (1 - v) * Height(i, j) + (1 - u) * v * Height(i, j + 1) + u * (1 - v) * Height(i + 1, j) + u * v * Height(i + 1, j + 1);
    }

    void save_scalar_field(const std::string &filename, const Ref<SF> &sf, int nx, int ny)
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
        for (int j = 0; j < ny; ++j)
        {
            float njx = j / ny * sf->Ny();
            for (int i = 0; i < nx; ++i)
            {
                float nix = i / nx * sf->Nx();
                float h = sf->Height({nix, njx});
                image.pixels[j * nx + i] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (h + min) * s)));
            }
        }

        write_image_data(image, filename.c_str());
    }
} // namespace mmv
