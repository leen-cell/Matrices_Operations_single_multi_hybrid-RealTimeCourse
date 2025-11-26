#ifndef MATRIX_OMP_MULTI_H
#define MATRIX_OMP_MULTI_H

#include "matrix_io_mem.h"
#include "matrix_calc.h"
#include "matrix_multi.h"  // reuse structs and process pool

pid_t create_child_hybrid(void);
int get_free_child_hybrid(void);

// Determinant computation (hybrid)
 void get_cofactor_hybrid(double **M, int n, double **temp, int p, int q);
 double det_recursive_hybrid(double **M, int n);
//double det_recursive_serial_hybrid(double **M, int n);

double matrix_determinant_hybrid(Matrix M);
Matrix matrix_multiply_hybrid(Matrix x, Matrix y);
EigenResult matrix_eigen_hybrid(Matrix M);

// Menu handler (if used directly)

#endif