#pragma once

#include "pch.h"

#include "Memory.h"

using PixelType = unsigned char;
using ScalarType = float;
using IndexType = int;

namespace mmv
{
    
    class Array2
    {
    public:
        Array2() = default;
        explicit Array2(int dim);
        Array2(int nx, int ny, ScalarType v=0.f);
        Array2(const std::vector<ScalarType> &elements, int nx, int ny);
        Array2(const vec2 &a, const vec2 &b, int nx, int ny, ScalarType v=0.f);
        Array2(const std::vector<ScalarType> &elements, const vec2 &a, const vec2 &b, int nx, int ny);
        ~Array2() = default;

        //! Get the elements with position (i [col], j [row]).
        ScalarType &operator()(IndexType i, IndexType j);
        
        ScalarType &operator()(IndexType i);

        //! Get the elements with position (i [col], j [row]).
        ScalarType At(IndexType i, IndexType j) const;

        ScalarType At(IndexType i) const;

        //! Return the normalize value between 0 and 1
        ScalarType Normalize(IndexType i, IndexType j) const;

        ScalarType Clamp(IndexType i, IndexType j, ScalarType l, ScalarType h) const;

        //! Getters
        int Nx() const;
        int Ny() const;

        vec2 A() const;
        vec2 B() const;

        ScalarType Min() const;
        ScalarType Max() const;

        void UpdateMinMax();

        //! Compute the diagonal vector of a cell.
        vec2 Diagonal() const;

    protected:
        std::vector<ScalarType> m_Elements{};

        vec2 m_A{0.f, 0.f}, m_B{10.f, 10.f}; //! Boundaries

        int m_Nx{10}, m_Ny{10}; //! Resolution on x & y

    private: 
        ScalarType m_Min, m_Max;
    };

    class ScalarField : public Array2
    {
    public:
        ScalarField();
        explicit ScalarField(int dim);
        ScalarField(int nx, int ny);
        ScalarField(const std::vector<ScalarType> &elevations, int nx, int ny);
        ScalarField(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);
        ~ScalarField() = default;

        static Ref<ScalarField> Create(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);

        //! Modify elevation values 
        void Elevations(const std::vector<ScalarType>& elevations, int nx = -1, int ny = -1);

        //! Get the 3D point from the scalar field.
        Point Point3D(IndexType i, IndexType j) const;
        
        //! Get height of the point with position (i [col], j [row]).
        ScalarType Height(IndexType i, IndexType j) const;

        //! Get height of a point within the scalar field.
        ScalarType Height(ScalarType x, ScalarType y) const; 

        //! Get height of a point within the scalar field.
        ScalarType Height(const vec2& point) const; 

        //! Compute gradient at a point with position (i [col], j [row]).
        vec2 Gradient(IndexType i, IndexType j) const;

        //! Compute gradient for a given point with 2D coordinates (x, y).
        vec2 Gradient(ScalarType x, ScalarType y) const;

        //! Compute laplacian at a point with position (i [col], j [row]).
        ScalarType Laplacian(IndexType i, IndexType j) const;

        //! Compute laplacian for a given point with 2D coordinates (x, y).
        ScalarType Laplacian(ScalarType x, ScalarType y) const;

        //! Save the elevations of a scalarfield as a grayscale image. 
        int ExportElevation(const std::string& filename, int nx=-1, int ny=-1);

        //! Save an image of the gradient values. 
        int ExportGradient(const std::string& filename, int nx=-1, int ny=-1);

        //! Save an image of the laplacian values. 
        int ExportLaplacian(const std::string& filename, int nx=-1, int ny=-1);
        
        //! Save the elevations of a scalarfield as a text file containing each point coordinates. 
        int ExportElevationAsTxt(const std::string& filename, int nx=-1, int ny=-1);

    protected:
        int ExportGrayscaleImage(const std::string& filename, const int nx, const int ny, const Array2& values) const;

    protected:
        vec2 m_Diag{};
    } typedef SF;

    class HeightField : public ScalarField
    {
    public: 
        HeightField();
        explicit HeightField(int dim);
        HeightField(int nx, int ny);
        HeightField(const std::vector<ScalarType> &elevations, int nx, int ny);
        HeightField(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);
        ~HeightField()=default;

        //! Return a shared pointer on a new instance of HF. 
        static Ref<HeightField> Create(const std::vector<ScalarType> &elevations, const vec2 &a, const vec2 &b, int nx, int ny);

        //! Return a mesh of the HF. 
        Mesh Polygonize(int resolution) const; 

        //! Compute the normal vector at point of coordinates (i [col], j [row]) in the grid. 
        Vector Normal(IndexType i, IndexType j) const;

        //! Compute the normal vector for a given point of coordinates (x, y) in 2D. 
        Vector Normal(ScalarType x, ScalarType y) const;

        //! Compute the slope at point of coordinates (i [col], j [row]) in the grid. 
        ScalarType Slope(IndexType i, IndexType j) const;

        //! Compute the slope for a given point of coordinates (x, y) in 2D. 
        ScalarType Slope(ScalarType x, ScalarType y) const;

        //! Compute the average slope (8-connexity) for a given point (i [col], j [row]) in the grid.
        ScalarType AverageSlope(IndexType i, IndexType j) const;

        //! Compute the average slope (8-connexity) for a given point of coordinates (x, y) in 2D.
        ScalarType AverageSlope(ScalarType x, ScalarType y) const;

        //! Save an image of the normals. 
        int ExportNormal(const std::string &filename, int nx = -1, int ny = -1) const;

        //! Save an image of the slopes. 
        int ExportSlope(const std::string &filename, int nx = -1, int ny = -1) const;

        //! Save an image of the average slopes. 
        int ExportAverageSlope(const std::string &filename, int nx = -1, int ny = -1) const;
        
        //! Save an image of the shading. 
        int ExportShading(const std::string &filename, const Vector& light_direction, int nx = -1, int ny = -1) const;

        //! Export the Height Field as an OBJ.
        int ExportObj(const std::string& filename, int resolution);

        void CompleteBreach();

        int StreamArea(const std::string& filename) const;

        int StreamPower() const;

    } typedef HF;
} // namespace mmv
