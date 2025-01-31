#include "HeightField.h"

#include "gkitext.h"
#include "vecext.h"
#include "Utils.h"

namespace mmv
{
    /***********************************************************/
    /********************* CLASS SCALAR_FIELD ******************/

    ScalarField::ScalarField() : Array2()
    {
        m_Diag = Diagonal();
    }

    ScalarField::ScalarField(int dim) : Array2(dim)
    {
    }

    ScalarField::ScalarField(int nx, int ny) : Array2(nx, ny)
    {
    }

    ScalarField::ScalarField(const std::vector<scalar_t> &elevations, int nx, int ny) : Array2(elevations, nx, ny)
    {
    }

    ScalarField::ScalarField(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny) : Array2(elevations, a, b, nx, ny)
    {
        assert(nx * ny == elevations.size());
        m_Diag = Diagonal();
    }

    Ref<SF> ScalarField::Create(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny)
    {
        return create_ref<SF>(elevations, a, b, nx, ny);
    }

    void ScalarField::Elevations(const std::vector<scalar_t> &elevations, int nx, int ny)
    {
        m_Elements.clear();
        m_Elements = elevations;

        if (nx > 0)
            m_Nx = nx;
        if (ny > 0)
            m_Ny = ny;
    }

    Point ScalarField::Point3D(index_t i, index_t j) const
    {
        return {m_A.x + m_Diag.x * i, m_A.y + m_Diag.y * j, Height(i, j)};
    }

    vec2 ScalarField::Gradient(index_t i, index_t j) const
    {
        scalar_t grad_x = 0.f;
        if (i == 0)
            grad_x = (Height(i + 1, j) - Height(i, j)) * 0.5f;
        else if (i == m_Nx - 1)
            grad_x = (Height(i, j) - Height(i - 1, j)) * 0.5f;
        else
            grad_x = (Height(i + 1, j) - Height(i - 1, j)) * 0.5f;

        scalar_t grad_y = 0.f;
        if (j == 0)
            grad_y = (Height(i, j + 1) - Height(i, j)) * 0.5f;
        else if (j == m_Ny - 1)
            grad_y = (Height(i, j) - Height(i, j - 1)) * 0.5f;
        else
            grad_y = (Height(i, j + 1) - Height(i, j - 1)) * 0.5f;

        return {grad_x, grad_y};
    }

    vec2 ScalarField::Gradient(scalar_t x, scalar_t y) const
    {
        scalar_t grad_x = 0.f;
        if (x < 1.f)
            grad_x = (Height(x + 1, y) - Height(x, y)) * 0.5f;
        else if (x > scalar_t(m_Nx - 2))
            grad_x = (Height(x, y) - Height(x - 1, y)) * 0.5f;
        else
            grad_x = (Height(x + 1, y) - Height(x - 1, y)) * 0.5f;

        scalar_t grad_y = 0.f;
        if (y < 1.f)
            grad_y = (Height(x, y + 1) - Height(x, y)) * 0.5f;
        else if (y > scalar_t(m_Ny - 2))
            grad_y = (Height(x, y) - Height(x, y - 1)) * 0.5f;
        else
            grad_y = (Height(x, y + 1) - Height(x, y - 1)) * 0.5f;

        return {grad_x, grad_y};
    }

    scalar_t ScalarField::Laplacian(index_t i, index_t j) const
    {
        scalar_t laplacian_x = 0.f;
        if (i == 0)
            laplacian_x = (Height(i + 2, j) - 2.f * Height(i + 1, j) + Height(i, j));
        else if (i == m_Nx - 1)
            laplacian_x = (Height(i, j) - 2.f * Height(i - 1, j) + Height(i - 2, j));
        else
            laplacian_x = (Height(i + 1, j) - 2.f * Height(i, j) + Height(i - 1, j));

        scalar_t laplacian_y = 0.f;
        if (j == 0)
            laplacian_y = (Height(i, j + 2) - 2.f * Height(i, j + 1) + Height(i, j));
        else if (j == m_Ny - 1)
            laplacian_y = (Height(i, j) - 2.f * Height(i, j - 1) + Height(i, j - 2));
        else
            laplacian_y = (Height(i, j + 1) - 2.f * Height(i, j) + Height(i, j - 1));

        return laplacian_x + laplacian_y;
    }

