#pragma once

#include "pch.h"

#include "Memory.h"

namespace mmv
{
    class Grid
    {
    public:
        Grid() = default;
        Grid(const std::vector<float> &elements, const vec2 &a, const vec2 &b, size_t nx, size_t ny);
        ~Grid() = default;

        //! Get the elements with position (i [col], j [row]).
        float &operator()(size_t i, size_t j);

        //! Get the elements with position (i [col], j [row]).
        float At(size_t i, size_t j) const;

        //! Getters
        size_t Nx() const;
        size_t Ny() const;

        vec2 A() const;
        vec2 B() const;

        float Min() const;
        float Max() const;

        //! Compute the diagonal vector of a cell.
        vec2 Diagonal() const;

    protected:
        std::vector<float> m_Elements{};

        vec2 m_A{0.f, 0.f}, m_B{10.f, 10.f}; //! Boundaries

        size_t m_Nx{10}, m_Ny{10}; //! Resolution on x & y
    };

    class ScalarField : public Grid
    {
    public:
        ScalarField();
        ScalarField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny);
        ~ScalarField() = default;

        static Ref<ScalarField> Create(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny);

        //! Get the 3D point from the scalar field.
        Point Point3D(size_t i, size_t j) const;
        
        //! Get height of the point with position (i [col], j [row]).
        float Height(size_t i, size_t j) const;

        //! Get height of a point within the scalar field.
        float Height(const vec2& point) const; 

        //! Compute gradient at a point with position (i [col], j [row]).
        vec2 Gradient(size_t i, size_t j) const;

        //! Compute laplacian at a point with position (i [col], j [row]).
        vec2 Laplacian(size_t i, size_t j) const;

    protected:
        vec2 m_Diag{};
    } typedef SF;

    class HeightField : public ScalarField
    {
    public: 
        HeightField();
        HeightField(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny);
        ~HeightField()=default;

        static Ref<HeightField> Create(const std::vector<float> &heights, const vec2 &a, const vec2 &b, size_t nx, size_t ny);

    protected:
        //! Compute the normal vector at point of coordinates (i [col], j [row]) in the grid. 
        Vector Normal(size_t i, size_t j) const;
        //! Compute the slope at point of coordinates (i [col], j [row]) in the grid. 
        float Slope(size_t i, size_t j) const;

    } typedef HF;

    //! Save a scalarfield as a grayscale image. 
    void save_scalar_field(const std::string& filename, const Ref<ScalarField> &sf, size_t nx=-1, size_t ny=-1);

    // Point sin_noise(const Point& p);

    std::vector<float> surface_points(size_t n, const std::function<double(double, double)> &f);
    
} // namespace mmv
