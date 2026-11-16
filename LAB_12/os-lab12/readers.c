#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t rlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wlock = PTHREAD_MUTEX_INITIALIZER;

int readers = 0;
int shared_data = 100;

void *reader(void *arg) {
    int id = *(int *)arg;

    pthread_mutex_lock(&rlock);

    readers++;

    if (readers == 1) {
        pthread_mutex_lock(&wlock);
    }

    pthread_mutex_unlock(&rlock);

    printf("Reader %d: reading value %d\n", id, shared_data);
    sleep(1);
    printf("Reader %d: finished reading\n", id);

    pthread_mutex_lock(&rlock);

    readers--;

    if (readers == 0) {
        pthread_mutex_unlock(&wlock);
    }

    pthread_mutex_unlock(&rlock);

    return NULL;
}

void *writer(void *arg) {
    int id = *(int *)arg;

    pthread_mutex_lock(&wlock);

    shared_data += 10;

    printf("Writer %d: updated value to %d\n", id, shared_data);

    sleep(1);

    pthread_mutex_unlock(&wlock);

    return NULL;
}

int main(void) {
    pthread_t r1, r2, r3, w1;
    int id1 = 1, id2 = 2, id3 = 3, wid = 1;

    pthread_create(&r1, NULL, reader, &id1);
    pthread_create(&r2, NULL, reader, &id2);
    pthread_create(&w1, NULL, writer, &wid);
    pthread_create(&r3, NULL, reader, &id3);

    pthread_join(r1, NULL);
    pthread_join(r2, NULL);
    pthread_join(r3, NULL);
    pthread_join(w1, NULL);

    pthread_mutex_destroy(&rlock);
    pthread_mutex_destroy(&wlock);

    return 0;
}
