#ifndef MEASURES_H
#define MEASURES_H

#include "algorithms.h"

double* measures(int n_points, int n_repeats, int step, int** (multiplication) (int**, int, int, int**, int, int, double*),
    char *file_name);
int plots(char *names[], int n_plots, ...) ;

#endif //MEASURES_H
