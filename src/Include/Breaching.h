#pragma once

#include "HeightField.h"

namespace mmv
{

    class IPoint2
    {
    public:
        IPoint2(int x, int y) : m_x(x), m_y(y) {}
        inline int x() const { return m_x; }
        inline int y() const { return m_y; }

    private:
        int m_x, m_y;
    };

    using Element = std::pair<scalar_t, IPoint2>;

    struct comp
    {
        // Compare elevation, then x, then y
        bool operator()(const Element &a, const Element &b)
        {
            const IPoint2 &lhs = a.second;
            const IPoint2 &rhs = b.second;

            if (a.first > b.first)
                return true;
            else if ((a.first == b.first) && (lhs.x() > rhs.x() || (lhs.x() == rhs.x() && lhs.y() > rhs.y())))
                return true;
            else
                return false;
        }
    };

    using ZPriorityQueue = std::priority_queue<Element, std::vector<Element>, comp>;

    enum LindsayMode
    {
        COMPLETE_BREACHING,
        SELECTIVE_BREACHING,
        CONSTRAINED_BREACHING
    };

    enum LindsayCellType
    {
        UNVISITED,
        VISITED,
        EDGE
    };

} // namespace mmv
