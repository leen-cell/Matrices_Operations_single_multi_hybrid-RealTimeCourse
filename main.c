#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <omp.h>
#include "matrix_io_mem.h"
#include "matrix_calc.h"
#include "matrix_multi.h"
#include "matrix_omp_multi.h"
 
// Forward declarations for menu handlers
void enter_matrix_handler(void);
void display_matrix_handler(void);
void delete_matrix_handler(void);
void display_all_matrices_handler(void);
void add_matrices_handler(void);
void add_matrices_handler_mp(void); 
void subtract_matrices_handler(void);
void subtract_matrices_handler_mp(void);
void multiply_matrices_handler(void);
void determinant_handler(void);
void determinant_handler_mp(void);
void eigen_handler(void);
void modify_matrix_handler(void);  
void determinant_handler_hybrid(void);
void multiply_handler_hybrid(void);
void eigen_handler_hybrid(void);
double now_ms (void);
void eigen_handler_mp(void); 
void setup_aging_timer(void);
void aging_signal_handler(int);
void (*sigset(int sig, void (*func)(int)))(int);

//passed from the makefile
#if defined(MULTI)
    #define ADD_FUNC add_matrices_handler_mp
    #define SUB_FUNC subtract_matrices_handler_mp
    #define MUL_FUNC multiply_matrices_handler_mp
    #define DET_FUNC determinant_handler_mp
    #define EIGEN_FUNC eigen_handler_mp
    
#elif defined(HYBRID)
    #define ADD_FUNC add_matrices_handler_mp   // you can later make hybrid add/sub if desired
    #define SUB_FUNC subtract_matrices_handler_mp
    #define MUL_FUNC multiply_handler_hybrid
    #define DET_FUNC determinant_handler_hybrid
    #define EIGEN_FUNC eigen_handler_hybrid
#else
    #define ADD_FUNC add_matrices_handler
    #define SUB_FUNC subtract_matrices_handler
    #define MUL_FUNC multiply_matrices_handler
    #define DET_FUNC determinant_handler
    #define EIGEN_FUNC eigen_handler
#endif


ChildProcess *child_pool = NULL; // dynamic array for the pool
int child_count = 0;             // total children in the pool

// flag to prevent the child process from forking
int IS_CHILD = 0;

// void setup_aging_timer(void) {
//     sigset(SIGALRM, aging_signal_handler);  // simple handler setup
//     alarm(30);                              // first alarm in 30 sec
//     printf("Aging timer set: checks every 30 seconds (sigset version)\n");
// }

// // Signal handler that re-arms itself
// void aging_signal_handler(int signum) {
//     if (signum == SIGALRM) {
//         aging(120);      // kill idle children older than 120 s
//         alarm(30);       // set next alarm after 30 s
//     }
// }



// Display main menu and get user choice
int display_menu(void) {
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║        Matrix Calculator Menu        ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ 1.  Enter new matrix (manual)        ║\n");
    printf("║ 2.  Display matrix by index          ║\n");
    printf("║ 3.  Delete matrix by index           ║\n");
    printf("║ 4.  Modify matrix                    ║\n");
    printf("║ 5.  Load matrix from file            ║\n");
    printf("║ 6.  Load all matrices from folder    ║\n");
    printf("║ 7.  Save matrix to file (auto-name)  ║\n");
    printf("║ 8.  Save all matrices to folder      ║\n");
    printf("║ 9.  Display all matrices in memory   ║\n");
    printf("║ 10. Add two matrices                 ║\n");
    printf("║ 11. Subtract two matrices            ║\n");
    printf("║ 12. Multiply two matrices            ║\n");
    printf("║ 13. Determinant                      ║\n");
    printf("║ 14. Dominant Eigenvalue & Vector     ║\n");
    printf("║ 15. Exit                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("Enter your choice: ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return -1;
    }
    return choice;
}