    scalar_t ScalarField::Laplacian(scalar_t x, scalar_t y) const
    {
        scalar_t laplacian_x = 0.f;
        if (x < 1.f)
            laplacian_x = (Height(x + 2, y) - 2.f * Height(x + 1, y) + Height(x, y));
        else if (x > scalar_t(m_Nx - 2))
            laplacian_x = (Height(x, y) - 2.f * Height(x - 1, y) + Height(x - 2, y));
        else
            laplacian_x = (Height(x + 1, y) - 2.f * Height(x, y) + Height(x - 1, y));

        scalar_t laplacian_y = 0.f;
        if (y < 1.f)
            laplacian_y = (Height(x, y + 2) - 2.f * Height(x, y + 1) + Height(x, y));
        else if (y > scalar_t(m_Ny - 2))
            laplacian_y = (Height(x, y) - 2.f * Height(x, y - 1) + Height(x, y - 2));
        else
            laplacian_y = (Height(x, y + 1) - 2.f * Height(x, y) + Height(x, y - 1));

        return laplacian_x + laplacian_y;
    }

    int ScalarField::ExportElevation(const std::string &filename, int nx, int ny)
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        UpdateMinMax();

        ImageData image(nx, ny, 3);

        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                // scalar_t h = Height({u, v});
                auto value = static_cast<pixel_t>(Normalize(u, v) * 255);
                image.pixels[(j * nx + i) * 3 + 0] = value;
                image.pixels[(j * nx + i) * 3 + 1] = value;
                image.pixels[(j * nx + i) * 3 + 2] = value;
            }
        }

        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[Height] Image ", filename, " successfully saved in ./data/output");
#endif

        return 0;
    }

    int ScalarField::ExportGradient(const std::string &filename, int nx, int ny)
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        vec2 min{1000.f, 1000.f}, max{-1000.f, -1000.f};
        std::vector<vec2> grads;
        grads.reserve(nx * ny);
        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                vec2 grad = Gradient(u, v);
                if (grad.x < 0.f)
                    grad.x = -std::sqrt(-grad.x);
                else 
                    grad.x = std::sqrt(grad.x);
                
                if (grad.y < 0.f)
                    grad.y = -std::sqrt(-grad.y);
                else
                    grad.y = std::sqrt(grad.y);

                grads.emplace_back(grad.x, grad.y);
                min.x = std::min(min.x, grad.x);
                min.y = std::min(min.y, grad.y);
                max.x = std::max(max.x, grad.x);
                max.y = std::max(max.y, grad.y);
            }
        }

        ImageData image(nx, ny, 3);
        for (int j = 0; j < ny; ++j)
        {
            for (int i = 0; i < nx; ++i)
            {
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<pixel_t>((grads[j * nx + i].x - min.x) * 255.f / (max.x - min.x));
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<pixel_t>((grads[j * nx + i].y - min.y) * 255.f / (max.y - min.y));
                image.pixels[(j * nx + i) * 3 + 2] = 0;
            }
        }

        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[Gradient] Image ", filename, " successfully saved in ./data/output");
#endif

        return 0;
    }

    int ScalarField::ExportLaplacian(const std::string &filename, int nx, int ny)
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        Array2 laplacians(nx, ny);
        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;

                scalar_t laplacian = Laplacian(u, v);
                if (laplacian < 0.f)
                    laplacian = -std::sqrt(-laplacian);
                else
                    laplacian = std::sqrt(laplacian);
                laplacians(i, j) = laplacian;
            }
        }

        laplacians.UpdateMinMax();

        ExportGrayscaleImage(filename, nx, ny, laplacians);

        return 0;
    }

    int ScalarField::ExportElevationAsTxt(const std::string &filename, int nx, int ny)
    {
        if (std::string(filename).rfind(".txt") == std::string::npos)
        {
            utils::error("writing txt file '", filename, "'... not a .txt file.\n");
            return -1;
        }

        scalar_t nxf = scalar_t(nx < 0 ? m_Nx : nx);
        scalar_t nzf = scalar_t(ny < 0 ? m_Ny : ny);

        std::ofstream file(std::string(DATA_DIR) + "/output/" + filename);

        file << nx * ny << '\n';

        scalar_t min = std::abs(Min()), max = std::abs(Max());

        scalar_t s = 255.f / max;
        for (int j = 0; j < ny; ++j)
        {
            scalar_t njz = j / nzf * m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t nix = i / nxf * m_Nx;
                scalar_t h = Height({nix, njz});
                scalar_t y = static_cast<pixel_t>(std::max(0.f, std::min(255.f, (h + min) * s)));
                file << (scalar_t)i << ' ' << (scalar_t)j << ' ' << y << '\n';
            }
        }

        file.close();

