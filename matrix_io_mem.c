// matrix_io_mem.c - COMPLETE AND FINAL VERSION
#include "matrix_io_mem.h"

// Global arrays
Matrix *matrices = NULL;
int matrix_count = 0;

// ======================= MEMORY FUNCTIONS =======================

Matrix allocate_matrix(int rows, int cols)
{
    Matrix m = {rows, cols, NULL};
    m.data = malloc(rows * sizeof(double *));
    if (!m.data)
    {
        perror("malloc rows");
        exit(1);
    }
    for (int i = 0; i < rows; i++)
    {
        m.data[i] = malloc(cols * sizeof(double));
        if (!m.data[i])
        {
            perror("malloc cols");
            exit(1);
        }
    }
    return m;
}

void free_matrix_data(Matrix m)
{
    if (!m.data)
        return;
    for (int i = 0; i < m.rows; i++)
        free(m.data[i]);
    free(m.data);
    m.data = NULL;
}

void free_all_global_matrices(void)
{
    for (int i = 0; i < matrix_count; i++)
        free_matrix_data(matrices[i]);
    free(matrices);
    matrices = NULL;
    matrix_count = 0;
}

void enter_matrix()
{
    Matrix m;
    printf("Number of rows: ");
    if (scanf("%d", &m.rows) != 1 || m.rows <= 0)
    {
        printf("Invalid input!\n");
        while (getchar() != '\n')
            ;
        return;
    }

    printf("Nember of columns: ");
    if (scanf("%d", &m.cols) != 1 || m.cols <= 0)
    {
        while (getchar() != '\n')
            ;
        return;
    }

    m = allocate_matrix(m.rows, m.cols);
    printf("Enter matrix elemnts row-wise\n");
    for (int i = 0; i < m.rows; i++)
    {
        for (int j = 0; j < m.cols; j++)
        {
            printf(" [%d][%d]", i, j);
            if (scanf("%lf", &m.data[i][j]) != 1)
            {
                printf("Invalid input!\nPlease enter a number\n");
                while (getchar() != '\n')
                    ;
                j--;
                continue;
            }
        }
    }
    add_result_to_memory(m);
    printf("\nMatrix enterd successfully! Stored as Matrix %d", matrix_count - 1);
    display_matrix_by_index(matrix_count - 1);
}

void add_result_to_memory(Matrix result)
{
    Matrix *temp = realloc(matrices, (matrix_count + 1) * sizeof(Matrix));
    if (!temp)
    {
        perror("realloc");
        free_matrix_data(result);
        exit(1);
    }
    matrices = temp;
    matrices[matrix_count++] = result;
}

void delete_matrix_by_index(int index)
{
    if (index < 0 || index >= matrix_count)
    {
        printf("Invalid index.\n");
        return;
    }
    free_matrix_data(matrices[index]);
    for (int i = index; i < matrix_count - 1; i++)
        matrices[i] = matrices[i + 1];
    matrix_count--;
    matrices = realloc(matrices, matrix_count * sizeof(Matrix));
}

// ======================= FILE I/O =======================
Matrix read_matrix_from_file(const char *filename)
{
    Matrix m = {0, 0, NULL};
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("fopen");
        return m;
    }

    if (fscanf(fp, "%d %d", &m.rows, &m.cols) != 2 || m.rows <= 0 || m.cols <= 0)
    {
        printf("Invalid dimensions in file '%s'\n", filename);
        fclose(fp);
        return m;
    }

    m = allocate_matrix(m.rows, m.cols);
    for (int i = 0; i < m.rows; i++)
        for (int j = 0; j < m.cols; j++)
            if (fscanf(fp, "%lf", &m.data[i][j]) != 1)
            {
                printf("Error reading element [%d][%d]\n", i, j);
                free_matrix_data(m);
                m.data = NULL;
                fclose(fp);
                return m;
            }
    fclose(fp);
    return m;
}

// ======================= DISPLAY FUNCTIONS =======================
void display_matrix_by_index(int index)
{
    if (index < 0 || index >= matrix_count)
    {
        printf("Invalid index.\n");
        return;
    }
    Matrix m = matrices[index];
    printf("\nMatrix #%d (%dx%d):\n", index, m.rows, m.cols);
    for (int i = 0; i < m.rows; i++)
    {
        for (int j = 0; j < m.cols; j++)
            printf("%12.6f ", m.data[i][j]);
        printf("\n");
    }
    printf("\n");
}

