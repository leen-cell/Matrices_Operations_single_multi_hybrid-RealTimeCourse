#include "matrix_calc.h"

// Helper: extract cofactor
static void get_cofactor(double **M, int n, double **temp, int p, int q) {
    int i = 0, j = 0;
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            if (row != p && col != q) {
                temp[i][j++] = M[row][col];
                if (j == n - 1) { j = 0; i++; }
            }
        }
    }
}

// Recursive determinant using cofactor expansion (first layer only)
static double det_recursive(double **M, int n) {
    if (n == 1) return M[0][0];
    if (n == 2) return M[0][0] * M[1][1] - M[0][1] * M[1][0];

    double D = 0.0;
    int sign = 1;

    for (int f = 0; f < n; f++) {
        // Allocate a fresh minor for this cofactor
        double **minor = malloc((n - 1) * sizeof(double *));
        for (int i = 0; i < n - 1; i++)
            minor[i] = malloc((n - 1) * sizeof(double));

        // Get cofactor matrix
        get_cofactor(M, n, minor, 0, f);

        // Compute determinant of the minor using Gaussian elimination
        double subdet = determinant_gaussian(minor, n - 1);

        // Add contribution
        D += sign * M[0][f] * subdet;
        sign = -sign;

        // Free memory for this minor
        for (int i = 0; i < n - 1; i++) free(minor[i]);
        free(minor);
    }

    return D;
}

// Public determinant function
double matrix_determinant(Matrix M) {
    if (M.rows != M.cols) {
        printf("Error: Determinant requires square matrix.\n");
        return 0.0;
    }
    if (!M.data) {
        printf("Error: Matrix data is NULL.\n");
        return 0.0;
    }
    return det_recursive(M.data, M.rows);
}

// Addition
Matrix matrix_add(Matrix A, Matrix B) {
    Matrix C = {0, 0, NULL};
    if (A.rows != B.rows || A.cols != B.cols) {
        printf("Error: Matrices must have same dimensions for addition.\n");
        return C;
    }
    C = allocate_matrix(A.rows, A.cols);
    for (int i = 0; i < A.rows; i++)
        for (int j = 0; j < A.cols; j++)
            C.data[i][j] = A.data[i][j] + B.data[i][j];
    return C;
}

// Subtraction
Matrix matrix_subtract(Matrix A, Matrix B) {
    Matrix C = {0, 0, NULL};
    if (A.rows != B.rows || A.cols != B.cols) {
        printf("Error: Matrices must have same dimensions for subtraction.\n");
        return C;
    }
    C = allocate_matrix(A.rows, A.cols);
    for (int i = 0; i < A.rows; i++)
        for (int j = 0; j < A.cols; j++)
            C.data[i][j] = A.data[i][j] - B.data[i][j];
    return C;
}

// Multiplication
Matrix matrix_multiply(Matrix A, Matrix B) {
    Matrix C = {0, 0, NULL};
    if (A.cols != B.rows) {
        printf("Error: A.cols must equal B.rows for multiplication.\n");
        return C;
    }
    C = allocate_matrix(A.rows, B.cols);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            C.data[i][j] = 0.0;
            for (int k = 0; k < A.cols; k++)
                C.data[i][j] += A.data[i][k] * B.data[k][j];
        }
    }
    return C;
}

// Power method for dominant eigenvalue/eigenvector
EigenResult matrix_eigen(Matrix M) {
    EigenResult result = {NULL, NULL, 0};
	//if symmetric matrix
    if (M.rows != M.cols || !M.data) {
        printf("Error: Eigen requires square matrix with valid data.\n");
        return result;
    }

    int n = M.rows;
    double tol = 1e-10;
    int max_iter = 1000;

    Matrix x = allocate_matrix(n, 1);
    srand(time(NULL));//Using time as a seed for the random function

    double norm = 0.0;
    for(int i = 0; i < n; i++){
    	x.data[i][0] = (double)rand() / RAND_MAX;
    	norm += x.data[i][0] * x.data[i][0];
    }

    norm = sqrt(norm);
    for (int i = 0; i < n; i++){
    	x.data[i][0] /= norm; // |x| = 1
    }

    double lambda =0.0;
    double lambda_old = 0.0;

    printf("Using Power Method....\n");

    for(int iter =0 ; iter < max_iter; iter++){
    	// y = M * x
    	Matrix y = matrix_multiply(M,x);

		//Calculate lambda 
		lambda = 0.0;
		for(int i = 0; i < n; i++){
			lambda += x.data[i][0] * y.data[i][0];
		}
		
    	norm = 0.0;
    	for (int i = 0; i< n; i++){
    		norm += y.data[i][0] * y.data[i][0];
    	}
    	norm = sqrt(norm);

		//stop if norm near zero
    	if(norm < 1e-15){
    		printf("ERROR: Vector collapsed!\n");
    		free_matrix_data(x);
    		free_matrix_data(y);
    		return result;
    	}

    	for(int i = 0; i < n; i++){
    		x.data[i][0] = y.data[i][0] / norm;
    	}
		free_matrix_data(y);

		//if change stops
		if(fabs(lambda - lambda_old) < tol){
			printf("Converged after %d iterations!\n", iter + 1);
			break;
		}
		lambda_old = lambda;
		
	}
		result.count = 1;
		result.eigenvalues = malloc(sizeof(double));
		result.eigenvectors = malloc(sizeof(Matrix));
		result.eigenvalues[0] = lambda;
		result.eigenvectors[0] = x;

		return result;
    	
}

double determinant_gaussian(double **A, int n) {
    double det = 1.0;
    for (int i = 0; i < n; i++) {
        // Pivot if diagonal element is zero
        if (A[i][i] == 0) {
            int swap_row = i + 1;
            while (swap_row < n && A[swap_row][i] == 0) swap_row++;
            if (swap_row == n) return 0.0; 
            double *temp = A[i];
            A[i] = A[swap_row];
            A[swap_row] = temp;
            det *= -1; 
        }

        det *= A[i][i];
        #pragma omp parallel for schedule(dynamic)
        for (int j = i + 1; j < n; j++) {
            double factor = A[j][i] / A[i][i];
            for (int k = i; k < n; k++) {
                A[j][k] -= factor * A[i][k];
            }
        }
    }
    return det;
}