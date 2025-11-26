#ifndef MATRIX_CALC_H
#define MATRIX_CALC_H

#include "matrix_io_mem.h"
#include <math.h>

// Structure for eigen results
typedef struct {
    double *eigenvalues;
    Matrix *eigenvectors;
    int count;
} EigenResult;

// Arithmetic operations
Matrix matrix_add(Matrix A, Matrix B);
Matrix matrix_subtract(Matrix A, Matrix B);
Matrix matrix_multiply(Matrix A, Matrix B);
double matrix_determinant(Matrix M);
EigenResult matrix_eigen(Matrix M);

#endif