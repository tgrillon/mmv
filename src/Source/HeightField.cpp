#include "HeightField.h"

#include "vecext.h"
#include "Utils.h"

namespace mmv
{
    /***********************************************************/
    /************************* CLASS GRID **********************/
    Grid::Grid(const std::vector<float> &elements, const vec2 &a, const vec2 &b, int nx, int nz) : m_Nx(nx), m_Nz(nz), m_A(a), m_B(b), m_Elements(elements)
    {
        assert(nx * nz == elements.size());
    }

    float &Grid::operator()(int i, int j)
    {
        return m_Elements[j * m_Nx + i];
    }

    float Grid::At(int i, int j) const
    {
        assert(i >= 0 && i < m_Nx);
        assert(j >= 0 && j < m_Nz);

        return m_Elements[j * m_Nx + i];
    }

    int Grid::Nx() const
    {
        return m_Nx;
    }

    int Grid::Nz() const
    {
        return m_Nz;
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
        assert(m_Nz - 1 > 0);
        return {(m_B.x - m_A.x) / (m_Nx - 1), (m_B.y - m_A.y) / (m_Nz - 1)};
    }

    /***********************************************************/
    /********************* CLASS SCALAR_FIELD ******************/

    ScalarField::ScalarField() : Grid()
    {
        m_Diag = Diagonal();
    }

    ScalarField::ScalarField(const std::vector<float> &elevations, const vec2 &a, const vec2 &b, int nx, int nz) : Grid(elevations, a, b, nx, nz)
    {
        assert(nx * nz == elevations.size());
        m_Diag = Diagonal();
    }

    Ref<SF> ScalarField::Create(const std::vector<float> &elevations, const vec2 &a, const vec2 &b, int nx, int nz)
    {
        return create_ref<SF>(elevations, a, b, nx, nz);
    }

    void ScalarField::Elevations(const std::vector<float> &elevations, int nx, int nz)
    {
        m_Elements.clear();
        m_Elements = elevations;

        if (nx > 0)
            m_Nx = nx;
        if (nz > 0)
            m_Nz = nz;
    }

    Point ScalarField::Point3D(int i, int j) const
    {
        return {m_A.x + m_Diag.x * i, m_A.y + m_Diag.y * j, Height(i, j)};
    }

    vec2 ScalarField::Gradient(int i, int j) const
    {
        float grad_x = 0.f;
        if (i == 0)
            grad_x = (Height(i + 1, j) - Height(i, j)) * 0.5f;
        else if (i == m_Nx - 1)
            grad_x = (Height(i, j) - Height(i - 1, j)) * 0.5f;
        else
            grad_x = (Height(i + 1, j) - Height(i - 1, j)) * 0.5f;

        float grad_y = 0.f;
        if (j == 0)
            grad_y = (Height(i, j + 1) - Height(i, j)) * 0.5f;
        else if (j == m_Nx - 1)
            grad_y = (Height(i, j) - Height(i, j - 1)) * 0.5f;
        else
            grad_y = (Height(i, j + 1) - Height(i, j - 1)) * 0.5f;

        return {grad_x, grad_y};
    }

    vec2 ScalarField::Gradient(float x, float z) const
    {
        float grad_x = 0.f;
        if (x == 0)
            grad_x = (Height(x + 1, z) - Height(x, z)) * 0.5f;
        else if (x == m_Nx - 1)
            grad_x = (Height(x, z) - Height(x - 1, z)) * 0.5f;
        else
            grad_x = (Height(x + 1, z) - Height(x - 1, z)) * 0.5f;

        float grad_y = 0.f;
        if (z == 0)
            grad_y = (Height(x, z + 1) - Height(x, z)) * 0.5f;
        else if (z == m_Nx - 1)
            grad_y = (Height(x, z) - Height(x, z - 1)) * 0.5f;
        else
            grad_y = (Height(x, z + 1) - Height(x, z - 1)) * 0.5f;

        return {grad_x, grad_y};
    }

    vec2 ScalarField::Laplacian(int i, int j) const
    {
        assert(i > 0 && i < m_Nx - 1);
        assert(j > 0 && j < m_Nz - 1);

        return {(Height(i + 1, j) - 2.f * Height(i, j) + Height(i - 1, j)), (Height(i, j + 1) - 2.f * Height(i, j) + Height(i, j - 1)) * 0.5f};
    }

