#include "ImageUtils.h"

void convolve(const std::vector<scalar_t> &input, std::vector<scalar_t> &output, int nx, int ny, const float *kernel)
{
    const int N = 8;
    const int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    const int dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};

    for (int j = 1; j < ny - 1; ++j)
    {
        for (int i = 1; i < nx - 1; ++i)
        {
            for (int k = 0; k < N; ++k)
            {
                const int pi = i + dx[k];
                const int pj = j + dy[k];

                output[j * nx + i] += kernel[(1 + dy[k]) * 3 + (1 + dx[k])] * input[pj * nx + pi];
            }
            output[j * nx + i] += kernel[4] * input[j * nx + i];
        }
    }
}
