#include "HeightField.h"

#include "vecext.h"
#include "Utils.h"

namespace mmv
{
    /***********************************************************/
    /************************* CLASS GRID **********************/
    Array2::Array2(const std::vector<ScalarType> &elements, const vec2 &a, const vec2 &b, int nx, int ny) : Array2(elements, nx, ny)
    {
        m_A = a; m_B = b;
    }

    Array2::Array2(int nx, int ny, ScalarType v) : m_Nx(nx), m_Ny(ny), m_A(0, 0), m_B(nx, ny), m_Elements(nx * ny, v), m_Min(v), m_Max(v)
    {
    }

    Array2::Array2(int dim) : Array2(dim, dim)
    {
    }

    Array2::Array2(const std::vector<ScalarType> &elements, int nx, int ny) : m_Nx(nx), m_Ny(ny), m_A(0, 0), m_B(nx, ny), m_Elements(elements)
    {
        assert(nx * ny == elements.size());
        m_Min = *std::min_element(m_Elements.begin(), m_Elements.end());
        m_Max = *std::max_element(m_Elements.begin(), m_Elements.end());
    }

    Array2::Array2(const vec2 &a, const vec2 &b, int nx, int ny, ScalarType v) : Array2(nx, ny, v)
    {
        m_A = a; m_B = b;
    }

    ScalarType &Array2::operator()(IndexType i, IndexType j)
    {
        assert(j * m_Nx + i < m_Nx * m_Ny);
        return m_Elements[j * m_Nx + i];
    }

    ScalarType &Array2::operator()(IndexType i)
    {
        assert(i < m_Nx * m_Ny);
        return m_Elements[i];
    }

    ScalarType Array2::At(IndexType i, IndexType j) const
    {
        assert(i >= 0 && i < m_Nx);
        assert(j >= 0 && j < m_Ny);

        return m_Elements[j * m_Nx + i];
    }

    ScalarType Array2::At(IndexType i) const
    {
        assert(i < m_Nx * m_Ny);
        return m_Elements[i];
    }

    ScalarType Array2::Normalize(IndexType i, IndexType j) const
    {
        return (m_Elements[j * m_Nx + i] - m_Min) / (m_Max - m_Min);
    }

    ScalarType Array2::Clamp(IndexType i, IndexType j, ScalarType l, ScalarType h) const
    {
        return std::min(h, std::max(l, m_Elements[j * m_Nx + i]));
    }

    int Array2::Nx() const
    {
        return m_Nx;
    }

    int Array2::Ny() const
    {
        return m_Ny;
    }

    vec2 Array2::A() const
    {
        return m_A;
    }

    vec2 Array2::B() const
    {
        return m_B;
    }

    ScalarType Array2::Min() const
    {
        return m_Min;
    }

    ScalarType Array2::Max() const
    {
        return *std::max_element(m_Elements.begin(), m_Elements.end());
    }

    void Array2::UpdateMinMax()
    {
        m_Min = *std::min_element(m_Elements.begin(), m_Elements.end());
        m_Max = *std::max_element(m_Elements.begin(), m_Elements.end());
    }

    vec2 Array2::Diagonal() const
    {
        assert(m_Nx - 1 > 0);
        assert(m_Ny - 1 > 0);
        return {(m_B.x - m_A.x) / (m_Nx - 1), (m_B.y - m_A.y) / (m_Ny - 1)};
    }

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

    ScalarField::ScalarField(const std::vector<ScalarType> &elevations, int nx, int ny) : Array2(elevations, nx, ny)
    {
    }

    ScalarField::ScalarField(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny) : Array2(elevations, a, b, nx, ny)
    {
        assert(nx * ny == elevations.size());
        m_Diag = Diagonal();
    }

    Ref<SF> ScalarField::Create(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny)
    {
        return create_ref<SF>(elevations, a, b, nx, ny);
    }

    void ScalarField::Elevations(const std::vector<ScalarType> &elevations, int nx, int ny)
    {
        m_Elements.clear();
        m_Elements = elevations;

        if (nx > 0)
            m_Nx = nx;
        if (ny > 0)
            m_Ny = ny;
    }

    Point ScalarField::Point3D(IndexType i, IndexType j) const
    {
        return {m_A.x + m_Diag.x * i, m_A.y + m_Diag.y * j, Height(i, j)};
    }

