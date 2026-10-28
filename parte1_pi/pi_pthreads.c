#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Variables globales compartidas
long long n_terms;
int thread_count;
double global_sum = 0.0;
pthread_mutex_t mutex_sum;

// Estructura de argumentos para el hilo
typedef struct {
    long rank;
} ThreadArg;

void* Thread_sum(void* rank) {
    long my_rank = (long)rank;

    // Descomposición en bloques continuos equilibrados
    long long my_n = n_terms / thread_count;
    long long remainder = n_terms % thread_count;
    long long my_first, my_last;

    if (my_rank < remainder) {
        my_first = my_rank * (my_n + 1);
        my_last = my_first + (my_n + 1);
    } else {
        my_first = remainder * (my_n + 1) + (my_rank - remainder) * my_n;
        my_last = my_first + my_n;
    }

    // Acumulador local privado para evitar condiciones de carrera y sobrecarga de mutex
    double my_sum = 0.0;
    double factor = (my_first % 2 == 0) ? 1.0 : -1.0;

    for (long long i = my_first; i < my_last; i++) {
        my_sum += factor / (2.0 * (double)i + 1.0);
        factor = -factor;
    }

    // Reducción a la variable global compartida en sección crítica (una sola vez por hilo)
    pthread_mutex_lock(&mutex_sum);
    global_sum += my_sum;
    pthread_mutex_unlock(&mutex_sum);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <num_threads> <n_datos> [verbose=0|1]\n", argv[0]);
        return 1;
    }

    thread_count = atoi(argv[1]);
    n_terms = atoll(argv[2]);
    int verbose = (argc >= 4) ? atoi(argv[3]) : 0;

    if (thread_count <= 0 || n_terms <= 0) {
        fprintf(stderr, "Error: num_threads y n_datos deben ser mayores a 0\n");
        return 1;
    }

    pthread_t* thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
    if (thread_handles == NULL) {
        fprintf(stderr, "Error asignando memoria para handles de hilos\n");
        return 1;
    }

    pthread_mutex_init(&mutex_sum, NULL);
    global_sum = 0.0;

    struct timespec inicio, fin;

    // Medición exclusiva de la sección concurrente / paralela
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Thread_sum, (void*)thread);
    }

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    double pi_aprox = 4.0 * global_sum;

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_segundos = (fin.tv_sec - inicio.tv_sec) + 
                            (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    if (verbose) {
        fprintf(stderr, "[PI Pthreads] Hilos = %d | n = %lld | Pi calc = %.15f | Pi ref = %.15f | Error = %.5e | Tiempo = %.9f s\n",
                thread_count, n_terms, pi_aprox, M_PI, fabs(pi_aprox - M_PI), tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%lld,%.9f\n", thread_count, n_terms, tiempo_segundos);

    pthread_mutex_destroy(&mutex_sum);
    free(thread_handles);

    return 0;
}
