#include "HeightField.h"

#define EXPECT_EQ(X, Y) if (X != Y) std::exit(1);

void GridConstructTest()
{
    std::vector<float> elements = {1.0f, 2.0f, 3.0f, 4.0f};
    vec2 A = {0.f, 0.f}, B = {10.f, 10.f};
    int nx = 2, ny = 2;

    mmv::Array2 grid(elements, A, B, nx, ny);
    
    EXPECT_EQ(grid.Nx(), 2);
    EXPECT_EQ(grid.Ny(), 2);
    EXPECT_EQ(grid.At(0, 0), 1.0f);
    EXPECT_EQ(grid.At(1, 1), 4.0f);
} 
