#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

int thread_count;
long num_phases;

int counter1 = 0;
int counter2 = 0;
volatile int phase_flag = 0;
pthread_mutex_t barrier_mutex;

// Arreglo de verificación de fase por hilo
int* current_thread_phase = NULL;
volatile int barrier_violations = 0;

void Barrier_busywait(int my_rank, int current_phase) {
    (void)my_rank;
    (void)current_phase;

    int my_phase = phase_flag;
    pthread_mutex_lock(&barrier_mutex);
    counter1++;
    if (counter1 == thread_count) {
        counter1 = 0;
        phase_flag = 1 - my_phase; // Alterna la bandera de fase
    }
    pthread_mutex_unlock(&barrier_mutex);

    // Espera activa (busy-waiting) evaluando el cambio de fase
    while (phase_flag == my_phase) {
        #if defined(__x86_64__) || defined(_M_X64)
        __builtin_ia32_pause();
        #endif
    }
}

void* Thread_work(void* rank) {
    long my_rank = (long)rank;

    for (int phase = 0; phase < num_phases; phase++) {
        current_thread_phase[my_rank] = phase;

        // Verificación multifase: ningún hilo debe estar más de 1 fase adelante
        for (int i = 0; i < thread_count; i++) {
            if (current_thread_phase[i] > phase + 1) {
                __sync_fetch_and_add(&barrier_violations, 1);
            }
        }

        // Sincronización mediante barrera busy-wait
        Barrier_busywait((int)my_rank, phase);

        // Al salir de la barrera, todos los hilos deben haber alcanzado al menos 'phase'
        for (int i = 0; i < thread_count; i++) {
            if (current_thread_phase[i] < phase) {
                __sync_fetch_and_add(&barrier_violations, 1);
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <num_threads> <num_phases> [verbose=0|1]\n", argv[0]);
        return 1;
    }

    thread_count = atoi(argv[1]);
    num_phases = atol(argv[2]);
    int verbose = (argc >= 4) ? atoi(argv[3]) : 0;

    if (thread_count <= 0 || num_phases <= 0) {
        fprintf(stderr, "Error: num_threads y num_phases deben ser mayores a 0\n");
        return 1;
    }

    current_thread_phase = (int*)calloc(thread_count, sizeof(int));
    pthread_t* thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
    pthread_mutex_init(&barrier_mutex, NULL);
    barrier_violations = 0;
    counter1 = 0;
    phase_flag = 0;

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (long t = 0; t < thread_count; t++) {
        pthread_create(&thread_handles[t], NULL, Thread_work, (void*)t);
    }

    for (long t = 0; t < thread_count; t++) {
        pthread_join(thread_handles[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_segundos = (fin.tv_sec - inicio.tv_sec) + 
                            (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    if (verbose) {
        fprintf(stderr, "[Barrera Busy-Wait] Hilos = %d | Fases = %ld | Violaciones = %d | Tiempo = %.9f s\n",
                thread_count, num_phases, barrier_violations, tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%ld,%.9f\n", thread_count, num_phases, tiempo_segundos);

    pthread_mutex_destroy(&barrier_mutex);
    free(current_thread_phase);
    free(thread_handles);

    return 0;
}