double now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
void modify_matrix_handler(void) {
    if (matrix_count == 0) {
        printf("No matrices in memory!\n");
        return;
    }

    printf("\nModify options:\n");
    printf("1. Modify full row\n");
    printf("2. Modify full column\n");
    printf("3. Modify a particular value\n");
    printf("Enter your choice (1-3): ");
    
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        printf("Invalid choice!\n");
        while(getchar() != '\n');
        return;
    }

    display_all_matrices();
    int index;
    printf("Enter the index of the matrix to modify: ");
    if (scanf("%d", &index) != 1 || index < 0 || index >= matrix_count) {
        printf("Invalid index!\n");
        while(getchar() != '\n');
        return;
    }

    Matrix *m = &matrices[index];  

    if (choice == 1) {
        int row;
        printf("Enter row number (0 to %d): ", m->rows - 1);
        if (scanf("%d", &row) != 1 || row < 0 || row >= m->rows) {
            printf("Invalid row!\n");
            while(getchar() != '\n');
            return;
        }
        printf("Enter %d new values for row %d:\n", m->cols, row);
        for (int j = 0; j < m->cols; j++) {
            printf("   [%d][%d]: ", row, j);
            scanf("%lf", &m->data[row][j]);
        }
        printf("Row %d of Matrix #%d modified successfully!\n", row, index);
    }
    else if (choice == 2) {
        int col;
        printf("Enter column number (0 to %d): ", m->cols - 1);
        if (scanf("%d", &col) != 1 || col < 0 || col >= m->cols) {
            printf("Invalid column!\n");
            while(getchar() != '\n');
            return;
        }
        printf("Enter %d new values for column %d:\n", m->rows, col);
        for (int i = 0; i < m->rows; i++) {
            printf("   [%d][%d]: ", i, col);
            scanf("%lf", &m->data[i][col]);
        }
        printf("Column %d of Matrix #%d modified successfully!\n", col, index);
    }
    else if (choice == 3) {
        int row, col;
        printf("Enter row (0 to %d): ", m->rows - 1);
        if (scanf("%d", &row) != 1 || row < 0 || row >= m->rows) {
            printf("Invalid row!\n");
            while(getchar() != '\n');
            return;
        }
        printf("Enter column (0 to %d): ", m->cols - 1);
        if (scanf("%d", &col) != 1 || col < 0 || col >= m->cols) {
            printf("Invalid column!\n");
            while(getchar() != '\n');
            return;
        }
        printf("Current value: %.6f\n", m->data[row][col]);
        printf("Enter new value for [%d][%d]: ", row, col);
        scanf("%lf", &m->data[row][col]);
        printf("Element [%d][%d] updated successfully!\n", row, col);
    }

    printf("\nUpdated matrix:\n");
    display_matrix_by_index(index);
}

//Handlers
void enter_matrix_handler(void) {
    printf("Manual matrix entry selected.\n");
    enter_matrix();
}
void display_matrix_handler(void) {
    if (matrix_count == 0) { printf("No matrices in memory.\n"); return; }
    int idx; printf("Enter matrix index to display: "); scanf("%d", &idx);
    display_matrix_by_index(idx);
}
void delete_matrix_handler(void) {
    if (matrix_count == 0) { printf("No matrices to delete.\n"); return; }
    int idx; printf("Enter matrix index to delete: "); scanf("%d", &idx);
    delete_matrix_by_index(idx);
}
void display_all_matrices_handler(void) { display_all_matrices(); }

void add_matrices_handler(void) {
    printf("\n--- Matrix Addition ---\n");
    Matrix A = get_matrix_from_user("A");
    if (!A.data) return;
    Matrix B = get_matrix_from_user("B");
    if (!B.data) { free_matrix_data(A); return; }
    Matrix R = matrix_add(A, B);
    if (R.data) {
        add_result_to_memory(R);
        printf("Addition successful! Result stored as Matrix #%d\n", matrix_count - 1);
        display_matrix_by_index(matrix_count - 1);
    } else printf("Addition failed (dimension mismatch).\n");
    free_matrix_data(A); free_matrix_data(B);
}
void subtract_matrices_handler(void) {
    printf("\n--- Matrix Subtraction ---\n");
    Matrix A = get_matrix_from_user("A");
    if (!A.data) return;
    Matrix B = get_matrix_from_user("B");
    if (!B.data) { free_matrix_data(A); return; }
    Matrix R = matrix_subtract(A, B);
    if (R.data) {
        add_result_to_memory(R);
        printf("Subtraction successful! Result stored as Matrix #%d\n", matrix_count - 1);
        display_matrix_by_index(matrix_count - 1);
    }
    free_matrix_data(A); free_matrix_data(B);
}
// ========== MODIFIED: Matrix Addition Handler using Multiprocessing ==========
void add_matrices_handler_mp(void)
{
    printf("\n--- Matrix Addition (Multiprocessing) ---\n");
    // Get matrix A from user (either from memory or file)
    Matrix A = get_matrix_from_user("A");
    if (!A.data)
        return;

    // Get matrix B from user (either from memory or file)
    Matrix B = get_matrix_from_user("B");
    if (!B.data)
    {
        free_matrix_data(A);
        return;
    }

    // Perform addition using multiprocessing (one child per row)
    Matrix R = matrix_add_mp(A, B);

    if (R.data)
    {
        // Add result to memory
        add_result_to_memory(R);
        printf("Addition successful! Result stored as Matrix #%d\n", matrix_count - 1);
        printf("(Used %d child processes - one per row)\n", A.rows);
        display_matrix_by_index(matrix_count - 1);
    }
    else
    {
        printf("Addition failed (dimension mismatch).\n");
    }

    // Clean up
    free_matrix_data(A);
    free_matrix_data(B);
}

