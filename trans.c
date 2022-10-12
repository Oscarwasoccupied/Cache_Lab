/**
 * @file trans.c
 * @brief Contains various implementations of matrix transpose
 *
 * Each transpose function must have a prototype of the form:
 *   void trans(size_t M, size_t N, double A[N][M], double B[M][N],
 *              double tmp[TMPCOUNT]);
 *
 * All transpose functions take the following arguments:
 *
 *   @param[in]     M    Width of A, height of B
 *   @param[in]     N    Height of A, width of B
 *   @param[in]     A    Source matrix
 *   @param[out]    B    Destination matrix
 *   @param[in,out] tmp  Array that can store temporary double values
 *
 * A transpose function is evaluated by counting the number of hits and misses,
 * using the cache parameters and score computations described in the writeup.
 *
 * Programming restrictions:
 *   - No out-of-bounds references are allowed
 *   - No alterations may be made to the source array A
 *   - Data in tmp can be read or written
 *   - This file cannot contain any local or global doubles or arrays of doubles
 *   - You may not use unions, casting, global variables, or
 *     other tricks to hide array data in other forms of local or global memory.
 *
 * TODO: xianweiz
 * @author Xianwei Zou <xianweiz@andrew.cmu.edu>
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "cachelab.h"

/**
 * @brief Checks if B is the transpose of A.
 *
 * You can call this function inside of an assertion, if you'd like to verify
 * the correctness of a transpose function.
 *
 * @param[in]     M    Width of A, height of B
 * @param[in]     N    Height of A, width of B
 * @param[in]     A    Source matrix
 * @param[out]    B    Destination matrix
 *
 * @return True if B is the transpose of A, and false otherwise.
 */

void transpose_32(size_t M, size_t N, double A[N][M], double B[M][N],
                  double tmp[TMPCOUNT]);
void transpose_1024(size_t M, size_t N, double A[N][M], double B[M][N],
                    double tmp[TMPCOUNT]);
#ifndef NDEBUG
static bool is_transpose(size_t M, size_t N, double A[N][M], double B[M][N]) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                fprintf(stderr,
                        "Transpose incorrect.  Fails for B[%zd][%zd] = %.3f, "
                        "A[%zd][%zd] = %.3f\n",
                        j, i, B[j][i], i, j, A[i][j]);
                return false;
            }
        }
    }
    return true;
}
#endif

/*
 * You can define additional transpose functions here. We've defined
 * some simple ones below to help you get started, which you should
 * feel free to modify or delete.
 */

/**
 * @brief A simple baseline transpose function, not optimized for the cache.
 *
 * Note the use of asserts (defined in assert.h) that add checking code.
 * These asserts are disabled when measuring cycle counts (i.e. when running
 * the ./test-trans) to avoid affecting performance.
 */
static void trans_basic(size_t M, size_t N, double A[N][M], double B[M][N],
                        double tmp[TMPCOUNT]) {
    assert(M > 0);
    assert(N > 0);

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            B[j][i] = A[i][j];
        }
    }

    assert(is_transpose(M, N, A, B));
}

/**
 * @brief A contrived example to illustrate the use of the temporary array.
 *
 * This function uses the first four elements of tmp as a 2x2 array with
 * row-major ordering.
 */
static void trans_tmp(size_t M, size_t N, double A[N][M], double B[M][N],
                      double tmp[TMPCOUNT]) {
    assert(M > 0);
    assert(N > 0);

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            size_t di = i % 2;
            size_t dj = j % 2;
            tmp[2 * di + dj] = A[i][j];
            B[j][i] = tmp[2 * di + dj];
        }
    }

    assert(is_transpose(M, N, A, B));
}

/**
 * @brief The solution transpose function that will be graded.
 *
 * You can call other transpose functions from here as you please.
 * It's OK to choose different functions based on array size, but
 * this function must be correct for all values of M and N.
 */
static void transpose_submit(size_t M, size_t N, double A[N][M], double B[M][N],
                             double tmp[TMPCOUNT]) {
    if (M == 32 && N == 32) {
        transpose_32(M, N, A, B, tmp);
    } else if (M == 1024 && N == 1024) {
        transpose_1024(M, N, A, B, tmp);
    } else {
        trans_tmp(M, N, A, B, tmp);
    }
}

void transpose_32(size_t M, size_t N, double A[N][M], double B[M][N],
                  double tmp[TMPCOUNT]) {
    // clock cycle: 40832
    // for (size_t i = 0; i < 32; i += 8) {
    //     for (size_t j = 0; j < 32; j += 8) {
    //         for (size_t m = i; m < i + 8; m++) {
    //             for (size_t n = j; n < j + 8; n++) {
    //                 B[n][m] = A[m][n];
    //             }
    //         }
    //     }
    // }

    for (size_t i = 0; i < N; i += 8) {
        for (size_t j = 0; j < M; j += 8) {
            for (size_t m = i; m < i + 8; m++) {
                for (size_t n = j; n < j + 8; n++) {
                    if (m != n) {
                        B[n][m] = A[m][n];
                    }
                    if (n == (j + 7) && (i == j)) {
                        B[m][m] = A[m][m];
                    }
                }
            }
        }
    }
}
void transpose_1024(size_t M, size_t N, double A[N][M], double B[M][N],
                    double tmp[TMPCOUNT]) {
    for (size_t i = 0; i < N; i += 8) {
        for (size_t j = 0; j < M; j += 8) {
            for (size_t m = i; m < i + 8; m++) {
                for (size_t n = j; n < j + 8; n++) {
                    B[n][m] = A[m][n];
                }
            }
        }
    }
}

/**
 * @brief Registers all transpose functions with the driver.
 *
 * At runtime, the driver will evaluate each function registered here, and
 * and summarize the performance of each. This is a handy way to experiment
 * with different transpose strategies.
 */
void registerFunctions(void) {
    // Register the solution function. Do not modify this line!
    registerTransFunction(transpose_submit, SUBMIT_DESCRIPTION);
    // registerTransFunction(transpose_32, SUBMIT_DESCRIPTION);
    // registerTransFunction(transpose_1024, SUBMIT_DESCRIPTION);

    // Register any additional transpose functions
    registerTransFunction(trans_basic, "Basic transpose");
    registerTransFunction(trans_tmp, "Transpose using the temporary array");
}
