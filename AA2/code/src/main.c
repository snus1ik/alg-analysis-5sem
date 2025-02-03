#include <stdio.h>
#include <stdlib.h>

#include "algorithms.h"
#include "measures.h"

int main(void)
{
    int option = 0;
    printf("1) Input two matrices, get multiplication result\n");
    printf("2) Time measures\n");
    printf("Enter your choice:\n");
    if (scanf("%d", &option) != 1)
        return 1;
    if (option == 1) {
        int c_row1, c_col1;
        double time_def, time_win;
        int** m1 = scan_matrix(&c_row1, &c_col1);
        if (!m1) {
            printf("Input error\n");
            return INPUT_ERROR;
        }
        int c_row2, c_col2;
        int** m2 = scan_matrix(&c_row2, &c_col2);
        if (!m2) {
            printf("Input error\n");
            free_matrix(m1, c_row1);
            return INPUT_ERROR;
        }

        int** res_def = matrix_mul(m1, c_row1, c_col1, m2, c_row2, c_col2, &time_def);
        if (!res_def) {
            printf("Matrix multiplication failed\n");
            free_matrix(m1, c_row1);
            free_matrix(m2, c_row2);
            return MUL_ERROR;
        }
        printf("Result (default algorithm):\n");
        print_matrix(res_def, c_row1, c_col2);

        int** res_win = matrix_mul_winograd(m1, c_row1, c_col1, m2, c_row2, c_col2, &time_win);
        if (!res_win) {
            printf("Matrix multiplication failed\n");
            free_matrix(m1, c_row1);
            free_matrix(m2, c_row2);
            return MUL_ERROR;
        }
        printf("Result (Winograd algorithm):\n");
        print_matrix(res_win, c_row1, c_col2);

        int** res_win_opt = matrix_mul_winograd_optimized(m1, c_row1, c_col1, m2, c_row2, c_col2, &time_win);
        if (!res_win_opt) {
            printf("Matrix multiplication failed\n");
            free_matrix(m1, c_row1);
            free_matrix(m2, c_row2);
            return MUL_ERROR;
        }
        printf("Result (Optimized Winograd algorithm):\n");
        print_matrix(res_win_opt, c_row1, c_col2);

        free_matrix(m1, c_row1);
        free_matrix(m2, c_row2);
        free_matrix(res_def, c_row1);
        free_matrix(res_win, c_row1);
        free_matrix(res_win_opt, c_row1);
    }
    else if (option == 2) {
        int n_repeats, n_points, step;
        printf("Enter number of measures:\n");
        if (scanf("%d", &n_points) != 1 || n_points <= 0)
            return INPUT_ERROR;
        printf("Enter number of measures repeats:\n");
        if (scanf("%d", &n_repeats) != 1 || n_repeats <= 0)
            return INPUT_ERROR;
        printf("Enter matrix size step between experiments:\n");
        if (scanf("%d", &step) != 1 || step <= 0)
            return INPUT_ERROR;
        char *matr_mul = "matr_mul.dat";
        char *matr_mul_win = "matr_mul_win.dat";
        char *matr_mul_win_opt = "matr_mul_win_opt.dat";
        double *m = measures(n_points, n_repeats, step, matrix_mul, matr_mul);
        if (!m)
            return -1;
        for (int i = 0; i < n_points; i++)
            printf("%lf ", m[i]);
        printf("\n");
        free(m);

        m = measures(n_points, n_repeats, step, matrix_mul_winograd, matr_mul_win);
        if (!m)
            return -1;
        for (int i = 0; i < n_points; i++)
            printf("%lf ", m[i]);
        printf("\n");
        free(m);

        m = measures(n_points, n_repeats, step, matrix_mul_winograd_optimized, matr_mul_win_opt);
        if (!m)
            return -1;
        for (int i = 0; i < n_points; i++)
            printf("%lf ", m[i]);
        printf("\n");
        free(m);
        char *names[] = {"Стандартный", "Виноград", "Виноград(опт)"};
        plots(names, 3, matr_mul, matr_mul_win, matr_mul_win_opt);
    }
    return 0;
}
