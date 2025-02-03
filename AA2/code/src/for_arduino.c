#include <stdlib.h>
#include <stdio.h>
#include "time.h"

void free_matrix(int** matrix, int c_row)
{
    for (int i = 0; i < c_row; i++)
        free(matrix[i]);
    free(matrix);
}

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
                result[i][j] = result[i][j] + (fir[i][k * 2 + 1]+ sec[k * 2][j]) * (fir[i][k * 2] + sec[k * 2 + 1][j]);
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

int** matrix_mul_winograd_optimized(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time) {
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

double* measures(int n_points, int n_repeats, int step, int** (multiplication) (int**, int, int, int**, int, int, double*)) {
    srand(time(NULL));
    double *points = malloc(sizeof(double) * n_points);
    if (!points)
        return NULL;
    for (int i = 0; i < n_points; i++) {
        points[i] = 0.0;
    }
    double process_time;
    for (int i = 1; i < n_points + 1; i++) {
        for (int j = 0; j < n_repeats; j++) {
            int size = i * step;
            int **fir = random_matrix(size, size);
            if (!fir)
                return NULL;
            int **sec = random_matrix(size, size);
            if (!sec)
                return NULL;
            int **result = multiplication(fir, size, size, sec, size, size, &process_time);
            if (!result) {
                free(fir);
                free(sec);
                return NULL;
            }
            points[i - 1] += process_time;
            free(fir);
            free(sec);
            free(result);
        }
        points[i - 1] /= n_repeats;
    }
    return points;
}

int main(void) {
    const int n_repeats = 50, n_points = 10, step = 10;

    double *m = measures(n_points, n_repeats, step, matrix_mul);
    if (!m)
        return -1;
    for (int i = 0; i < n_points; i++)
        printf("%d %lf ", step * (i + 1), m[i]);
    printf("\n");
    free(m);

    m = measures(n_points, n_repeats, step, matrix_mul_winograd);
    if (!m)
        return -1;
    for (int i = 0; i < n_points; i++)
        printf("%d %lf ", step * (i + 1), m[i]);
    printf("\n");
    free(m);

    m = measures(n_points, n_repeats, step, matrix_mul_winograd_optimized);
    if (!m)
        return -1;
    for (int i = 0; i < n_points; i++)
        printf("%d %lf ", step * (i + 1), m[i]);
    printf("\n");
    free(m);
    return 0;
}