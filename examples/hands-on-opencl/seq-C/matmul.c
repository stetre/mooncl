//------------------------------------------------------------------------------
//
// Run sequential version of matrix multiplication on the host
//
// Derived from the HandsOnOpenCL code (matrix_lib.[ch], matmul.[ch], wtime.c).
//
//------------------------------------------------------------------------------

#if 0
#endif

#include "matrix_lib.h"

extern double wtime();

int main(int argc, char *argv[])
    {
    float *h_A;             // A matrix
    float *h_B;             // B matrix
    float *h_C;             // C = A*B matrix
    int N;                  // A[N][N], B[N][N], C[N][N]
    int size;               // number of elements in each matrix

    double start_time;      // Starting time
    double run_time;        // timing data

    N    = ORDER;
    size = N * N;

    h_A = (float *)malloc(size * sizeof(float));
    h_B = (float *)malloc(size * sizeof(float));
    h_C = (float *)malloc(size * sizeof(float));

    // Run sequential version on the host
    initmat(N, h_A, h_B, h_C);

    printf("\n===== Sequential (C), matrix mult (dot prod), order %d on host CPU ======\n",ORDER);
    for(int i = 0; i < COUNT; i++)
        {
        zero_mat(N, h_C);
        start_time = wtime();
        seq_mat_mul_sdot(N, h_A, h_B, h_C);
        run_time  = wtime() - start_time;
        results(N, h_C, run_time);
        }

    free(h_A);
    free(h_B);
    free(h_C);
    return EXIT_SUCCESS;
    }

