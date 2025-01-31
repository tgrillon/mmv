#include "ImageUtils.h"

void convolve(const std::vector<scalar_t> &input, std::vector<scalar_t> &output, const int nx, const int ny, const float *kernel, const int nk)
{
    const int N = 8;
    const int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    const int dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};

    for (int j = 0; j < ny; ++j)
    {
        for (int i = 0; i < nx; ++i)
        {
            int count = 0;
            for (int k = 0; k < N; ++k)
            {
                const int pi = i + dx[k];
                const int pj = j + dy[k];
                
                if (pi < 0 || pi >= nx || pj < 0 || pj >= ny)
                    continue;

                const int qi = 1 + dx[k];
                const int qj = 1 + dy[k];
                output[j * nx + i] += kernel[qj * nk + qi] * input[pj * nx + pi];
                count++;
            }

            if (count > 0)
                output[j * nx + i] += kernel[nk + nk / 2] * input[j * nx + i];
        }
    }
}