    vec2 ScalarField::Gradient(IndexType i, IndexType j) const
    {
        ScalarType grad_x = 0.f;
        if (i == 0)
            grad_x = (Height(i + 1, j) - Height(i, j)) * 0.5f;
        else if (i == m_Nx - 1)
            grad_x = (Height(i, j) - Height(i - 1, j)) * 0.5f;
        else
            grad_x = (Height(i + 1, j) - Height(i - 1, j)) * 0.5f;

        ScalarType grad_y = 0.f;
        if (j == 0)
            grad_y = (Height(i, j + 1) - Height(i, j)) * 0.5f;
        else if (j == m_Ny - 1)
            grad_y = (Height(i, j) - Height(i, j - 1)) * 0.5f;
        else
            grad_y = (Height(i, j + 1) - Height(i, j - 1)) * 0.5f;

        return {grad_x, grad_y};
    }

    vec2 ScalarField::Gradient(ScalarType x, ScalarType y) const
    {
        ScalarType grad_x = 0.f;
        if (x < 1.f)
            grad_x = (Height(x + 1, y) - Height(x, y)) * 0.5f;
        else if (x > ScalarType(m_Nx - 2))
            grad_x = (Height(x, y) - Height(x - 1, y)) * 0.5f;
        else
            grad_x = (Height(x + 1, y) - Height(x - 1, y)) * 0.5f;

        ScalarType grad_y = 0.f;
        if (y < 1.f)
            grad_y = (Height(x, y + 1) - Height(x, y)) * 0.5f;
        else if (y > ScalarType(m_Ny - 2))
            grad_y = (Height(x, y) - Height(x, y - 1)) * 0.5f;
        else
            grad_y = (Height(x, y + 1) - Height(x, y - 1)) * 0.5f;

        return {grad_x, grad_y};
    }

    ScalarType ScalarField::Laplacian(IndexType i, IndexType j) const
    {
        ScalarType laplacian_x = 0.f;
        if (i == 0)
            laplacian_x = (Height(i + 2, j) - 2.f * Height(i + 1, j) + Height(i, j));
        else if (i == m_Nx - 1)
            laplacian_x = (Height(i, j) - 2.f * Height(i - 1, j) + Height(i - 2, j));
        else
            laplacian_x = (Height(i + 1, j) - 2.f * Height(i, j) + Height(i - 1, j));

        ScalarType laplacian_y = 0.f;
        if (j == 0)
            laplacian_y = (Height(i, j + 2) - 2.f * Height(i, j + 1) + Height(i, j));
        else if (j == m_Ny - 1)
            laplacian_y = (Height(i, j) - 2.f * Height(i, j - 1) + Height(i, j - 2));
        else
            laplacian_y = (Height(i, j + 1) - 2.f * Height(i, j) + Height(i, j - 1));

        return laplacian_x + laplacian_y;
    }

