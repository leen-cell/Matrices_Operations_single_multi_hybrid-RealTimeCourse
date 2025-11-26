
#include "matrix_multi.h"
// to create new child processes and add them to the pool array
pid_t create_child()
{
    // grow pool
    child_pool = realloc(child_pool, (child_count + 1) * sizeof(ChildProcess));
    // couldn't realloc the size
    if (!child_pool)
    {
        perror("realloc failed");
        exit(1);
    }

    ChildProcess *c = &child_pool[child_count];
    // create the pipes for this child
    if (pipe(c->fd_p_to_c) < 0 || pipe(c->fd_c_to_p) < 0)
    {
        perror("pipe failed");
        return -1;
    }

    // create child process
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        return -1;
    }
    else if (pid == 0)
    {                 // we are in the child process
        IS_CHILD = 1; // mark that this process is a child
        // child reads only from p to c and only writes to c to p
        close(c->fd_p_to_c[1]);
        close(c->fd_c_to_p[0]);
        // after closing the not necessary pipes the child will wait until it is asked to do something

        // initially the child is created to do specific thing, when releasing it we can remove this command to make the child general again
        while (1)
        {
            ChildCommand cmd;
            // since the pipe reads pipes
            // the parent will send a complete struct with all data needed by the child
            ssize_t bytesRead = read(c->fd_p_to_c[0], &cmd, sizeof(cmd));
            if (bytesRead <= 0)
            {
                perror("child read failed or pipe closed");
                _exit(1);
            }

            // if the parent asked the child to stop
            if (cmd.cmd == CMD_EXIT)
            {
                _exit(0);
            }

            else if (cmd.cmd == CMD_DETERMINANT)
            {

                int n = cmd.size; // the size sent by parent

                double **minor = malloc(n * sizeof(double *)); // submatrix
                for (int i = 0; i < n; i++)
                {
                    minor[i] = malloc(n * sizeof(double));
                    read(c->fd_p_to_c[0], minor[i], sizeof(double) * n);
                }

                double det = determinant_gaussian(minor, n); 
                double result = cmd.args.det_args.sign * cmd.args.det_args.factor * det;

                write(c->fd_c_to_p[1], &result, sizeof(double));

                for (int i = 0; i < n; i++)
                    free(minor[i]);
                free(minor);
                usleep(1000);
            }
            
            // This child will add one row from matrix A with one row from matrix B
            else if (cmd.cmd == CMD_ADD)
            {
                int row = cmd.args.arith_args.row;   // which row number we're processing
                int cols = cmd.args.arith_args.cols; // how many columns in each row

                // Read row from matrix A
                double *rowA = malloc(cols * sizeof(double));
                read(c->fd_p_to_c[0], rowA, sizeof(double) * cols);

                // Read row from matrix B
                double *rowB = malloc(cols * sizeof(double));
                read(c->fd_p_to_c[0], rowB, sizeof(double) * cols);

                // Calculate result row (A + B)
                // For each element in the row: result[j] = A[j] + B[j]
                double *result_row = malloc(cols * sizeof(double));
                for (int j = 0; j < cols; j++)
                {
                    result_row[j] = rowA[j] + rowB[j];
                }

                // Send result back to parent through the pipe
                write(c->fd_c_to_p[1], result_row, sizeof(double) * cols);

                // Clean up memory
                free(rowA);
                free(rowB);
                free(result_row);
                usleep(1000);
            }
            
            // This child will subtract one row of matrix B from one row of matrix A
            else if (cmd.cmd == CMD_SUBTRACT)
            {
                int row = cmd.args.arith_args.row;   // which row number we're processing
                int cols = cmd.args.arith_args.cols; // how many columns in each row

                // Read row from matrix A
                double *rowA = malloc(cols * sizeof(double));
                read(c->fd_p_to_c[0], rowA, sizeof(double) * cols);

                // Read row from matrix B
                double *rowB = malloc(cols * sizeof(double));
                read(c->fd_p_to_c[0], rowB, sizeof(double) * cols);

                // Calculate result row (A - B)
                // For each element in the row: result[j] = A[j] - B[j]
                double *result_row = malloc(cols * sizeof(double));
                for (int j = 0; j < cols; j++)
                {
                    result_row[j] = rowA[j] - rowB[j];
                }

                // Send result back to parent through the pipe
                write(c->fd_c_to_p[1], result_row, sizeof(double) * cols);

                // Clean up memory
                free(rowA);
                free(rowB);
                free(result_row);
                usleep(1000);
            }

            // add for multiplication, etc. ................
            else if (cmd.cmd == CMD_MUL)
            {
                // define the size;
                int size = cmd.size;
                double *x_row = malloc(size * sizeof(double));
                read(c->fd_p_to_c[0], x_row, sizeof(double) * size);

                int y_rows;
                int y_cols;
                // the child must read the second matrix.
                read(c->fd_p_to_c[0], &y_rows, sizeof(int));
                read(c->fd_p_to_c[0], &y_cols, sizeof(int));
                // creat an empty matrix for the 2'nd mat. receved from the parent.
                Matrix y = empty_mt(y_rows, y_cols);
                for (size_t i = 0; i < y_rows; i++)
                {
                    /* code */
                    // read matrix data from the parent
                    read(c->fd_p_to_c[0], y.data[i], sizeof(double) * y_cols);
                }
                // alocate resultant matrix
                double *z_row = malloc(y_cols * sizeof(double));
                for (int j = 0; j < y_cols; j++)
                {
                    z_row[j] = 0.0;
                    for (int k = 0; k < size; k++)
                        z_row[j] += x_row[k] * y.data[k][j];
                }
                // now the child sends the result back to his parent
                write(c->fd_c_to_p[1], z_row, sizeof(double) * y_cols);
                free(x_row); // the child doesn't need it any more
                for (int i = 0; i < y_rows; i++)
                    free(y.data[i]); // to releas each 2'nd mat. row.
                free(y.data);        // releas the 2'nd mat. pointer.
                free(z_row);         // releas the resultant mat. since the child sends its result to his father
                usleep(1000);
            }

        else if (cmd.cmd == CMD_EIGEN)
                {
    int row = cmd.args.eigen_args.row;
    int n   = cmd.args.eigen_args.n;

    // Read one vector x[n]
    double *x = malloc(n * sizeof(double));
    ssize_t r = read(c->fd_p_to_c[0], x, sizeof(double) * n);
    if (r != sizeof(double) * n) {
        perror("child eigen read x");
        free(x);
        continue;
    }

    // Read this child's row of the matrix M
    double *Mrow = malloc(n * sizeof(double));
    r = read(c->fd_p_to_c[0], Mrow, sizeof(double) * n);
    if (r != sizeof(double) * n) {
        perror("child eigen read Mrow");
        free(x); free(Mrow);
        continue;
    }

    // Compute one output value y[row] = Mrow ⋅ x
    double y_row = 0.0;
    for (int k = 0; k < n; k++)
        y_row += Mrow[k] * x[k];

    // Send result back
    write(c->fd_c_to_p[1], &y_row, sizeof(double));

    free(Mrow);
    free(x);
    usleep(1000);
        }

        }

} else
    { // if I am in the parent process

        // this is the pid of the child process since we are in the parent process
        c->pid = pid;
        c->last_used = time(NULL);
        c->busy = 0;
        // parent closes the ends it doesn't need
        close(c->fd_p_to_c[0]);
        close(c->fd_c_to_p[1]);
        child_count++;
        printf("Created child process PID=%d parent is process =%d\n", pid, getpid());
        return pid;
    }
}

