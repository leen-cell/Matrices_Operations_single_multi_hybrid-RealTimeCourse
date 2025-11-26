#ifndef MATRIX_IO_MEM_H
#define MATRIX_IO_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

// Matrix structure
typedef struct {
    int rows;
    int cols;
    double **data;
} Matrix;

// Global variables
extern Matrix *matrices;
extern int matrix_count;

// Core memory functions
Matrix allocate_matrix(int rows, int cols);
void free_matrix_data(Matrix m);
void free_all_global_matrices(void);
void add_result_to_memory(Matrix result);
void delete_matrix_by_index(int index);

// I/O functions
Matrix read_matrix_from_file(const char *filename);
Matrix get_matrix_from_memory(int index);
void display_matrix_by_index(int index);
Matrix get_user_input_matrix(const char *prompt_name);

// Menu & helpers
int display_menu(void);
void display_all_matrices(void);
void enter_matrix_to_file(void);
void display_folder(void);
void save_all_matrices_to_folder(void);
void enter_matrix(void);
// Helper used in main.c
Matrix get_matrix_from_user(const char *name);
double determinant_gaussian(double **A, int n);

#endif
