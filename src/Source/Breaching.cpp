#include "Breaching.h"
#include "HeightField.h"

// Neighbour Directions
const int DIRECTIONS = 8;
const int dx[DIRECTIONS] = {-1, -1, 0, 1, 1, 1, 0, -1};
const int dy[DIRECTIONS] = {0, -1, -1, -1, 0, 1, 1, 1};

namespace mmv
{
    /*!
    \brief Breach depressions.

    Depression breaching drills a path from a depression's pit cell (its lowest
    point) along the least-cost (Priority-Flood) path to the nearest cell
    outside the depression to have the same or lower elevation.

    See https://rbarnes.org/

    See https://github.com/r-barnes/richdem

    See https://github.com/r-barnes/richdem/blob/master/include/richdem/depressions/Lindsay2016.hpp

    \author John Lindsay, implementation by Richard Barnes (rbarnes@umn.edu).
    */
    void HeightField::CompleteBreach()
    {
        int mode = COMPLETE_BREACHING;
        bool fill_depressions = true;

        const int NO_BACK_LINK = -1;

        Array2<int> backlinks(m_Nx, m_Ny, NO_BACK_LINK);
        Array2<int> visited(m_Nx, m_Ny, LindsayCellType::UNVISITED);
        Array2<int> pits(m_Nx, m_Ny, false);
        std::vector<index_t> flood_array;
        ZPriorityQueue pq;

        int total_pits = 0;

        // Seed the priority queue
        for (int j = 0; j < m_Ny; j++)
        {
            for (int i = 0; i < m_Nx; i++)
            {
                // Valid edge cells go on priority-queue
                if (i == 0 || i == (m_Nx - 1) || j == 0 || j == (m_Ny - 1))
                {
                    pq.emplace(At(i, j), IPoint2(i, j));
                    visited(i, j) = LindsayCellType::EDGE;
                    continue;
                }

                // Determine if this is an edge cell, gather information used to determine if it is a pit cell
                scalar_t lowest_neighbour = std::numeric_limits<scalar_t>::max();
                for (int n = 0; n < DIRECTIONS; n++)
                {
                    const index_t pi = i + dx[n];
                    const index_t pj = j + dy[n];

                    // No need for an inGrid check here because edge cells are filtered above
                    // Used for identifying the lowest neighbour
                    lowest_neighbour = std::min(At(pi, pj), lowest_neighbour);
                }

                // This is a pit cell if it is lower than any of its neighbours. In this
                // case: raise the cell to be just lower than its lowest neighbour. This
                // makes the breaching/tunneling procedures work better.
                if (At(i, j) < lowest_neighbour)
                {
                    operator()(i, j) = lowest_neighbour - 2.0 * 1e-5;
                }
                // Since depressions might have flat bottoms, we treat flats as pits. Mark
                // flat/pits as such now.
                if (At(i, j) <= lowest_neighbour)
                {
                    pits(i, j) = true;
                    total_pits++; // May not need this
                }
            }
        }

        // The Priority-Flood operation assures that we reach pit cells by passing into
        // depressions over the outlet of minimal elevation on their edge.
        while (!pq.empty())
        {
            const Element c = pq.top();
            pq.pop();

            const IPoint2 p = c.second;

            // T Cell is a pit, consider doing some breaching: locate a cell that is lower than the pit cell, or an edge cell
            if (pits(p.x(), p.y()))
            {
                index_t cc = OneDIndex(p.x(), p.y());      // Current cell on the path
                scalar_t target_height = At(p.x(), p.y()); // Depth to which the cell currently being considered should be carved

                if (mode == COMPLETE_BREACHING)
                {
                    // Trace path back to a cell low enough for the path to drain into it, or to an edge of the DEM
                    while (cc != NO_BACK_LINK && At(cc) >= target_height)
                    {
                        m_Elements[cc] = target_height;
                        cc = backlinks.At(cc);                      // Follow path back
                        target_height = target_height - 2.0 * 1e-5; // Decrease target depth slightly for each cell on path to ensure drainage
                    }
                }
                else
                {
                }

                --total_pits;
                if (total_pits == 0)
                    break;
            }

            // Looks for neighbours which are either unvisited or pits
            for (int n = 0; n < DIRECTIONS; n++)
            {
                const index_t pi = p.x() + dx[n];
                const index_t pj = p.y() + dy[n];

                if (!InBounds(pi, pj))
                    continue;
                if (visited(pi, pj) != LindsayCellType::UNVISITED)
                    continue;

                const scalar_t my_e = At(pi, pj);

                // The neighbour is unvisited. Add it to the queue
                pq.emplace(my_e, IPoint2(pi, pj));
                if (fill_depressions)
                    flood_array.emplace_back(OneDIndex(pi, pj));
                visited(pi, pj) = LindsayCellType::VISITED;
                backlinks(pi, pj) = OneDIndex(p.x(), p.y());
            }

            if (mode != COMPLETE_BREACHING && fill_depressions)
            {
                for (const auto f : flood_array)
                {
                    int parent = backlinks.At(f);
                    if (At(f) <= At(parent))
                    {
                        m_Elements[f] = std::nextafter(At(parent), std::numeric_limits<scalar_t>::max());
                    }
                }
            }
        }
    }
} // namespace mmv