    ScalarType ScalarField::Laplacian(ScalarType x, ScalarType y) const
    {
        ScalarType laplacian_x = 0.f;
        if (x < 1.f)
            laplacian_x = (Height(x + 2, y) - 2.f * Height(x + 1, y) + Height(x, y));
        else if (x > ScalarType(m_Nx - 2))
            laplacian_x = (Height(x, y) - 2.f * Height(x - 1, y) + Height(x - 2, y));
        else
            laplacian_x = (Height(x + 1, y) - 2.f * Height(x, y) + Height(x - 1, y));

        ScalarType laplacian_y = 0.f;
        if (y < 1.f)
            laplacian_y = (Height(x, y + 2) - 2.f * Height(x, y + 1) + Height(x, y));
        else if (y > ScalarType(m_Ny - 2))
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

        ImageData image(nx, ny, 3);

        for (int j = 0; j < ny; ++j)
        {
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                ScalarType h = Height({u, v});
                auto value = static_cast<PixelType>(h * 255.f);
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
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                vec2 grad = Gradient(u, v);
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
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<PixelType>((grads[j * nx + i].x - min.x) / (max.x - min.x) * 255.f);
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<PixelType>((grads[j * nx + i].y - min.y) / (max.y - min.y) * 255.f);
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

        std::string fullpath = std::string(DATA_DIR) + "/output/" + filename;

        Array2 laplacians(nx, ny);
        for (int j = 0; j < ny; ++j)
        {
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                laplacians(i, j) = Laplacian(u, v);
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

        ScalarType nxf = ScalarType(nx < 0 ? m_Nx : nx);
        ScalarType nzf = ScalarType(ny < 0 ? m_Ny : ny);

        std::ofstream file(std::string(DATA_DIR) + "/output/" + filename);

        file << nx * ny << '\n';

        ScalarType min = std::abs(Min()), max = std::abs(Max());

        ScalarType s = 255.f / max;
        for (int j = 0; j < ny; ++j)
        {
            ScalarType njz = j / nzf * m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType nix = i / nxf * m_Nx;
                ScalarType h = Height({nix, njz});
                ScalarType y = static_cast<PixelType>(std::max(0.f, std::min(255.f, (h + min) * s)));
                file << (ScalarType)i << ' ' << (ScalarType)j << ' ' << y << '\n';
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
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                PixelType value = static_cast<PixelType>(values.Normalize(i, j) * 255.f);
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

    ScalarType ScalarField::Height(IndexType i, IndexType j) const
    {
        if (i >= m_Nx || j >= m_Ny)
            return 0.f;
        return (*this).At(i, j);
    }

    ScalarType ScalarField::Height(ScalarType x, ScalarType y) const
    {
        ScalarType fi = (x - m_A.x) / m_Diag.x;
        int i = int(fi);

        ScalarType fj = (y - m_A.y) / m_Diag.y;
        int j = int(fj);

        ScalarType u = fi - i;
        ScalarType v = fj - j;

        //! Bilinear Interpolation
        return (1 - u) * (1 - v) * Height(i, j) + (1 - u) * v * Height(i, j + 1) + u * (1 - v) * Height(i + 1, j) + u * v * Height(i + 1, j + 1);
    }

    ScalarType ScalarField::Height(const vec2 &point) const
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

    HeightField::HeightField(const std::vector<ScalarType> &elevations, int nx, int ny) : ScalarField(elevations, nx, ny)
    {
    }

    HeightField::HeightField(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny) : ScalarField(elevations, a, b, nx, ny)
    {
    }

    Ref<HeightField> HeightField::Create(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny)
    {
        return create_ref<HF>(elevations, a, b, nx, ny);
    }

    Mesh HeightField::Polygonize(int n) const
    {
        Mesh mesh(GL_TRIANGLES);

        ScalarType step = 1.f / ScalarType(n - 1);
        for (int j = 0; j < n; ++j)
        {
            ScalarType v = j * step * m_Ny;
            for (int i = 0; i < n; ++i)
            {
                ScalarType u = i * step * m_Nx;

                mesh.normal(Normal(u, v));
                mesh.texcoord({u / (ScalarType)m_Nx, v / (ScalarType)m_Ny});
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
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                vec3 normal = Normal(u, v);
                image.pixels[(j * nx + i) * 3 + 0] = static_cast<PixelType>(std::max(0.f, std::min(255.f, (normal.x + 0.5f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 2] = static_cast<PixelType>(std::max(0.f, std::min(255.f, (normal.y + 0.5f) * 0.5f * 255.f)));
                image.pixels[(j * nx + i) * 3 + 1] = static_cast<PixelType>(std::max(0.f, std::min(255.f, (normal.z + 0.5f) * 0.5f * 255.f)));
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
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                slope(i, j) = Slope(u, v);
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
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                avgslope(i, j) = AverageSlope(u, v);
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
            ScalarType v = (ScalarType)j / (ScalarType)ny * (ScalarType)m_Ny;
            for (int i = 0; i < nx; ++i)
            {
                ScalarType u = (ScalarType)i / (ScalarType)nx * (ScalarType)m_Nx;
                Vector normal = Normal(u, v);
                ScalarType cos_theta = std::max(0.f, dot(normalize(-light_direction), normal));
                PixelType value = static_cast<PixelType>(cos_theta * 255.f);
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

    int HeightField::ExportObj(const std::string &filename, int resolution)
    {
        return write_mesh(Polygonize(resolution), filename.c_str());
    }

    bool comp(ScalarType a, ScalarType b) { return a > b; }

    /**
     * |v00|v10|v20|---|
     * |v01|idx|v21|---|
     * |v02|v12|v22|---|
     */
    int HeightField::StreamArea(const std::string &filename) const
    {
        //! On trie les hauteurs dans l'ordre décroissant et on les stocke dans une queue
        std::priority_queue<std::pair<ScalarType, int>> Q;
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
            ScalarType u = int(std::round(grad.x));
            ScalarType v = int(std::round(grad.y));

            /*if (d == 0.f)
            {
                int startI = I - 1 < 0 ? 0 : I - 1;
                int endI = I + 1 > m_Nx - 1 ? m_Nx - 1 : I + 1;
                int startJ = J - 1 < 0 ? 0 : J - 1;
                int endJ = J + 1 > m_Ny - 1 ? m_Ny - 1 : J + 1;
                for (int j = startJ; j < endJ; ++j)
                {
                    for (int i = startI; i < endI; ++i)
                    {
                        if (i != I || j != J)
                        {
                            A(i, j) += A(I, J);
                            if (abs(I) + abs(J) == 2)
                                A(i, j) /= sqrt(2);
                        }
                    }
                }
            }
            else */
            if (I + u >= 0 && I + u < m_Nx && J + v >= 0 && J + v < m_Ny)
            {
                ScalarType d = sqrt(abs(u) + abs(v));
                if (d != 0.f)
                    A(I + u, J + v) += A(I, J) / d;
            }

            // for (int j = startJ; j < endJ; ++j)
            // {
            //     for (int i = startI; i < endI; ++i)
            //     {
            //         if (i != I || j != J)
            //         {

            //         }
            //     }
            // }
        }

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
                image.pixels[(j * m_Nx + i) * 3 + 0] = 0;
                image.pixels[(j * m_Nx + i) * 3 + 1] = 0;
                image.pixels[(j * m_Nx + i) * 3 + 2] = static_cast<PixelType>(A.Normalize(i, j) * 255.f);
            }
        }

        if (write_image_data(image, fullpath.c_str()) < 0)
            return -1;

#ifndef NDEBUG
        utils::status("[Stream area] Image ", filename, " successfully saved in ./data/output");
#endif
        return 0;
    }

    Vector HeightField::Normal(IndexType i, IndexType j) const
    {
        vec2 grad = Gradient(i, j);
        return normalize({-grad.x, 1.f, -grad.y});
    }

    Vector HeightField::Normal(ScalarType x, ScalarType y) const
    {
        vec2 grad = Gradient(x, y);
        return normalize({-grad.x, 1.f, -grad.y});
    }

    ScalarType HeightField::Slope(IndexType i, IndexType j) const
    {
        return length(Gradient(i, j));
    }

    ScalarType HeightField::Slope(ScalarType x, ScalarType y) const
    {
        return length(Gradient(x, y));
    }

    ScalarType HeightField::AverageSlope(IndexType i, IndexType j) const
    {
        assert(i >= 0.f && i <= m_Nx);
        assert(j >= 0.f && j <= m_Ny);

        int count = 1;
        ScalarType sum_slope = Slope(i, j);
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

        if (i + 1 < (ScalarType)m_Nx)
        {
            sum_slope += Slope(i + 1, j);
            count++;
        }

        if (j - 1 >= 0)
        {
            sum_slope += Slope(i, j - 1);
            count++;
        }

        if (j + 1 < (ScalarType)m_Ny)
        {
            sum_slope += Slope(i, j + 1);
            count++;
        }

        return sum_slope / (ScalarType)count;
    }

    ScalarType HeightField::AverageSlope(ScalarType x, ScalarType y) const
    {
        assert(x >= 0.f && x <= (ScalarType)m_Nx);
        assert(y >= 0.f && y <= (ScalarType)m_Ny);

        int count = 1;
        ScalarType sum_slope = Slope(x, y);
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

        if (x + 1 < (ScalarType)m_Nx)
        {
            sum_slope += Slope(x + 1, y);
            count++;
        }

        if (y - 1 >= 0)
        {
            sum_slope += Slope(x, y - 1);
            count++;
        }

        if (y + 1 < (ScalarType)m_Ny)
        {
            sum_slope += Slope(x, y + 1);
            count++;
        }

        return sum_slope / (ScalarType)count;
    }
} // namespace mmv
