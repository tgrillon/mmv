#pragma once

#include "pch.h"

#include "Memory.h"

namespace mmv
{
    class Grid
    {
    public:
        Grid() = default;
        Grid(int nx, int ny, const vec2 &min, const vec2 &max, const std::vector<float> &elements);
        ~Grid() = default;

        //! Get the elements with position (i [col], j [row]).
        float &operator()(int i, int j);

        //! Get the elements with position (i [col], j [row]).
        float At(int i, int j) const;

        //! Getters
        float Nx() const;
        float Ny() const;

        vec2 A() const;
        vec2 B() const;

        float Min() const;
        float Max() const;

        //! Compute the diagonal vector of a cell.
        vec2 Diagonal() const;

    protected:
        std::vector<float> m_Elements{};

        vec2 m_A{0.f, 0.f}, m_B{10.f, 10.f}; //! Boundaries

        int m_Nx{10}, m_Ny{10}; //! Resolution on x & y
    };

    class ScalarField : public Grid
    {
    public:
        ScalarField();
        ScalarField(const std::vector<float> &heights, const vec2 &min, const vec2 &max, int nx, int ny);
        ~ScalarField() = default;

        static Ref<ScalarField> Create(const std::vector<float> &heights, const vec2 &min, const vec2 &max, int nx, int ny);

        //! Get the 3D point from the scalar field.
        Point Point(int i, int j) const;
        
        //! Compute gradient at a point with position (i [col], j [row]).
        vec2 Grad(int i, int j) const;

        //! Get height of the point with position (i [col], j [row]).
        float Height(int i, int j) const;

        //! Get height of a point within the scalar field.
        float Height(const vec2& point) const; 

    private:
        vec2 m_Diag{};
    } typedef SF;

    void save_scalar_field(const std::string& filename, const Ref<ScalarField> &sf, int nx=-1, int ny=-1);
} // namespace mmv
