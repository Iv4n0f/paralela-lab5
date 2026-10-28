#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_KEY 65535
#define INITIAL_KEYS 1000

struct list_node_s {
    int data;
    struct list_node_s* next;
};

struct list_node_s* head = NULL;
pthread_mutex_t list_mutex;
int thread_count;
long total_ops;
double pct_member = 0.80;
double pct_insert = 0.10;

int Member(int value) {
    struct list_node_s* curr_p;
    int result = 0;

    pthread_mutex_lock(&list_mutex);
    curr_p = head;
    while (curr_p != NULL && curr_p->data < value)
        curr_p = curr_p->next;

    if (curr_p != NULL && curr_p->data == value)
        result = 1;
    pthread_mutex_unlock(&list_mutex);

    return result;
}

int Insert(int value) {
    struct list_node_s* curr_p;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;
    int result = 0;

    pthread_mutex_lock(&list_mutex);
    curr_p = head;
    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data > value) {
        temp_p = (struct list_node_s*)malloc(sizeof(struct list_node_s));
        if (temp_p != NULL) {
            temp_p->data = value;
            temp_p->next = curr_p;
            if (pred_p == NULL)
                head = temp_p;
            else
                pred_p->next = temp_p;
            result = 1;
        }
    }
    pthread_mutex_unlock(&list_mutex);

    return result;
}

int Delete(int value) {
    struct list_node_s* curr_p;
    struct list_node_s* pred_p = NULL;
    int result = 0;

    pthread_mutex_lock(&list_mutex);
    curr_p = head;
    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value) {
        if (pred_p == NULL) {
            head = curr_p->next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        result = 1;
    }
    pthread_mutex_unlock(&list_mutex);

    return result;
}

void Free_list(void) {
    struct list_node_s* current = head;
    while (current != NULL) {
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

    for (long i = 0; i < ops_per_thread; i++) {
        float prob = (float)rand_r(&seed) / (float)RAND_MAX;
        int val = rand_r(&seed) % MAX_KEY;

        if (prob < pct_member) {
            Member(val);
        } else if (prob < pct_member + pct_insert) {
            Insert(val);
        } else {
            Delete(val);
        }
    }
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

    pthread_mutex_init(&list_mutex, NULL);

    // Inicializar lista con datos iniciales
    unsigned int init_seed = 42;
    int initial_keys = (INITIAL_KEYS < total_ops) ? INITIAL_KEYS : (int)(total_ops / 2);
    for (int i = 0; i < initial_keys; i++) {
        Insert(rand_r(&init_seed) % MAX_KEY);
    }

    pthread_t* thread_handles = (pthread_t*)malloc(thread_count * sizeof(pthread_t));

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
        fprintf(stderr, "[Lista Mutex] Hilos = %d | Ops = %ld | Tiempo = %.9f s\n",
                thread_count, total_ops, tiempo_segundos);
    }

    // Salida estándar exclusiva para CSV: num_threads, n_datos, tiempo_segundos
    printf("%d,%ld,%.9f\n", thread_count, total_ops, tiempo_segundos);

    pthread_mutex_destroy(&list_mutex);
    free(thread_handles);
    Free_list();

    return 0;
}