// find a free child or create a new one if no available ones.
int get_free_child()
{
    for (int i = 0; i < child_count; i++)
    {
        if (child_pool[i].busy == 0)
        {
            // say that the child is now not available
            child_pool[i].busy = 1;
            child_pool[i].last_used = time(NULL);
            // return this child since it was found as free
            return i;
        }
    }
    // if not returned then none is free
    // none is free then create a new child
    pid_t pid = create_child();
    if (pid < 1)
        return -1;

    child_pool[child_count - 1].busy = 1;
    return child_count - 1;
}

// this function is used when the function is not busy anymore
// we should free it so it can be used again
void release_child(pid_t pid)
{
    for (int i = 0; i < child_count; i++)
    {
        if (child_pool[i].pid == pid)
        {
            child_pool[i].busy = 0;
            child_pool[i].last_used = time(NULL);
            return;
        }
    }
}

void aging(int max_age)
{
    time_t now = time(NULL);
    // difftime is a function to calculate the period given two times
    for (int i = 0; i < child_count; i++)
    {
        if (child_pool[i].busy == 0  && (difftime(now, child_pool[i].last_used) > max_age))
        {
            // SIGTERM is to kill the process in a polite way
            kill(child_pool[i].pid, SIGTERM);
            // printf("Killed idle child PID=%d\n", child_pool[i].pid);
            // we now need to remove the child by shifting the array
            for (int j = i; j < child_count; j++)
            {
                child_pool[j] = child_pool[j + 1];
            }
            child_count--;
            child_pool = realloc(child_pool, (child_count) * sizeof(ChildProcess));
            // since we removed an element i should be less by 1
            i--; // we moved a step backward
        }
    }
}

