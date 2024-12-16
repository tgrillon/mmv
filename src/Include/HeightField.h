#pragma once

#include "pch.h"

#include "ImageUtils.h"
#include "Memory.h"

using pixel_t = unsigned char;
using scalar_t = float;
using index_t = int;

namespace mmv
{
    template <typename T>
    class Array2
    {
    public:
        Array2() = default;
        explicit Array2(int dim) : Array2(dim, dim) {}
        Array2(int nx, int ny, T v = 0) : m_Nx(nx), m_Ny(ny), m_Elements(nx * ny, v), m_A(0, 0), m_B(nx, ny), m_Min(v), m_Max(v) {}
        Array2(const std::vector<T> &elements, int nx, int ny) : m_Nx(nx), m_Ny(ny), m_A(0, 0), m_B(nx, ny), m_Elements(elements)
        {
            assert(nx * ny == elements.size());
            m_Min = *std::min_element(m_Elements.begin(), m_Elements.end());
            m_Max = *std::max_element(m_Elements.begin(), m_Elements.end());
        }

        Array2(const vec2 &a, const vec2 &b, int nx, int ny, T v = 0) : Array2(nx, ny, v)
        {
            m_A = a;
            m_B = b;
        }

        Array2(const std::vector<T> &elements, const vec2 &a, const vec2 &b, int nx, int ny) : Array2(elements, nx, ny)
        {
            m_A = a;
            m_B = b;
        }

        ~Array2() = default;

        //! Get the elements with position (i [col], j [row]).
        inline T &operator()(index_t i, index_t j)
        {
            assert(InBounds(i, j));
            return m_Elements[j * m_Nx + i];
        }

        inline T &operator()(index_t i)
        {
            assert(InBounds(i));
            return m_Elements[i];
        }

        //! Get the elements with position (i [col], j [row]).
        inline T At(index_t i, index_t j) const
        {
            assert(InBounds(i, j));
            return m_Elements[j * m_Nx + i];
        }

        inline T At(index_t i) const
        {
            assert(InBounds(i));
            return m_Elements[i];
        }

        inline index_t OneDIndex(index_t i, index_t j) const
        {
            assert(InBounds(i, j));
            return j * m_Nx + i;
        }

        inline bool InBounds(index_t i, index_t j) const
        {
            return i >= 0 && i < m_Nx && j >= 0 && j < m_Ny;
        }

        inline bool InBounds(index_t i) const
        {
            return i >= 0 && i < m_Nx * m_Ny;
        }

        //! Return the normalize value between 0 and 1
        inline T Normalize(index_t i, index_t j) const
        {
            return (m_Elements[j * m_Nx + i] - m_Min) / (m_Max - m_Min);
        }

        inline T Clamp(index_t i, index_t j, T l, T h) const
        {
            return std::min(h, std::max(l, m_Elements[j * m_Nx + i]));
        }

        void Smooth();

        //! Getters
        inline int Nx() const { return m_Nx; }

        inline int Ny() const { return m_Ny; }

        inline vec2 A() const { return m_A; }
        inline vec2 B() const { return m_B; }

        inline T Min() const { return m_Min; }
        inline T Max() const { return m_Max; }

        inline void UpdateMinMax()
        {
            m_Min = *std::min_element(m_Elements.begin(), m_Elements.end());
            m_Max = *std::max_element(m_Elements.begin(), m_Elements.end());
        }

        //! Compute the diagonal vector of a cell.
        inline vec2 Diagonal() const
        {
            assert(m_Nx - 1 > 0);
            assert(m_Ny - 1 > 0);
            return {(m_B.x - m_A.x) / (m_Nx - 1), (m_B.y - m_A.y) / (m_Ny - 1)};
        }

    protected:
        std::vector<T> m_Elements{};

        vec2 m_A{0.f, 0.f}, m_B{10.f, 10.f}; //! Boundaries

        int m_Nx{10}, m_Ny{10}; //! Resolution on x & y

    private:
        scalar_t m_Min, m_Max;
    };

    template <typename T>
    inline void Array2<T>::Smooth()
    {
        const float d = 1. / 16.;
        const float kernel[9] = { d, 2. * d, d, 2. * d, 4. * d, 2. * d, d, 2. * d, d };

        std::vector<scalar_t> output(m_Nx * m_Ny); 
        convolve(m_Elements, output, m_Nx, m_Ny, kernel);
        m_Elements = output;
    }

