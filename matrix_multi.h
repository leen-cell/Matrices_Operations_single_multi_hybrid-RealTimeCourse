#ifndef MATRIX_MULTI_H
#define MATRIX_MULTI_H
#include "matrix_io_mem.h"
#include "matrix_calc.h"

#include <unistd.h>     // for fork, pipe, read, write, close, _exit, getpid
#include <signal.h>     // for SIGTERM, kill
#include <sys/types.h>  // for pid_t
#include <time.h>       // for time_t
#include <string.h>     // for memset
#include <stdio.h>      // for perror, printf
#include <stdlib.h>     // for malloc, realloc, free, exit


// structs:
// it can be one if the commands

typedef enum
{
    CMD_EXIT = 0,
    CMD_DETERMINANT = 1,
    CMD_ADD = 2,      // NEW: Command for matrix addition
    CMD_SUBTRACT = 3, // NEW: Command for matrix subtraction
    CMD_MUL = 4,
    CMD_EIGEN = 5
} CommandType;

// union means different data types can occur but only one at a time

typedef struct
{
    CommandType cmd; // tells the child what to do
    int size;        // size of matrix (for determinant)
    union
    {
        struct
        {
            // the args needed by the child for the determinant
            int sign;
            double factor;
        } det_args;
        // NEW: Args for arithmetic operations (add/subtract)
        struct
        {
            int row;  // which row this child should process
            int cols; // number of columns in the matrices
        } arith_args; // for add/subtract
        // Add other operation structs here later
        // since we have different operations its better to do it this way
                struct
        {
        	int row;
        	int n;
        }eigen_args;
    
    } args;
} ChildCommand;

typedef struct
{
    pid_t pid;        // the pid of the child process to keep track
    time_t last_used; // last time the process was used
    int busy;         // 0 = free, 1 = currently working
    int fd_p_to_c[2]; // parent writes, child reads
    int fd_c_to_p[2]; // child writes, parent reads
} ChildProcess;

pid_t create_child();
int get_free_child();
void release_child(pid_t);
void aging(int);
void get_cofactor_mp(double **, int, double **, int, int);
 double det_recursive_mp(double **, int);
double matrix_determinant_mp(Matrix);
 double det_recursive_serial(double **, int);
Matrix matrix_add_mp(Matrix, Matrix);      // NEW: Multiprocessing addition function
Matrix matrix_subtract_mp(Matrix, Matrix); // NEW: Multiprocessing subtraction function
Matrix empty_mt(int row, int col);
Matrix matrix_multiply_mp(Matrix, Matrix);
EigenResult matrix_eigen_mp(Matrix M);


extern int child_count;
extern int IS_CHILD;
extern ChildProcess *child_pool;

#endif