// ========== MODIFIED: Matrix Subtraction Handler using Multiprocessing ==========
void subtract_matrices_handler_mp(void)
{
    printf("\n--- Matrix Subtraction (Multiprocessing) ---\n");
    // Get matrix A from user (either from memory or file)
    Matrix A = get_matrix_from_user("A");
    if (!A.data)
        return;

    // Get matrix B from user (either from memory or file)
    Matrix B = get_matrix_from_user("B");
    if (!B.data)
    {
        free_matrix_data(A);
        return;
    }

    // Perform subtraction using multiprocessing (one child per row)
    Matrix R = matrix_subtract_mp(A, B);

    if (R.data)
    {
        // Add result to memory
        add_result_to_memory(R);
        printf("Subtraction successful! Result stored as Matrix #%d\n", matrix_count - 1);
        printf("(Used %d child processes - one per row)\n", A.rows);
        display_matrix_by_index(matrix_count - 1);
    }
    else
    {
        printf("Subtraction failed (dimension mismatch).\n");
    }

    // Clean up
    free_matrix_data(A);
    free_matrix_data(B);
}

void multiply_matrices_handler(void) {
    printf("\n--- Matrix Multiplication ---\n");
    Matrix A = get_matrix_from_user("A");
    if (!A.data) return;
    Matrix B = get_matrix_from_user("B");
    if (!B.data) { free_matrix_data(A); return; }
    Matrix R = matrix_multiply(A, B);
    if (R.data) {
        add_result_to_memory(R);
        printf("Multiplication successful! Result stored as Matrix #%d\n", matrix_count - 1);
        display_matrix_by_index(matrix_count - 1);
    }
    free_matrix_data(A); free_matrix_data(B);
}

//---------------------------------------------------------------------------
// Multiprocessing handler for multiplication
void multiply_matrices_handler_mp(void)
{
    Matrix x = get_matrix_from_user("x");
    if (!x.data)
        return;
    Matrix y = get_matrix_from_user("y");
    if (!y.data)
    {
        free_matrix_data(x);
        return;
    }
    Matrix z = matrix_multiply_mp(x, y);
    if (z.data)
    {
        add_result_to_memory(z);
        display_matrix_by_index(matrix_count - 1);
    }
    free_matrix_data(x);
    free_matrix_data(y);
}

void multiply_handler_hybrid(void)
{
    Matrix x = get_matrix_from_user("x");
    if (!x.data)
        return;
    Matrix y = get_matrix_from_user("y");
    if (!y.data)
    {
        free_matrix_data(x);
        return;
    }
    Matrix z = matrix_multiply_hybrid(x, y);
    if (z.data)
    {
        add_result_to_memory(z);
        display_matrix_by_index(matrix_count - 1);
    }
    free_matrix_data(x);
    free_matrix_data(y);
}

//_________________________________________________________________________________________

void determinant_handler(void) {
    printf("\n--- Determinant Calculation ---\n");
    Matrix M = get_matrix_from_user("M");
    if (!M.data) return;
    if (M.rows != M.cols) printf("Only square matrices have determinants.\n");
    else {
        double det = matrix_determinant(M);
        printf("Determinant = %.10f\n", det);
    }
    free_matrix_data(M);
}
//-------------------------------------------------------------------------
void determinant_handler_mp(void)
{
    printf("\n--- Determinant Calculation (Multiprocessing) ---\n");
    Matrix M = get_matrix_from_user("M");
    if (!M.data)
        return;
    if (M.rows != M.cols)
        printf("Only square matrices have determinants.\n");
    else
    {
        double det = matrix_determinant_mp(M);
        printf("Determinant = %.10f\n", det);
    }
    free_matrix_data(M);
}


void eigen_handler(void) {
    printf("\n--- Dominant Eigenvalue & Eigenvector (Power Method) ---\n");
    Matrix M = get_matrix_from_user("M");
    if (!M.data) return;
    if (M.rows != M.cols) { printf("Eigen requires square matrix.\n"); free_matrix_data(M); return; }
    EigenResult res = matrix_eigen(M);
    if (res.count > 0) {
        printf("Dominant Eigenvalue: %.10f\n", res.eigenvalues[0]);
        printf("Eigenvector (normalized):\n");
        Matrix vec = res.eigenvectors[0];
        for (int i = 0; i < vec.rows; i++)
            printf(" [ %.10f ]\n", vec.data[i][0]);
        free_matrix_data(vec);
        free(res.eigenvalues);
        free(res.eigenvectors);
    } else printf("Eigen calculation failed.\n");
    free_matrix_data(M);
}

