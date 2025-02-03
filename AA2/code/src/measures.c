#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "time.h"
#include "measures.h"

int plots(char *names[], int n_plots, ...) {
    FILE* plot = popen("gnuplot.exe -persist", "w");
    if (plot) {
        va_list args;
        va_start(args, n_plots);
        fprintf(plot, "set encoding utf8\n");
        fprintf(plot, "set xlabel \"Количество строк матрицы\"\n");
        fprintf(plot, "set ylabel \"Время выполнения, с\"\n");
        fprintf(plot, "set key left\n");
        fprintf(plot, "plot");
        for (int i = 0; i < n_plots; i++) {
            fprintf(plot, " '%s' with linespoints title \"%s\",", va_arg(args, char*), names[i]);
        }
        fprintf(plot, "\n");
        va_end(args);
        fflush(plot);
        pclose(plot);
        return 0;
    }
    pclose(plot);
    return 1;
}

double* measures(int n_points, int n_repeats, int step, int** (multiplication) (int**, int, int, int**, int, int, double*),
    char *file_name) {
    FILE* fp = fopen(file_name, "w");
    if (!fp)
        return NULL;
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
        fprintf(fp, "%d\t%.6lf\n", i * step, points[i - 1]);
    }
    fclose(fp);
    return points;
}