#ifndef NDEBUG
        utils::status("File ", filename, " successfully saved in ./data/output");
#endif

        return 0;
    }

    int ScalarField::ExportGrayscaleImage(const std::string &filename, const int nx, const int ny, const Array2 &values) const
    {
        ImageData image(nx, ny, 3);

        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                pixel_t value = static_cast<pixel_t>(values.Normalize(i, j) * 255.f);
                image.pixels[(j * nx + i) * 3 + 0] = value;
                image.pixels[(j * nx + i) * 3 + 1] = value;
                image.pixels[(j * nx + i) * 3 + 2] = value;
            }
        }

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;
        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[ExportGrayscaleImage] Image ", filename, " successfully saved in ./data/output");
#endif

        return 0;
    }

    scalar_t ScalarField::Height(index_t i, index_t j) const
    {
        if (i >= m_Nx || j >= m_Ny)
            return 0.f;
        return (*this).At(i, j);
    }

    scalar_t ScalarField::Height(scalar_t x, scalar_t y) const
    {
        scalar_t fi = (x - m_A.x) / m_Diag.x;
        int i = int(fi);

        scalar_t fj = (y - m_A.y) / m_Diag.y;
        int j = int(fj);

        scalar_t u = fi - i;
        scalar_t v = fj - j;

        //! Bilinear Interpolation
        return (1 - u) * (1 - v) * Height(i, j) + (1 - u) * v * Height(i, j + 1) + u * (1 - v) * Height(i + 1, j) + u * v * Height(i + 1, j + 1);
    }

    scalar_t ScalarField::Height(const vec2 &point) const
    {
        return Height(point.x, point.y);
    }

    HeightField::HeightField() : ScalarField()
    {
    }

    HeightField::HeightField(int dim) : ScalarField(dim)
    {
    }

    HeightField::HeightField(int nx, int ny) : ScalarField(nx, ny)
    {
    }

    HeightField::HeightField(const std::vector<scalar_t> &elevations, int nx, int ny) : ScalarField(elevations, nx, ny)
    {
    }

    HeightField::HeightField(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny) : ScalarField(elevations, a, b, nx, ny)
    {
    }

    Ref<HeightField> HeightField::Create(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny)
    {
        return create_ref<HF>(elevations, a, b, nx, ny);
    }

    Mesh HeightField::Polygonize(int n) const
    {
        Mesh mesh(GL_TRIANGLES);

        scalar_t step = 1.f / scalar_t(n - 1);
        for (int j = 0; j < n; ++j)
        {
            scalar_t v = j * step * m_Ny;
            for (int i = 0; i < n; ++i)
            {
                scalar_t u = i * step * m_Nx;

                mesh.normal(Normal(u, v));
                mesh.texcoord({u / (scalar_t)m_Nx, v / (scalar_t)m_Ny});
                mesh.vertex(u, Height(u, v), v);

                if (i > 0 && j > 0)
                {
                    int prev_i = i - 1;
                    int prev_j = j - 1;

                    mesh.triangle(prev_j * n + prev_i, j * n + prev_i, j * n + i);
                    mesh.triangle(prev_j * n + prev_i, j * n + i, prev_j * n + i);
                }
            }
        }

        return mesh;
    }

    int HeightField::ExportNormal(const std::string &filename, int nx, int ny) const
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(nx, ny, 3);

        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                vec3 normal = Normal(u, v);
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<pixel_t>(std::max(0.f, std::min(255.f, (normal.x + 0.5f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<pixel_t>(std::max(0.f, std::min(255.f, (normal.z + 0.5f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 2] = static_cast<pixel_t>(std::max(0.f, std::min(255.f, (normal.y + 0.5f) * 0.5f * 255.f)));
            }
        }

        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[Normal] Image ", filename, " successfully saved in ./data/output");
#endif

        return 0;
    }

    int HeightField::ExportSlope(const std::string &filename, int nx, int ny) const
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        Array2 slope(nx, ny);

        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                slope(i, j) = std::log2f(Slope(u, v));
            }
        }

        slope.UpdateMinMax();

        ExportGrayscaleImage(filename, nx, ny, slope);

        return 0;
    }

    int HeightField::ExportAverageSlope(const std::string &filename, int nx, int ny) const
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        Array2 avgslope(nx, ny);
        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                avgslope(i, j) = std::log2f(AverageSlope(u, v));
            }
        }

        avgslope.UpdateMinMax();

        ExportGrayscaleImage(filename, nx, ny, avgslope);

        return 0;
    }

    int HeightField::ExportShading(const std::string &filename, const Vector &light_direction, int nx, int ny) const
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(nx, ny, 3);

        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                Vector normal = Normal(u, v);
                scalar_t cos_theta = std::max(0.f, dot(normalize(-light_direction), normal));
                pixel_t value = static_cast<pixel_t>(cos_theta * 255.f);
                image.pixels[(j * nx + i) * 3 + 0] = value;
                image.pixels[(j * nx + i) * 3 + 1] = value;
                image.pixels[(j * nx + i) * 3 + 2] = value;
            }
        }

        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[Shading] Image ", filename, " successfully saved in ./data/output");
