#pragma once

#include "pch.h"

#include "Memory.h"

namespace mmv
{
    class Grid
    {
    public:
        Grid() = default;
        Grid(const std::vector<float> &elements, const vec2 &a, const vec2 &b, int nx, int nz);
        ~Grid() = default;

        //! Get the elements with position (i [col], j [row]).
        float &operator()(int i, int j);

        //! Get the elements with position (i [col], j [row]).
        float At(int i, int j) const;

        //! Getters
        int Nx() const;
        int Nz() const;

        vec2 A() const;
        vec2 B() const;

        float Min() const;
        float Max() const;

        //! Compute the diagonal vector of a cell.
        vec2 Diagonal() const;

    protected:
        std::vector<float> m_Elements{};

        vec2 m_A{0.f, 0.f}, m_B{10.f, 10.f}; //! Boundaries

        int m_Nx{10}, m_Nz{10}; //! Resolution on x & y
    };

    class ScalarField : public Grid
    {
    public:
        ScalarField();
        ScalarField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, int nx, int nz);
        ~ScalarField() = default;

        static Ref<ScalarField> Create(const std::vector<float> &heights, const vec2 &a, const vec2 &b, int nx, int nz);

        //! Modify elevation values 
        void Elevations(const std::vector<float>& elevations, int nx, int nz);

        //! Get the 3D point from the scalar field.
        Point Point3D(int i, int j) const;
        
        //! Get height of the point with position (i [col], j [row]).
        float Height(int i, int j) const;

        //! Get height of a point within the scalar field.
        float Height(float x, float z) const; 

        //! Get height of a point within the scalar field.
        float Height(const vec2& point) const; 

        //! Compute gradient at a point with position (i [col], j [row]).
        vec2 Gradient(int i, int j) const;

        vec2 Gradient(float x, float z) const;

        //! Compute laplacian at a point with position (i [col], j [row]).
        vec2 Laplacian(int i, int j) const;

        //! Save the elevations of a scalarfield as a grayscale image. 
        int SaveHeightAsImage(const std::string& filename, int nx=-1, int nz=-1);

        int SaveGradientAsImage(const std::string& filename, int nx=-1, int nz=-1);
        
        //! Save the elevations of a scalarfield as a text file containing each point coordinates. 
        int SaveHeightAsTxt(const std::string& filename, int nx=-1, int nz=-1);

    protected:
        vec2 m_Diag{};
    } typedef SF;

    class HeightField : public ScalarField
    {
    public: 
        HeightField();
        HeightField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, int nx, int nz);
        ~HeightField()=default;

        static Ref<HeightField> Create(const std::vector<float> &heights, const vec2 &a, const vec2 &b, int nx, int nz);

    protected:
        //! Compute the normal vector at point of coordinates (i [col], j [row]) in the grid. 
        Vector Normal(int i, int j) const;
        //! Compute the slope at point of coordinates (i [col], j [row]) in the grid. 
        float Slope(int i, int j) const;

    } typedef HF;
} // namespace mmv
