#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

// Variables globales compartidas
long n_dim;
int thread_count;
double* A_mat = NULL;
double* x_vec = NULL;
double* y_vec = NULL;

void init_data(double* A, double* x, long n) {
    for (long i = 0; i < n; i++) {
        x[i] = 1.0 + (i % 100) * 0.01;
        for (long j = 0; j < n; j++) {
            A[i * n + j] = ((i + j) % 100) * 0.01;
        }
    }
}

void* Thread_matvec(void* rank) {
    long my_rank = (long)rank;

    // Descomposición 1D por bloques contiguos de filas
    long my_rows = n_dim / thread_count;
    long remainder = n_dim % thread_count;
    long my_first_row, my_last_row;

    if (my_rank < remainder) {
        my_first_row = my_rank * (my_rows + 1);
        my_last_row = my_first_row + (my_rows + 1);
    } else {
        my_first_row = remainder * (my_rows + 1) + (my_rank - remainder) * my_rows;
        my_last_row = my_first_row + my_rows;
    }

    // Cada hilo calcula su partición de filas.
    // Como las particiones de filas son estrictamente disjuntas entre hilos,
    // cada hilo escribe exclusivamente en posiciones y[i] únicas.
    // Por ende, no existe condición de carrera y no se requiere mutex.
    for (long i = my_first_row; i < my_last_row; i++) {
        double sum = 0.0;
        long row_offset = i * n_dim;
        for (long j = 0; j < n_dim; j++) {
            sum += A_mat[row_offset + j] * x_vec[j];
        }
        y_vec[i] = sum;
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <num_threads> <dimension_n> [verbose=0|1]\n", argv[0]);
        return 1;
    }

    thread_count = atoi(argv[1]);
    n_dim = atol(argv[2]);
    int verbose = (argc >= 4) ? atoi(argv[3]) : 0;

    if (thread_count <= 0 || n_dim <= 0) {
        fprintf(stderr, "Error: num_threads y dimension_n deben ser mayores a 0\n");
        return 1;
    }

    A_mat = (double*)malloc(n_dim * n_dim * sizeof(double));
    x_vec = (double*)malloc(n_dim * sizeof(double));
    y_vec = (double*)malloc(n_dim * sizeof(double));
    pthread_t* thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));

    if (!A_mat || !x_vec || !y_vec || !thread_handles) {
        fprintf(stderr, "Error al asignar memoria\n");
        return 1;
    }

    init_data(A_mat, x_vec, n_dim);

    struct timespec inicio, fin;

    // Medición exclusiva de la sección concurrente / paralela
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Thread_matvec, (void*)thread);
    }

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_segundos = (fin.tv_sec - inicio.tv_sec) + 
                            (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    if (verbose) {
        // Verificación contra cálculo secuencial
        double max_diff = 0.0;
        for (long i = 0; i < n_dim; i++) {
            double seq_sum = 0.0;
            long row_offset = i * n_dim;
            for (long j = 0; j < n_dim; j++) {
                seq_sum += A_mat[row_offset + j] * x_vec[j];
            }
            double diff = fabs(y_vec[i] - seq_sum);
            if (diff > max_diff) max_diff = diff;
        }
        fprintf(stderr, "[MatVec Pthreads] Hilos = %d | n = %ld | MaxDiffSeq = %.5e | y[0] = %.6f | Tiempo = %.9f s\n",
                thread_count, n_dim, max_diff, y_vec[0], tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%ld,%.9f\n", thread_count, n_dim, tiempo_segundos);

    free(A_mat);
    free(x_vec);
    free(y_vec);
    free(thread_handles);

    return 0;
}
