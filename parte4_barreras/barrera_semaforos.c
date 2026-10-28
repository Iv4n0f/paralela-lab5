#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

int thread_count;
long num_phases;

sem_t count_sem;
sem_t barrier_sem1;
sem_t barrier_sem2;
int counter = 0;

int* current_thread_phase = NULL;
volatile int barrier_violations = 0;

void Barrier_sem(void) {
    // Fase 1: Llegada y bloqueo
    sem_wait(&count_sem);
    counter++;
    if (counter == thread_count) {
        for (int i = 0; i < thread_count; i++) {
            sem_post(&barrier_sem1);
        }
    }
    sem_post(&count_sem);
    sem_wait(&barrier_sem1);

    // Fase 2: Salida controlada para permitir reutilización segura
    sem_wait(&count_sem);
    counter--;
    if (counter == 0) {
        for (int i = 0; i < thread_count; i++) {
            sem_post(&barrier_sem2);
        }
    }
    sem_post(&count_sem);
    sem_wait(&barrier_sem2);
}

void* Thread_work(void* rank) {
    long my_rank = (long)rank;

    for (int phase = 0; phase < num_phases; phase++) {
        current_thread_phase[my_rank] = phase;

        for (int i = 0; i < thread_count; i++) {
            if (current_thread_phase[i] > phase + 1) {
                __sync_fetch_and_add(&barrier_violations, 1);
            }
        }

        Barrier_sem();

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

    sem_init(&count_sem, 0, 1);
    sem_init(&barrier_sem1, 0, 0);
    sem_init(&barrier_sem2, 0, 0);
    counter = 0;
    barrier_violations = 0;

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
        fprintf(stderr, "[Barrera Semáforos] Hilos = %d | Fases = %ld | Violaciones = %d | Tiempo = %.9f s\n",
                thread_count, num_phases, barrier_violations, tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%ld,%.9f\n", thread_count, num_phases, tiempo_segundos);

    sem_destroy(&count_sem);
    sem_destroy(&barrier_sem1);
    sem_destroy(&barrier_sem2);
    free(current_thread_phase);
    free(thread_handles);

    return 0;
}
