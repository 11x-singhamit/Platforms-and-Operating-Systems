#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS 16
#define ITERATIONS 1000000

pthread_mutex_t mutex;
long counter = 0;

void* worker(void* arg) {
    for(int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

double get_time_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main() {

    pthread_t threads[MAX_THREADS];
    pthread_mutex_init(&mutex, NULL);

    printf("Linux Mutex Benchmark\n\n");

    for(int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {

        counter = 0;
        double start = get_time_sec();

        for(int i = 0; i < num_threads; i++)
            pthread_create(&threads[i], NULL, worker, NULL);

        for(int i = 0; i < num_threads; i++)
            pthread_join(threads[i], NULL);

        double end = get_time_sec();

        double time_taken = end - start;
        double throughput = counter / time_taken;

        printf("Threads: %d | Throughput: %.0f ops/sec\n", num_threads, throughput);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}
