#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <n_datos> [verbose=0|1]\n", argv[0]);
        return 1;
    }

    long long n = atoll(argv[1]);
    int verbose = (argc >= 3) ? atoi(argv[2]) : 0;

    if (n <= 0) {
        fprintf(stderr, "Error: n_datos debe ser mayor a 0\n");
        return 1;
    }

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    double sum = 0.0;
    double factor = 1.0;
    for (long long i = 0; i < n; i++) {
        sum += factor / (2.0 * (double)i + 1.0);
        factor = -factor;
    }
    double pi_aprox = 4.0 * sum;

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_segundos = (fin.tv_sec - inicio.tv_sec) + 
                            (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    if (verbose) {
        fprintf(stderr, "[PI Secuencial] n = %lld | Pi calc = %.15f | Pi ref = %.15f | Error = %.5e | Tiempo = %.9f s\n",
                n, pi_aprox, M_PI, fabs(pi_aprox - M_PI), tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%lld,%.9f\n", 1, n, tiempo_segundos);

    return 0;
}