void display_all_matrices(void)
{
    printf("\n=== %d MATRICES IN MEMORY ===\n", matrix_count);
    if (matrix_count == 0)
    {
        printf("   (empty)\n\n");
        return;
    }
    for (int i = 0; i < matrix_count; i++)
        display_matrix_by_index(i);
}

Matrix get_matrix_from_memory(int index)
{
    Matrix empty = {0, 0, NULL};
    if (index < 0 || index >= matrix_count)
        return empty;
    Matrix copy = allocate_matrix(matrices[index].rows, matrices[index].cols);
    for (int i = 0; i < copy.rows; i++)
        for (int j = 0; j < copy.cols; j++)
            copy.data[i][j] = matrices[index].data[i][j];
    return copy;
}

// ======================= USER INPUT HELPERS =======================
Matrix get_matrix_from_user(const char *name)
{
    Matrix m = {0, 0, NULL};
    int option;
    printf("\n[Matrix %s] 1=Memory, 2=File: ", name);
    scanf("%d", &option);
    if (option == 1)
    {
        display_all_matrices();
        int idx;
        printf("Index: ");
        scanf("%d", &idx);
        m = get_matrix_from_memory(idx);
    }
    else if (option == 2)
    {
        char path[256];
        printf("File path: ");
        scanf("%255s", path);
        m = read_matrix_from_file(path);
        // if (m.data)
        //  add_result_to_memory(m);
    }
    return m;
}

// ======================= IMPORTANT MISSING FUNCTIONS =======================
void enter_matrix_to_file(void)
{
    Matrix m;
    printf("Number of rows: ");
    if (scanf("%d", &m.rows) != 1 || m.rows <= 0)
    {
        printf("Invalid input!\n");
        while (getchar() != '\n')
            ;
        return;
    }
    printf("Number of columns: ");
    if (scanf("%d", &m.cols) != 1 || m.cols <= 0)
    {
        printf("Invalid input!\n");
        while (getchar() != '\n')
            ;
        return;
    }
    m = allocate_matrix(m.rows, m.cols);
    printf("Enter elements row-wise:\n");
    for (int i = 0; i < m.rows; i++)
        for (int j = 0; j < m.cols; j++)
        {
            printf(" [%d][%d]", i, j);
            if (scanf("%lf", &m.data[i][j]) != 1)
            {
                printf("Invalid input!\nPlease enter a number\n");
                while (getchar() != '\n')
                    ;
                j--;
                continue;
            }
        }
    add_result_to_memory(m);

    char filename[256];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(filename, "matrices/matrix_%d_%04d%02d%02d_%02d%02d%02d.txt",
            matrix_count - 1, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    FILE *fp = fopen(filename, "w");
    if (fp)
    {
        fprintf(fp, "%d %d\n", m.rows, m.cols);
        for (int i = 0; i < m.rows; i++)
        {
            for (int j = 0; j < m.cols; j++)
                fprintf(fp, "%.6f ", m.data[i][j]);
            fprintf(fp, "\n");
        }
        fclose(fp);
        printf("Saved as %s\n", filename);
    }
}

void display_folder(void)
{
    char folder[256];
    printf("Folder name: ");
    scanf("%255s", folder);
    DIR *dir = opendir(folder);
    if (!dir)
    {
        printf("Folder not found!\n");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (strstr(entry->d_name, ".txt"))
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", folder, entry->d_name);
            Matrix m = read_matrix_from_file(path);
            if (m.data)
            {
                add_result_to_memory(m);
                printf("Loaded #%d: %s\n", matrix_count - 1, entry->d_name);
            }
        }
    }
    closedir(dir);
}

void save_all_matrices_to_folder(void)
{
    if (matrix_count == 0)
    {
        printf("No matrices to save.\n");
        return;
    }
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    for (int i = 0; i < matrix_count; i++)
    {
        char filename[256];
        sprintf(filename, "matrices/saved_%d_%04d%02d%02d_%02d%02d%02d.txt",
                i, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        FILE *fp = fopen(filename, "w");
        if (fp)
        {
            fprintf(fp, "%d %d\n", matrices[i].rows, matrices[i].cols);
            for (int r = 0; r < matrices[i].rows; r++)
            {
                for (int c = 0; c < matrices[i].cols; c++)
                    fprintf(fp, "%.6f ", matrices[i].data[r][c]);
                fprintf(fp, "\n");
            }
            fclose(fp);
            printf("Saved #%d -> %s\n", i, filename);
        }
    }
}