    int ScalarField::SaveHeightAsImage(const std::string &filename, int nx, int nz)
    {
        nx = nx < 0 ? m_Nx : nx;
        nz = nz < 0 ? m_Nz : nz;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(nx, nz, 3);

        for (int j = 0; j < nz; ++j)
        {
            float v = (float)j / (float)nz * (float)m_Nz;
            for (int i = 0; i < nx; ++i)
            {
                float u = (float)i / (float)nx * (float)m_Nx;
                float h = Height({u, v});
                auto value = static_cast<unsigned char>(h * 255.f);
                image.pixels[(j * nx + i) * 3 + 0] = value;
                image.pixels[(j * nx + i) * 3 + 1] = value;
                image.pixels[(j * nx + i) * 3 + 2] = value;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("Image ", filename, " successfully saved in ./data/output");

        return 0;
    }

    int ScalarField::SaveGradientAsImage(const std::string &filename, int nx, int nz)
    {
        nx = nx < 0 ? m_Nx : nx;
        nz = nz < 0 ? m_Nz : nz;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(nx, nz, 3);

        for (int j = 0; j < nz; ++j)
        {
            float v = (float)j / (float)nz * (float)m_Nz;
            for (int i = 0; i < nx; ++i)
            {
                float u = (float)i / (float)nx * (float)m_Nx;
                vec2 grad = Gradient(u, v);
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (grad.x + 1.f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (grad.y + 1.f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 2] = 0;
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[SaveGradientAsImage] Image ", filename, " successfully saved in ./data/output");

        return 0;
    }

    int ScalarField::SaveHeightAsTxt(const std::string &filename, int nx, int nz)
    {
        if (std::string(filename).rfind(".txt") == std::string::npos)
        {
            utils::error("writing color image '", filename, "'... not a .png / .bmp image.\n");
            return -1;
        }

        float nxf = float(nx < 0 ? m_Nx : nx);
        float nzf = float(nz < 0 ? m_Nz : nz);

        std::ofstream file(std::string(DATA_DIR) + "/output/" + filename);

        file << nx * nz << '\n';

        float min = std::abs(Min()), max = std::abs(Max());

        float s = 255.f / max;
        for (int j = 0; j < nz; ++j)
        {
            float njz = j / nzf * m_Nz;
            for (int i = 0; i < nx; ++i)
            {
                float nix = i / nxf * m_Nx;
                float h = Height({nix, njz});
                float y = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (h + min) * s)));
                file << (float)i << ' ' << (float)j << ' ' << y << '\n';
            }
        }

        file.close();
        utils::status("File ", filename, " successfully saved in ./data/output");

        return 0;
    }

    float ScalarField::Height(int i, int j) const
    {
        if (i >= m_Nx || j >= m_Nz)
            return 0.f;
        return (*this).At(i, j);
    }

    float ScalarField::Height(float x, float z) const
    {
        float fi = (x - m_A.x) / m_Diag.x;
        int i = int(fi);

        float fj = (z - m_A.y) / m_Diag.y;
        int j = int(fj);

        float u = fi - i;
        float v = fj - j;

        //! Bilinear Interpolation
        return (1 - u) * (1 - v) * Height(i, j) + (1 - u) * v * Height(i, j + 1) + u * (1 - v) * Height(i + 1, j) + u * v * Height(i + 1, j + 1);
    }

    float ScalarField::Height(const vec2 &point) const
    {
        return Height(point.x, point.y);
    }

    HeightField::HeightField() : ScalarField()
    {
    }

    HeightField::HeightField(const std::vector<float> &elevations, const vec2 &a, const vec2 &b, int nx, int nz) : ScalarField(elevations, a, b, nx, nz)
    {
    }

    Ref<HeightField> HeightField::Create(const std::vector<float> &elevations, const vec2 &a, const vec2 &b, int nx, int nz)
    {
        return create_ref<HF>(elevations, a, b, nx, nz);
    }

    Mesh HeightField::Polygonize(int n) const
    {
        Mesh mesh(GL_TRIANGLE_STRIP);

        float step = 1.f / float(n);
        for (int j = 0; j < n; ++j)
        {
            float v = j * step * m_Nz;
            for (int i = 0; i < n; ++i)
            {
                float u = i * step * m_Nx;
                mesh.normal(Normal(u, v));
                mesh.texcoord({u / (float)m_Nx, v / (float)m_Nz});
                mesh.vertex(u, Height(u, v), v);

                mesh.normal(Normal(u, v));
                mesh.texcoord({u / (float)m_Nx, (v + 1) / (float)m_Nz});
                mesh.vertex(u, Height(u, v + 1), v + 1);
            }
            mesh.restart_strip();
        }

        return mesh;
    }

    int HeightField::SaveNormalAsImage(const std::string &filename, int nx, int nz) const
    {
        nx = nx < 0 ? m_Nx : nx;
        nz = nz < 0 ? m_Nz : nz;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(nx, nz, 3);

        for (int j = 0; j < nz; ++j)
        {
            float v = (float)j / (float)nz * (float)m_Nz;
            for (int i = 0; i < nx; ++i)
            {
                float u = (float)i / (float)nx * (float)m_Nx;
                vec3 normal = Normal(u, v);
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (normal.x + 1.f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 2] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (normal.y + 1.f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, (normal.z + 1.f) * 0.5f * 255.f)));
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[Normal] Image ", filename, " successfully saved in ./data/output");

        return 0;
    }

    int HeightField::SaveSlopeAsImage(const std::string &filename, int nx, int nz) const
    {
        nx = nx < 0 ? m_Nx : nx;
        nz = nz < 0 ? m_Nz : nz;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(nx, nz, 3);

        for (int j = 0; j < nz; ++j)
        {
            float v = (float)j / (float)nz * (float)m_Nz;
            for (int i = 0; i < nx; ++i)
            {
                float u = (float)i / (float)nx * (float)m_Nx;
                float slope = Slope(u, v);
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, slope * 255.f)));
                image.pixels[(j * nx + i) * 3 + 2] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, slope * 255.f)));
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<unsigned char>(std::max(0.f, std::min(255.f, slope * 255.f)));
            }
        }

        write_image_data(image, fullpath.c_str());
        utils::status("[Slope] Image ", filename, " successfully saved in ./data/output");

        return 0;
    }

    Vector HeightField::Normal(int i, int j) const
    {
        vec2 grad = Gradient(i, j);
        return normalize({-grad.x, 1.f, -grad.y});
    }

    float HeightField::Slope(int i, int j) const
    {
        return length(Gradient(i, j));
    }
} // namespace mmv
