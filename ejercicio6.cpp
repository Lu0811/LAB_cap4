#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    int readers;
    int writers;
    int waiting_writers;
    int prefer_writers; // 1 for writer preference, 0 for reader preference
} rwlock_t;

void rwlock_init(rwlock_t *lock, int prefer_writers) {
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->read_cond, NULL);
    pthread_cond_init(&lock->write_cond, NULL);
    lock->readers = 0;
    lock->writers = 0;
    lock->waiting_writers = 0;
    lock->prefer_writers = prefer_writers;
}

void rwlock_read_lock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    while (lock->writers > 0 || (lock->prefer_writers && lock->waiting_writers > 0)) {
        pthread_cond_wait(&lock->read_cond, &lock->mutex);
    }
    lock->readers++;
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_read_unlock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->readers--;
    if (lock->readers == 0) {
        pthread_cond_signal(&lock->write_cond);
    }
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_write_lock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->waiting_writers++;
    while (lock->readers > 0 || lock->writers > 0) {
        pthread_cond_wait(&lock->write_cond, &lock->mutex);
    }
    lock->waiting_writers--;
    lock->writers++;
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_write_unlock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->writers--;
    if (lock->waiting_writers > 0) {
        pthread_cond_signal(&lock->write_cond);
    } else {
        pthread_cond_broadcast(&lock->read_cond);
    }
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_destroy(rwlock_t *lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->read_cond);
    pthread_cond_destroy(&lock->write_cond);
}

rwlock_t lock;
int readers_count = 0, writers_count = 0;

void *reader(void *arg) {
    rwlock_read_lock(&lock);
    usleep(100); 
    rwlock_read_unlock(&lock);
    return NULL;
}

void *writer(void *arg) {
    rwlock_write_lock(&lock);
    usleep(100); 
    rwlock_write_unlock(&lock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <num_lectores> <num_escritores> <preferencia_escritores (1 o 0)> <num_iteraciones>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    int prefer_writers = atoi(argv[3]);
    int iterations = atoi(argv[4]);

    rwlock_init(&lock, prefer_writers);

    pthread_t readers[num_readers], writers[num_writers];
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < num_readers; i++) {
            pthread_create(&readers[i], NULL, reader, NULL);
        }
        for (int i = 0; i < num_writers; i++) {
            pthread_create(&writers[i], NULL, writer, NULL);
        }
        for (int i = 0; i < num_readers; i++) {
            pthread_join(readers[i], NULL);
        }
        for (int i = 0; i < num_writers; i++) {
            pthread_join(writers[i], NULL);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double total_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    printf("Tiempo total: %.2f ms\n", total_time / 1e6);

    rwlock_destroy(&lock);
    return EXIT_SUCCESS;
}
