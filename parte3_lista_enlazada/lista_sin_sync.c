#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_KEY 65535
#define INITIAL_KEYS 1000
#define MAX_HOPS 50000

struct list_node_s {
    int data;
    struct list_node_s* next;
};

struct list_node_s* head = NULL;
int thread_count;
long total_ops;
double pct_member = 0.80;
double pct_insert = 0.10;

// Variables de manejo de fallos seguras por hilo (TLS)
// Para evitar siglongjmp entre hilos (UB), cada hilo tiene su propio jmp_buf privado
static __thread sigjmp_buf thread_jmp_buf;
static __thread volatile sig_atomic_t thread_jump_ready = 0;
static volatile sig_atomic_t race_failure_detected = 0;

void signal_handler(int sig) {
    (void)sig;
    race_failure_detected = 1;
    if (thread_jump_ready) {
        thread_jump_ready = 0;
        // Salto intra-hilo seguro: restaura el contexto del MISMO hilo
        siglongjmp(thread_jmp_buf, 1);
    } else {
        _exit(1);
    }
}

int Member(int value) {
    struct list_node_s* curr_p = head;
    int hops = 0;
    // Límite de saltos para evitar bloqueos indefinidos si se forman ciclos
    while (curr_p != NULL && curr_p->data < value && ++hops < MAX_HOPS)
        curr_p = curr_p->next;

    if (curr_p == NULL || curr_p->data > value || hops >= MAX_HOPS)
        return 0;
    else
        return 1;
}

int Insert(int value) {
    struct list_node_s* curr_p = head;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;
    int hops = 0;

    while (curr_p != NULL && curr_p->data < value && ++hops < MAX_HOPS) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (hops >= MAX_HOPS) {
        return 0; // Abortar inserción ante detección de ciclo
    }

    if (curr_p == NULL || curr_p->data > value) {
        temp_p = (struct list_node_s*)malloc(sizeof(struct list_node_s));
        if (!temp_p) return 0;
        temp_p->data = value;
        temp_p->next = curr_p;
        if (pred_p == NULL)
            head = temp_p;
        else
            pred_p->next = temp_p;
        return 1;
    } else {
        return 0;
    }
}

int Delete(int value) {
    struct list_node_s* curr_p = head;
    struct list_node_s* pred_p = NULL;
    int hops = 0;

    while (curr_p != NULL && curr_p->data < value && ++hops < MAX_HOPS) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (hops >= MAX_HOPS) {
        return 0; // Abortar eliminación ante detección de ciclo
    }

    if (curr_p != NULL && curr_p->data == value) {
        if (pred_p == NULL) {
            head = curr_p->next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    } else {
        return 0;
    }
}

void Free_list(void) {
    struct list_node_s* current = head;
    int hops = 0;
    while (current != NULL && ++hops < MAX_HOPS) {
        struct list_node_s* temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
}

void* Thread_work(void* rank) {
    long my_rank = (long)rank;
    long ops_per_thread = total_ops / thread_count;
    unsigned int seed = (unsigned int)(my_rank + 100);

    // Preparar recuperación intra-hilo para este hilo específico
    thread_jump_ready = 1;
    if (sigsetjmp(thread_jmp_buf, 1) != 0) {
        // Fallo por condición de carrera capturado de forma segura en este hilo
        thread_jump_ready = 0;
        return NULL;
    }

    for (long i = 0; i < ops_per_thread; i++) {
        if (race_failure_detected) {
            break;
        }

        float prob = (float)rand_r(&seed) / (float)RAND_MAX;
        int val = rand_r(&seed) % MAX_KEY;

        // SIN sincronización: condiciones de carrera intencionales
        if (prob < pct_member) {
            Member(val);
        } else if (prob < pct_member + pct_insert) {
            Insert(val);
        } else {
            Delete(val);
        }
    }

    thread_jump_ready = 0;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <num_threads> <total_ops> [verbose=0|1]\n", argv[0]);
        return 1;
    }

    thread_count = atoi(argv[1]);
    total_ops = atol(argv[2]);
    int verbose = (argc >= 4) ? atoi(argv[3]) : 0;

    if (thread_count <= 0 || total_ops <= 0) {
        fprintf(stderr, "Error: num_threads y total_ops deben ser mayores a 0\n");
        return 1;
    }

    // Inicializar lista con datos iniciales
    unsigned int init_seed = 42;
    int initial_keys = (INITIAL_KEYS < total_ops) ? INITIAL_KEYS : (int)(total_ops / 2);
    for (int i = 0; i < initial_keys; i++) {
        Insert(rand_r(&init_seed) % MAX_KEY);
    }

    pthread_t* thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
    if (!thread_handles) {
        fprintf(stderr, "Error asignando memoria para handles\n");
        return 1;
    }

    // Configurar manejadores de señales para fallos de memoria concurrentes
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    struct timespec inicio, fin;
    double tiempo_segundos = 0.0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (long t = 0; t < thread_count; t++) {
        pthread_create(&thread_handles[t], NULL, Thread_work, (void*)t);
    }

    for (long t = 0; t < thread_count; t++) {
        pthread_join(thread_handles[t], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    if (race_failure_detected) {
        tiempo_segundos = -1.0; // Señal de fallo por condición de carrera
        if (verbose) {
            fprintf(stderr, "[Lista Sin Sync] Condición de carrera detectada y manejada con seguridad.\n");
        }
    } else {
        tiempo_segundos = (fin.tv_sec - inicio.tv_sec) + 
                          (fin.tv_nsec - inicio.tv_nsec) / 1e9;
        if (verbose) {
            fprintf(stderr, "[Lista Sin Sync] Hilos = %d | Ops = %ld | Tiempo = %.9f s (Inseguro)\n",
                    thread_count, total_ops, tiempo_segundos);
        }
    }

    // Formato CSV
    printf("%d,%ld,%.9f\n", thread_count, total_ops, tiempo_segundos);

    free(thread_handles);
    // Solo liberar memoria si no hubo corrupción de memoria detectada
    if (!race_failure_detected) {
        Free_list();
    }

    return 0;
}
