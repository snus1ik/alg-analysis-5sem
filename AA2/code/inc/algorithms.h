#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#define _CRT_SECURE_NO_WARNINGS

#define INPUT_ERROR -1
#define MUL_ERROR -2
int** allocate_matrix(int c_row, int c_col);
void free_matrix(int** matrix, int c_row);
int** scan_matrix(int* c_row, int* c_col);
void print_matrix(int** matrix, int c_row, int c_col);
int** random_matrix(int c_row, int c_col);
int** matrix_mul(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time);
int** matrix_mul_winograd(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time);
int** matrix_mul_winograd_optimized(int** fir, int c_row1, int c_col1, int** sec, int c_row2, int c_col2, double *process_time);

#endif

