#include <stdio.h>
#include <stdlib.h>
#include "algorithms.h"
#include "time.h"

int** allocate_matrix(int c_row, int c_col)
{
    int** matrix = calloc(c_row, sizeof(int*));
    if (!matrix)
        return NULL;
    for (int i = 0; i < c_row; i++){
        matrix[i] = malloc(c_col * sizeof(int));
        if (!matrix[i]){
            free_matrix(matrix, c_row);
            return NULL;
        }
    }
    return matrix;
}

void free_matrix(int** matrix, int c_row)
{
    for (int i = 0; i < c_row; i++)
        free(matrix[i]);
    free(matrix);
}

int** scan_matrix(int *c_row, int *c_col)
{
    printf("Please enter the number of rows and columns:\n");
    if (scanf("%d", c_row) != 1 || c_row <= 0)
        return NULL;
    if (scanf("%d", c_col) != 1 || c_col <= 0)
        return NULL;
    int** matrix = allocate_matrix(*c_row, *c_col);
    if (!matrix)
        return NULL;
    printf("Enter the elements of the matrix:\n");
    for (int i = 0; i < *c_row; i++)
        for (int j = 0; j < *c_col; j++)
            if (scanf("%d", &matrix[i][j]) != 1){
                free_matrix(matrix, *c_row);
                return NULL;
            }
    return matrix;
}

void print_matrix(int** matrix, int c_row, int c_col)
{
    for (int i = 0; i < c_row; i++){
        for (int j = 0; j < c_col; j++)
            printf("%d ", matrix[i][j]);
        printf("\n");
    }
}

int** random_matrix(int c_row, int c_col)
{
    srand(time(NULL));
    int** matrix = allocate_matrix(c_row, c_col);
    if (!matrix)
        return NULL;
    for (int i = 0; i < c_row; i++)
        for (int j = 0; j < c_col; j++)
            matrix[i][j] = rand() % 100;
    return matrix;
}

int** matrix_mul(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time)
{
    int c_row = c_row1;
    int c_col = c_col2;
    if (c_col1 != c_row2)
        return NULL;
    int** result = allocate_matrix(c_row, c_col);
    if (!result)
        return NULL;
    clock_t begin = clock();
    for (int i = 0; i < c_row; i++)
        for (int j = 0; j < c_col; j++){
            result[i][j] = 0;
            for (int k = 0; k < c_col1; k++)
                result[i][j] = result[i][j] + fir[i][k] * sec[k][j];
        }
    *process_time = (double) (clock() - begin) / CLOCKS_PER_SEC;
    return result;
}

int** matrix_mul_winograd(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time)
{
    int c_row = c_row1;
    int c_col = c_col2;
    int* mulH = malloc(c_row1 * sizeof(int));
    if (!mulH)
        return NULL;
    for (int i = 0; i < c_row1; i++)
        mulH[i] = 0;

    int* mulV = malloc(c_col2 * sizeof(int));
    if (!mulV){
        free(mulH);
        return NULL;
    }
    for (int i = 0; i < c_col2; i++)
        mulV[i] = 0;

    int** result = allocate_matrix(c_row, c_col);
    if (!result) {
        free(mulH);
        free(mulV);
        return NULL;
    }

    clock_t begin = clock();
    for (int i = 0; i < c_row1; i++){
        for (int j = 0; j < c_col1 / 2; j++){
            mulH[i] = mulH[i] + fir[i][j * 2] * fir[i][j * 2 + 1];
        }
    }

    for (int i = 0; i < c_col2; i++){
        for (int j = 0; j < c_row2 / 2; j++){
            mulV[i] = mulV[i] + sec[j * 2][i] * sec[j * 2 + 1][i];
        }
    }

    for (int i = 0; i < c_row; i++) {
        for (int j = 0; j < c_col; j++){
            result[i][j] = - (mulH[i] + mulV[j]);
            for (int k = 0; k < c_col1 / 2; k++){
                result[i][j] = result[i][j] + (fir[i][k * 2 + 1] + sec[k * 2][j]) * (fir[i][k * 2] + sec[k * 2 + 1][j]);
            }
        }
    }
    if (c_col1 % 2) {
        for (int i = 0; i < c_row; i++){
            for (int j = 0; j < c_col; j++){
                result[i][j] = result[i][j] + fir[i][c_col1 - 1] * sec[c_col1 - 1][j];
            }
        }
    }
    *process_time = (double) (clock() - begin) / CLOCKS_PER_SEC;
    free(mulH);
    free(mulV);

    return result;
}

int** matrix_mul_winograd_optimized(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time)
{
    int c_row = c_row1;
    int c_col = c_col2;
    int* mulH = malloc(c_row1 * sizeof(int));
    if (!mulH)
        return NULL;
    for (int i = 0; i < c_row1; i++)
        mulH[i] = 0;

    int* mulV = malloc(c_col2 * sizeof(int));
    if (!mulV){
        free(mulH);
        return NULL;
    }
    for (int i = 0; i < c_col2; i++)
        mulV[i] = 0;

    int** result = allocate_matrix(c_row, c_col);
    if (!result) {
        free(mulH);
        free(mulV);
        return NULL;
    }

    clock_t begin = clock();
    for (int i = 0; i < c_row1; i++){
        for (int j = 0; j < c_col1 / 2; j++){
            mulH[i] -= fir[i][j << 1] * fir[i][(j << 1) + 1];
        }
    }

    for (int i = 0; i < c_col2; i++){
        for (int j = 0; j < c_row2 / 2; j++){
            mulV[i] -= sec[j << 1][i] * sec[(j << 1) + 1][i];
        }
    }

    for (int i = 0; i < c_row; i++) {
        for (int j = 0; j < c_col; j++){
            result[i][j] = mulH[i] + mulV[j];
            for (int k = 0; k < c_col1 / 2; k++){
                result[i][j] += (fir[i][(k << 1) + 1]+ sec[k << 1][j]) * (fir[i][k << 1] + sec[(k << 1) + 1][j]);
            }
        }
    }

    if (c_col1 % 2) {
        for (int i = 0; i < c_row; i++){
            for (int j = 0; j < c_col; j++){
                result[i][j] += fir[i][c_col1 - 1] * sec[c_col1 - 1][j];
            }
        }
    }
    *process_time = (double) (clock() - begin) / CLOCKS_PER_SEC;
    free(mulH);
    free(mulV);

    return result;
}
