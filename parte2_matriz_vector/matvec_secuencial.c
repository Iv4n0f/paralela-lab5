#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void init_data(double* A, double* x, long n) {
    for (long i = 0; i < n; i++) {
        x[i] = 1.0 + (i % 100) * 0.01;
        for (long j = 0; j < n; j++) {
            A[i * n + j] = ((i + j) % 100) * 0.01;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <dimension_n> [verbose=0|1]\n", argv[0]);
        return 1;
    }

    long n = atol(argv[1]);
    int verbose = (argc >= 3) ? atoi(argv[2]) : 0;

    if (n <= 0) {
        fprintf(stderr, "Error: dimension_n debe ser mayor a 0\n");
        return 1;
    }

    double* A = (double*)malloc(n * n * sizeof(double));
    double* x = (double*)malloc(n * sizeof(double));
    double* y = (double*)malloc(n * sizeof(double));

    if (!A || !x || !y) {
        fprintf(stderr, "Error al asignar memoria para matrices/vectores\n");
        return 1;
    }

    init_data(A, x, n);

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (long i = 0; i < n; i++) {
        double sum = 0.0;
        long row_offset = i * n;
        for (long j = 0; j < n; j++) {
            sum += A[row_offset + j] * x[j];
        }
        y[i] = sum;
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_segundos = (fin.tv_sec - inicio.tv_sec) + 
                            (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    if (verbose) {
        fprintf(stderr, "[MatVec Secuencial] n = %ld | y[0] = %.6f | y[n-1] = %.6f | Tiempo = %.9f s\n",
                n, y[0], y[n - 1], tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%ld,%.9f\n", 1, n, tiempo_segundos);

    free(A);
    free(x);
    free(y);

    return 0;
}