// Helper: extract cofactor
 void get_cofactor_mp(double **M, int n, double **temp, int p, int q)
{
    int i = 0, j = 0;
    for (int row = 0; row < n; row++)
    {
        for (int col = 0; col < n; col++)
        {
            if (row != p && col != q)
            {
                temp[i][j++] = M[row][col];
                if (j == n - 1)
                {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

// Recursive determinant using cofactor expansion
 double det_recursive_mp(double **M, int n)
{
    if (n == 1)
        return M[0][0];
    if (n == 2)
        return M[0][0] * M[1][1] - M[0][1] * M[1][0];

    double result = 0.0;
    int child_ids[n];
    if (IS_CHILD)
    {
        // Children compute determinant locally without using pool
        // so that children do not fork causing high overhead in speed and memory
        return determinant_gaussian(M, n);
    }

    for (int f = 0; f < n; f++)
    {
        double **minor = malloc((n - 1) * sizeof(double *));
        for (int i = 0; i < n - 1; i++)
        {
            minor[i] = malloc((n - 1) * sizeof(double));
        }

        get_cofactor_mp(M, n, minor, 0, f);

        int child_index = get_free_child();
        ChildProcess *child = &child_pool[child_index];

        // telling the child all the data about the matrix
        ChildCommand cmd;
        cmd.cmd = CMD_DETERMINANT;
        // removed a row and a col so less by 1
        cmd.size = n - 1;
        // alternative negative sign
        cmd.args.det_args.sign = (f % 2 == 0) ? 1 : -1;
        // first row of the matrix
        cmd.args.det_args.factor = M[0][f];
        // write the data the child needs to the pipe

        write(child->fd_p_to_c[1], &cmd, sizeof(cmd));

        for (int i = 0; i < n - 1; i++)
        {
            write(child->fd_p_to_c[1], minor[i], sizeof(double) * (n - 1));
            free(minor[i]);
        }
        child_ids[f] = child_index;
        free(minor);
    }

    for (int f = 0; f < n; f++)
    {
        ChildProcess *child = &child_pool[child_ids[f]]; // get the child processes that the parent used
        double partial = 0.0;
        // read from children child by child that were called by the parent

        ssize_t r = read(child->fd_c_to_p[0], &partial, sizeof(double));
        if (r != sizeof(double))
        {
            perror("Parent read failed from child");
        }
        result += partial;
        release_child(child->pid);
    }

    return result;
}

 double det_recursive_serial(double **M, int n)
{
    if (n == 1)
        return M[0][0];
    if (n == 2)
        return M[0][0] * M[1][1] - M[0][1] * M[1][0];
    double D = 0;
    double **temp = malloc((n - 1) * sizeof(double *));
    for (int i = 0; i < n - 1; i++)
        temp[i] = malloc((n - 1) * sizeof(double));
    int sign = 1;
    for (int f = 0; f < n; f++)
    {
        get_cofactor_mp(M, n, temp, 0, f);
        D += sign * M[0][f] * det_recursive_serial(temp, n - 1);
        sign = -sign;
    }
    for (int i = 0; i < n - 1; i++)
        free(temp[i]);
    free(temp);
    return D;
}

// Public determinant function
double matrix_determinant_mp(Matrix M)
{
    if (M.rows != M.cols)
    {
        printf("Error: Determinant requires square matrix.\n");
        return 0.0;
    }
    if (!M.data)
    {
        printf("Error: Matrix data is NULL.\n");
        return 0.0;
    }
    return det_recursive_mp(M.data, M.rows);
}

// ========== NEW: Multiprocessing Matrix Addition Function ==========
// This function adds two matrices using child processes
// Each child process handles one complete row of the result matrix
Matrix matrix_add_mp(Matrix A, Matrix B)
{
    Matrix result = {0, 0, NULL};

    // Check if matrices have the same dimensions (required for addition)
    if (A.rows != B.rows || A.cols != B.cols)
    {
        printf("Error: Matrices must have same dimensions for addition.\n");
        return result;
    }

    // Allocate memory for the result matrix
    result.rows = A.rows;
    result.cols = A.cols;
    result.data = malloc(result.rows * sizeof(double *));
    for (int i = 0; i < result.rows; i++)
    {
        result.data[i] = malloc(result.cols * sizeof(double));
    }

    // Array to keep track of which child handles which row
    int child_ids[A.rows];

    // Spawn a child process for each row
    // For a 3x3 matrix, this creates 3 children
    // For a 100x100 matrix, this creates 100 children
    for (int i = 0; i < A.rows; i++)
    {
        // Get a free child from the pool (or create a new one)
        int child_index = get_free_child();
        if (child_index < 0)
        {
            printf("Error: Could not get free child!\n");
            free_matrix_data(result);
            result.data = NULL;
            return result;
        }

        ChildProcess *child = &child_pool[child_index];

        // Prepare the command for the child
        ChildCommand cmd;
        cmd.cmd = CMD_ADD;                 // Tell child to do addition
        cmd.args.arith_args.row = i;       // Which row to process
        cmd.args.arith_args.cols = A.cols; // How many columns

        // Send command to child through pipe
        write(child->fd_p_to_c[1], &cmd, sizeof(cmd));

        // Send row i from matrix A to the child
        write(child->fd_p_to_c[1], A.data[i], sizeof(double) * A.cols);

        // Send row i from matrix B to the child
        write(child->fd_p_to_c[1], B.data[i], sizeof(double) * B.cols);

        // Remember which child is handling this row
        child_ids[i] = child_index;
    }

    // Now collect results from all children
    // Each child will send back one complete row of the result
    for (int i = 0; i < A.rows; i++)
    {
        ChildProcess *child = &child_pool[child_ids[i]];

        // Read the result row from the child through the pipe
        ssize_t bytes_read = read(child->fd_c_to_p[0], result.data[i], sizeof(double) * A.cols);
        if (bytes_read != sizeof(double) * A.cols)
        {
            printf("Error: Failed to read result from child for row %d\n", i);
        }

        // Release the child back to the pool so it can be reused
        release_child(child->pid);
    }

    return result;
}

// ========== NEW: Multiprocessing Matrix Subtraction Function ==========
// This function subtracts matrix B from matrix A using child processes
// Each child process handles one complete row of the result matrix
Matrix matrix_subtract_mp(Matrix A, Matrix B)
{
    Matrix result = {0, 0, NULL};

    // Check if matrices have the same dimensions (required for subtraction)
    if (A.rows != B.rows || A.cols != B.cols)
    {
        printf("Error: Matrices must have same dimensions for subtraction.\n");
        return result;
    }

    // Allocate memory for the result matrix
    result.rows = A.rows;
    result.cols = A.cols;
    result.data = malloc(result.rows * sizeof(double *));
    for (int i = 0; i < result.rows; i++)
    {
        result.data[i] = malloc(result.cols * sizeof(double));
    }

    // Array to keep track of which child handles which row
    int child_ids[A.rows];

    // Spawn a child process for each row
    // For a 3x3 matrix, this creates 3 children
    // For a 100x100 matrix, this creates 100 children
    for (int i = 0; i < A.rows; i++)
    {
        // Get a free child from the pool (or create a new one)
        int child_index = get_free_child();
        if (child_index < 0)
        {
            printf("Error: Could not get free child!\n");
            free_matrix_data(result);
            result.data = NULL;
            return result;
        }

        ChildProcess *child = &child_pool[child_index];

        // Prepare the command for the child
        ChildCommand cmd;
        cmd.cmd = CMD_SUBTRACT;            // Tell child to do subtraction
        cmd.args.arith_args.row = i;       // Which row to process
        cmd.args.arith_args.cols = A.cols; // How many columns

        // Send command to child through pipe
        write(child->fd_p_to_c[1], &cmd, sizeof(cmd));

        // Send row i from matrix A to the child
        write(child->fd_p_to_c[1], A.data[i], sizeof(double) * A.cols);

        // Send row i from matrix B to the child
        write(child->fd_p_to_c[1], B.data[i], sizeof(double) * B.cols);

        // Remember which child is handling this row
        child_ids[i] = child_index;
    }

    // Now collect results from all children
    // Each child will send back one complete row of the result
    for (int i = 0; i < A.rows; i++)
    {
        ChildProcess *child = &child_pool[child_ids[i]];

        // Read the result row from the child through the pipe
        ssize_t bytes_read = read(child->fd_c_to_p[0], result.data[i], sizeof(double) * A.cols);
        if (bytes_read != sizeof(double) * A.cols)
        {
            printf("Error: Failed to read result from child for row %d\n", i);
        }

        // Release the child back to the pool so it can be reused
        release_child(child->pid);
    }

    return result;
}


//----------------------------------------------------------------------------------
// creat empty matrix function
Matrix empty_mt(int row, int col)
{
    Matrix z;
    z.rows = row;
    z.cols = col;
    z.data = malloc(row * sizeof(double *));
    if (!z.data)
    {

        perror("malloc faild ...");
        z.rows = z.cols = 0;
        return z;
    }
    for (int i = 0; i < row; i++)
    {
        z.data[i] = malloc(col * sizeof(double));
        if (!z.data[i])
        {
            perror("malloc faild ...");
            // free
            for (int j = 0; j < i; j++)
                free(z.data[j]);
            z.data = NULL;
            z.rows = z.cols = 0;
            return z;
        }
        memset(z.data[i], 0, col * sizeof(double));
    }
    return z;
}
//_________________________________________________________________________
// Multiprocessing - matrix multiplication
Matrix matrix_multiply_mp(Matrix x, Matrix y)
{
    // Matrix Multiplication must be achieved
    if (x.cols != y.rows)
    {
        Matrix z = {0, 0, NULL};
        return z;
    }

    Matrix z = empty_mt(x.rows, y.cols); // result matrix-zero at the begining
    int child_id_array[x.rows];          // #children = #x matrix rows.
    // create the childern
    for (int i = 0; i < x.rows; i++)
    {
        int child_indix = get_free_child();
        //// i am here
        ChildProcess *child = &child_pool[child_indix];
        ChildCommand cmd;
        cmd.cmd = CMD_MUL;
        cmd.size = x.cols;
        // father commands to his children
        write(child->fd_p_to_c[1], &cmd, sizeof(cmd));
        write(child->fd_p_to_c[1], x.data[i], sizeof(double) * x.cols);
        write(child->fd_p_to_c[1], &y.rows, sizeof(int));
        write(child->fd_p_to_c[1], &y.cols, sizeof(int));
        for (int j = 0; j < y.rows; j++)
            write(child->fd_p_to_c[1], y.data[j], sizeof(double) * y.cols);
        child_id_array[i] = child_indix;
    }
    for (int i = 0; i < x.rows; i++)
    {
        ChildProcess *child = &child_pool[child_id_array[i]];
        read(child->fd_c_to_p[0], z.data[i], sizeof(double) * y.cols);
        release_child(child->pid);
    }
    return z;
}
//------------------------------------------------------------
//Eigenfunction multi-process
EigenResult matrix_eigen_mp(Matrix M)
{
    EigenResult result = {NULL, NULL, 0};

    // Input validation
    if (M.rows != M.cols || !M.data) {
        printf("ERROR: Input matrix must be square and valid!\n");
        return result;
    }

    int n = M.rows;

    
    if (child_count < n) {
        printf("Initializing %d worker processes for Power Method...\n", n);
        for (int i = child_count; i < n; i++) {
            create_child();                 // creates one child and adds it to the pool
        }
    }

  
    double tol = 1e-10;
    int max_iter  = 2000;
    double lambda_old = 0.0;

    
    Matrix x = allocate_matrix(n, 1);
    srand(time(NULL) ^ getpid());
    double norm = 0.0;
    for (int i = 0; i < n; i++) {
        x.data[i][0] = (double)rand() / RAND_MAX + 0.3;   // avoid zero entries
        norm += x.data[i][0] * x.data[i][0];
    }
    norm = sqrt(norm);
    for (int i = 0; i < n; i++) x.data[i][0] /= norm;

    printf("\nPower Method (Multiprocessing) - Size %dx%d\n", n, n);
    printf("Using %d child processes\n", child_count);

    
    for (int iter = 0; iter < max_iter; iter++)
    {
        int child_idx[n];
        memset(child_idx, -1, sizeof(child_idx));

        //Send one row-computation task to each child 
        for (int row = 0; row < n; row++)
        {
            int idx = get_free_child();
            child_idx[row] = idx;
            ChildProcess *child = &child_pool[idx];

            // Send command header
            ChildCommand cmd;
             cmd.cmd = CMD_EIGEN;
            cmd.args.eigen_args.row = row;
            cmd.args.eigen_args.n = n;
            write(child->fd_p_to_c[1], &cmd, sizeof(cmd));

            // Send current vector x
            double *xbuf = malloc(sizeof(double) * n);
            for (int i = 0; i < n; i++) xbuf[i] = x.data[i][0];
            write(child->fd_p_to_c[1], xbuf, sizeof(double) * n);

            // Send this child's matrix row
            write(child->fd_p_to_c[1], M.data[row], sizeof(double) * n);
            free(xbuf);
        }


        // Collect results from children
        Matrix y = allocate_matrix(n, 1);
        for (int i = 0; i < n; i++)
        {
            ChildProcess *child = &child_pool[child_idx[i]];
            double row_val;

            ssize_t r = read(child->fd_c_to_p[0], &row_val, sizeof(double));
            if (r == sizeof(double)) {
                y.data[i][0] = row_val;
            } else {
                printf("Warning: Failed to read from child %d\n", child->pid);
                y.data[i][0] = 0.0;
            }

            // return child to the pool (keep it alive!)
            child->busy = 0;
            child->last_used = time(NULL);
        }

        // Compute new eigenvalue 
        double lambda_new = 0.0;
        for (int i = 0; i < n; i++)
            lambda_new += x.data[i][0] * y.data[i][0];

        //Normalize y to get next x 
        norm = 0.0;
        for (int i = 0; i < n; i++) norm += y.data[i][0] * y.data[i][0];
        norm = sqrt(norm);

        
        if (norm < 1e-12) {
            printf("WARNING: Vector collapse detected - reinitializing x...\n");
            srand(time(NULL) ^ getpid() ^ iter);
            norm = 0.0;
            for (int i = 0; i < n; i++) {
                x.data[i][0] = (double)rand() / RAND_MAX + 0.4;
                norm += x.data[i][0] * x.data[i][0];
            }
            norm = sqrt(norm);
            for (int i = 0; i < n; i++) x.data[i][0] /= norm;
            free_matrix_data(y);
            iter = -1;                // restart iteration count
            continue;
        }

        for (int i = 0; i < n; i++) x.data[i][0] = y.data[i][0] / norm;
        free_matrix_data(y);

        //  Convergence test
        if (fabs(lambda_new - lambda_old) < tol) {
            printf("CONVERGED in %d iterations!\n", iter + 1);
            printf("Dominant Eigenvalue = %.10f\n\n", lambda_new);
            lambda_old = lambda_new;
            break;
        }

        lambda_old = lambda_new;

        if ((iter + 1) % 100 == 0)
            printf("Iteration %d  lambda ~ %.10f\n", iter + 1, lambda_new);
    }

    //return result
    result.count = 1;
    result.eigenvalues = malloc(sizeof(double));
    result.eigenvectors = malloc(sizeof(Matrix));
    result.eigenvalues[0] = lambda_old;
    result.eigenvectors[0] = x;          // x is already normalized



    return result;
}