void eigen_handler_mp(void)
{
    printf("\n--- Dominant Eigenvalue & Eigenvector (Power Method) ---\n");
    Matrix M = get_matrix_from_user("M");
    if (!M.data)
        return;
    if (M.rows != M.cols)
    {
        printf("Eigen requires square matrix.\n");
        free_matrix_data(M);
        return;
    }
    EigenResult res = matrix_eigen_mp(M);
    if (res.count > 0)
    {
        printf("Dominant Eigenvalue: %.10f\n", res.eigenvalues[0]);
        printf("Eigenvector (normalized):\n");
        Matrix vec = res.eigenvectors[0];
        for (int i = 0; i < vec.rows; i++)
            printf(" [ %.10f ]\n", vec.data[i][0]);
        free_matrix_data(vec);
        free(res.eigenvalues);
        free(res.eigenvectors);
    }
    else
        printf("Eigen calculation failed.\n");
    free_matrix_data(M);
}
void eigen_handler_hybrid(void)
{
    printf("\n--- Dominant Eigenvalue & Eigenvector (Power Method) ---\n");
    Matrix M = get_matrix_from_user("M");
    if (!M.data)
        return;
    if (M.rows != M.cols)
    {
        printf("Eigen requires square matrix.\n");
        free_matrix_data(M);
        return;
    }
    EigenResult res = matrix_eigen_hybrid(M);
    if (res.count > 0)
    {
        printf("Dominant Eigenvalue: %.10f\n", res.eigenvalues[0]);
        printf("Eigenvector (normalized):\n");
        Matrix vec = res.eigenvectors[0];
        for (int i = 0; i < vec.rows; i++)
            printf(" [ %.10f ]\n", vec.data[i][0]);
        free_matrix_data(vec);
        free(res.eigenvalues);
        free(res.eigenvectors);
    }
    else
        printf("Eigen calculation failed.\n");
    free_matrix_data(M);
}



//////////////////////////////openMP handlers
void determinant_handler_hybrid(void) {
    printf("\n--- Determinant Calculation (Hybrid: Multiprocessing + OpenMP) ---\n");
    Matrix M = get_matrix_from_user("M");
    if (!M.data) return;
    if (M.rows != M.cols) printf("Only square matrices have determinants.\n");
    else {
        double det = matrix_determinant_hybrid(M);
        printf("Determinant = %.10f\n", det);
    }
    free_matrix_data(M);
}

int main(void) {
    
    system("mkdir -p matrices");
    printf("Matrix Calculator started!\n");
    printf("All results are automatically saved in 'matrices/' folder.\n\n");
       
    int choice;
    do { 
         aging(120);
        choice = display_menu();
        switch (choice) {
            case 1:{ enter_matrix_handler();           break;}
            case 2:  display_matrix_handler();         break;
            case 3:  delete_matrix_handler();          break;
            case 4:  modify_matrix_handler();          break;  
            case 5:  {
                        char path[256];
                        printf("Enter file path: ");
                        scanf("%255s", path);
                        Matrix m = read_matrix_from_file(path);
                        if (m.data) {
                            add_result_to_memory(m);
                            printf("Loaded and stored as Matrix #%d\n", matrix_count - 1);
                        }
                     } break;
            case 6:  display_folder();                 break;
            case 7:  enter_matrix_to_file();           break;
            case 8:  save_all_matrices_to_folder();    break;
            case 9:  display_all_matrices_handler();   break;

case 10: {
    double start = now_ms();
    ADD_FUNC();  // chosen by #define above
    double end = now_ms();
    printf("Addition time: %.3f ms\n", end - start);
    break;
}
        case 11: {
    double start = now_ms();
    SUB_FUNC();
    double end = now_ms();
    printf("Subtraction time: %.3f ms\n", end - start);
    break;
        }
            case 12: {
    double start = now_ms();
    MUL_FUNC();
    double end = now_ms();
    printf("Multiplication time: %.3f ms\n", end - start);
    break;
            }
            case 13: {
    double start = now_ms();
    DET_FUNC();
    double end = now_ms();
    printf("Determinant time: %.3f ms\n", end - start);
    break;
            }

            case 14: {double start = now_ms();
                EIGEN_FUNC();   
                double end = now_ms();   
                printf("eigenValue/vector time: %.3f ms\n", end - start);            
                 break;}
            case 15:
                printf("Thank you for using Matrix Calculator. Goodbye!\n");
                break;
            default:
                if (choice != -1) printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 15);
    
    free_all_global_matrices();
        // Kill all child processes before exiting
for (int k = 0; k < child_count; k++) {
    close(child_pool[k].fd_p_to_c[1]);
    close(child_pool[k].fd_c_to_p[0]);
    kill(child_pool[k].pid, SIGTERM);
}
    return 0;
}