    class ScalarField : public Array2<scalar_t>
    {
    public:
        ScalarField();
        explicit ScalarField(int dim);
        ScalarField(int nx, int ny);
        ScalarField(const std::vector<scalar_t> &elevations, int nx, int ny);
        ScalarField(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);
        ~ScalarField() = default;

        static Ref<ScalarField> Create(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);

        //! Modify elevation values
        void Elevations(const std::vector<scalar_t> &elevations, int nx = -1, int ny = -1);

        //! Get the 3D point from the scalar field.
        Point Point3D(index_t i, index_t j) const;

        //! Get height of the point with position (i [col], j [row]).
        scalar_t Height(index_t i, index_t j) const;

        //! Get height of a point within the scalar field.
        scalar_t Height(scalar_t x, scalar_t y) const;

        //! Get height of a point within the scalar field.
        scalar_t Height(const vec2 &point) const;

        //! Compute gradient at a point with position (i [col], j [row]).
        vec2 Gradient(index_t i, index_t j) const;

        //! Compute gradient for a given point with 2D coordinates (x, y).
        vec2 Gradient(scalar_t x, scalar_t y) const;

        //! Compute laplacian at a point with position (i [col], j [row]).
        scalar_t Laplacian(index_t i, index_t j) const;

        //! Compute laplacian for a given point with 2D coordinates (x, y).
        scalar_t Laplacian(scalar_t x, scalar_t y) const;

        //! Save the elevations of a scalarfield as a grayscale image.
        int ExportElevation(const std::string &filename, int nx = -1, int ny = -1);

        //! Save an image of the gradient values.
        int ExportGradient(const std::string &filename, int nx = -1, int ny = -1);

        //! Save an image of the laplacian values.
        int ExportLaplacian(const std::string &filename, int nx = -1, int ny = -1);

        //! Save the elevations of a scalarfield as a text file containing each point coordinates.
        int ExportElevationAsTxt(const std::string &filename, int nx = -1, int ny = -1);

    protected:
        int ExportGrayscaleImage(const std::string &filename, const int nx, const int ny, const Array2 &values) const;

    protected:
        vec2 m_Diag{};
    } typedef SF;

    class HeightField : public ScalarField
    {
    public:
        HeightField();
        explicit HeightField(int dim);
        HeightField(int nx, int ny);
        HeightField(const std::vector<scalar_t> &elevations, int nx, int ny);
        HeightField(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);
        ~HeightField() = default;

        //! Return a shared pointer on a new instance of HF.
        static Ref<HeightField> Create(const std::vector<scalar_t> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);

        //! Return a mesh of the HF.
        Mesh Polygonize(int resolution) const;

        //! Compute the normal vector at point of coordinates (i [col], j [row]) in the grid.
        Vector Normal(index_t i, index_t j) const;

        //! Compute the normal vector for a given point of coordinates (x, y) in 2D.
        Vector Normal(scalar_t x, scalar_t y) const;

        //! Compute the slope at point of coordinates (i [col], j [row]) in the grid.
        scalar_t Slope(index_t i, index_t j) const;

        //! Compute the slope for a given point of coordinates (x, y) in 2D.
        scalar_t Slope(scalar_t x, scalar_t y) const;

        //! Compute the average slope (8-connexity) for a given point (i [col], j [row]) in the grid.
        scalar_t AverageSlope(index_t i, index_t j) const;

        //! Compute the average slope (8-connexity) for a given point of coordinates (x, y) in 2D.
        scalar_t AverageSlope(scalar_t x, scalar_t y) const;

        //! Save an image of the normals.
        int ExportNormal(const std::string &filename, int nx = -1, int ny = -1) const;

        //! Save an image of the slopes.
        int ExportSlope(const std::string &filename, int nx = -1, int ny = -1) const;

        //! Save an image of the average slopes.
        int ExportAverageSlope(const std::string &filename, int nx = -1, int ny = -1) const;

        //! Save an image of the shading.
        int ExportShading(const std::string &filename, const Vector &light_direction, int nx = -1, int ny = -1) const;

        //! Export the Height Field as an OBJ.
        int ExportObj(const std::string &filename, int resolution);

        int ExportStreamArea(const std::string &filename) const;

        Array2 StreamArea() const;

        void CompleteBreach();

        void StreamPower();

    } typedef HF;
} // namespace mmv