#endif
        return 0;
    }

    int HeightField::ExportGlobalShading(const std::string &filename, int ppp, int nx, int ny) const
    {
        nx = nx < 0 ? m_Nx : nx;
        ny = ny < 0 ? m_Ny : ny;

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        std::random_device hwseed;
        std::default_random_engine rng(hwseed());
        std::uniform_real_distribution<scalar_t> uniform(0, 1);

        Array2 shades(nx, ny);
        for (int j = 0; j < ny; ++j)
        {
            scalar_t v = (scalar_t)j / (scalar_t)ny * (scalar_t)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                scalar_t cos_theta = 0.;
                for (int k = 0; k < ppp; ++k)
                {
                    scalar_t u = (scalar_t)i / (scalar_t)nx * (scalar_t)m_Nx;
                    Vector normal = Normal(u, v);
                    
                    scalar_t u1 = uniform(rng);
                    scalar_t u2 = uniform(rng);
                    Vector l = sample34(u1, u2);

                    cos_theta += std::max(0.f, dot(normalize(l), normal));
                }

                shades(i, j) = cos_theta / ppp;
            }
        }

        shades.Gauss();
        shades.UpdateMinMax();

        ExportGrayscaleImage(filename, nx, ny, shades);

        return 0;
    }

    int HeightField::ExportObj(const std::string &filename, int resolution)
    {
        return write_mesh(Polygonize(resolution), filename.c_str());
    }

    bool comp(scalar_t a, scalar_t b) { return a > b; }

    /**
     * |v00|v10|v20|---|
     * |v01|idx|v21|---|
     * |v02|v12|v22|---|
     */
    int HeightField::ExportStreamArea(const std::string &filename) const
    {
        Array2 A = StreamArea();
        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        ImageData image(m_Nx, m_Ny, 3);

        for (int j = 0; j < A.Ny(); ++j)
        {
            for (int i = 0; i < A.Nx(); ++i)
            {
                A(i, j) = std::log2f(A(i, j));
            }
        }

        A.UpdateMinMax();

        for (int j = 0; j < m_Ny; ++j)
        {
            for (int i = 0; i < m_Nx; ++i)
            {
                pixel_t value = static_cast<pixel_t>(A.Normalize(i, j) * 128.f); 
                image.pixels[(j * m_Nx + i) * 3 + 0] = std::min(99 + value, 255);
                image.pixels[(j * m_Nx + i) * 3 + 1] = std::min(132 + value, 255);
                image.pixels[(j * m_Nx + i) * 3 + 2] = std::min(235 + value, 255);
            }
        }

        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[Stream area] Image ", filename, " successfully saved in ./data/output");
#endif
        return 0;
    }

    Array2<scalar_t> HeightField::StreamArea() const
    {
        //! On trie les hauteurs dans l'ordre décroissant et on les stocke dans une queue
        std::priority_queue<std::pair<scalar_t, int>> Q;
        for (int i = 0; auto e : m_Elements)
        {
            Q.emplace(e, i);
            i++;
        }

        Array2 A(m_Nx, m_Ny, 1.f);
        while (!Q.empty())
        {
            auto [elv, idx] = Q.top();
            Q.pop();

            int I = idx % m_Nx;
            int J = (idx - I) / m_Nx;

            vec2 grad = -Gradient(I, J);
            scalar_t u = int(std::round(grad.x));
            scalar_t v = int(std::round(grad.y));

            if (I + u >= 0 && I + u < m_Nx && J + v >= 0 && J + v < m_Ny)
            {
                scalar_t d = sqrt(abs(u) + abs(v));
                if (d != 0.f)
                    A(I + u, J + v) += A(I, J) / d;
            }
        }

        return A;
    }

    void HeightField::StreamPower()
    {
        const Array2 &A = StreamArea();

        scalar_t k = 1e-1;
        for (int j = 0; j < m_Ny; ++j)
        {
            for (int i = 0; i < m_Nx; ++i)
            {
                operator()(i, j) -= k * std::pow(A.At(i, j), 0.5) * Slope(i, j);
            }
        }
    }

    Vector HeightField::Normal(index_t i, index_t j) const
    {
        vec2 grad = Gradient(i, j);
        return normalize({-grad.x, 1.f, -grad.y});
    }

    Vector HeightField::Normal(scalar_t x, scalar_t y) const
    {
        vec2 grad = Gradient(x, y);
        return normalize({-grad.x, 1.f, -grad.y});
    }

    scalar_t HeightField::Slope(index_t i, index_t j) const
    {
        return length(Gradient(i, j));
    }

    scalar_t HeightField::Slope(scalar_t x, scalar_t y) const
    {
        return length(Gradient(x, y));
    }

    scalar_t HeightField::AverageSlope(index_t i, index_t j) const
    {
        assert(i >= 0.f && i <= m_Nx);
        assert(j >= 0.f && j <= m_Ny);

        int count = 1;
        scalar_t sum_slope = Slope(i, j);
        if (i - 1 >= 0 && j - 1 >= 0)
        {
            sum_slope += Slope(i - 1, j - 1);
            count++;
        }

        if (i + 1 < m_Nx && j - 1 >= 0)
        {
            sum_slope += Slope(i + 1, j - 1);
            count++;
        }

        if (i + 1 < m_Nx && j + 1 < m_Ny)
        {
            sum_slope += Slope(i + 1, j + 1);
            count++;
        }

        if (i - 1 >= 0 && j + 1 < m_Ny)
        {
            sum_slope += Slope(i - 1, j + 1);
            count++;
        }

        if (i - 1 >= 0)
        {
            sum_slope += Slope(i - 1, j);
            count++;
        }

        if (i + 1 < (scalar_t)m_Nx)
        {
            sum_slope += Slope(i + 1, j);
            count++;
        }

        if (j - 1 >= 0)
        {
            sum_slope += Slope(i, j - 1);
            count++;
        }

        if (j + 1 < (scalar_t)m_Ny)
        {
            sum_slope += Slope(i, j + 1);
            count++;
        }

        return sum_slope / (scalar_t)count;
    }

    scalar_t HeightField::AverageSlope(scalar_t x, scalar_t y) const
    {
        assert(x >= 0.f && x <= (scalar_t)m_Nx);
        assert(y >= 0.f && y <= (scalar_t)m_Ny);

        int count = 1;
        scalar_t sum_slope = Slope(x, y);
        if (x - 1 >= 0 && y - 1 >= 0)
        {
            sum_slope += Slope(x - 1, y - 1);
            count++;
        }

        if (x + 1 < m_Nx && y - 1 >= 0)
        {
            sum_slope += Slope(x + 1, y - 1);
            count++;
        }

        if (x + 1 < m_Nx && y + 1 < m_Ny)
        {
            sum_slope += Slope(x + 1, y + 1);
            count++;
        }

        if (x - 1 >= 0 && y + 1 < m_Ny)
        {
            sum_slope += Slope(x - 1, y + 1);
            count++;
        }

        if (x - 1 >= 0)
        {
            sum_slope += Slope(x - 1, y);
            count++;
        }

        if (x + 1 < (scalar_t)m_Nx)
        {
            sum_slope += Slope(x + 1, y);
            count++;
        }

        if (y - 1 >= 0)
        {
            sum_slope += Slope(x, y - 1);
            count++;
        }

        if (y + 1 < (scalar_t)m_Ny)
        {
            sum_slope += Slope(x, y + 1);
            count++;
        }

        return sum_slope / (scalar_t)count;
    }

    std::vector<scalar_t> load_elevation(const std::string &map)
    {
        const std::string FULLPATH = std::string(DATA_DIR) + "/input/" + map;

        ImageData image = read_image_data(FULLPATH.c_str());

        std::vector<scalar_t> elevation(image.width * image.height);

        for (size_t i = 0; i < image.width * image.height; ++i)
            elevation[i] = image.pixels[i * 3];

        return elevation;
    }

    Vector sample34(const float u1, const float u2)
    {
        float cos_theta = u1;
        float sin_theta = sqrt(1 - cos_theta * cos_theta);
        float phi = float(2 * M_PI) * u2;
        float sin_phi = sin(phi);
        float cos_phi = cos(phi);

        return {cos_phi * sin_theta, cos_theta, sin_theta * sin_phi};
    }

    float pdf34()
    {
        return 1.f / float(2 * M_PI);
    }
} // namespace mmv
