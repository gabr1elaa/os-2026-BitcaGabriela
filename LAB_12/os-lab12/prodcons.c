#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE 4
#define ITEMS 8

int buf[SIZE];
int in = 0, out = 0, count = 0;

sem_t empty, full;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *arg) {
    for (int item = 1; item <= ITEMS; item++) {
        sem_wait(&empty);
        pthread_mutex_lock(&lock);

        buf[in] = item;
        in = (in + 1) % SIZE;
        count++;

        printf("put %d (in buffer: %d)\n", item, count);

        pthread_mutex_unlock(&lock);

        sem_post(&full);
    }

    return NULL;
}

void *consumer(void *arg) {
    for (int k = 0; k < ITEMS; k++) {
        sem_wait(&full);

        pthread_mutex_lock(&lock);

        int item = buf[out];
        out = (out + 1) % SIZE;
        count--;

        printf(" take %d (in buffer: %d)\n", item, count);

        pthread_mutex_unlock(&lock);

        sem_post(&empty);

        usleep(300000);
    }

    return NULL;
}

int main(void) {
    sem_init(&empty, 0, SIZE);
    sem_init(&full, 0, 0);

    pthread_t p, c;

    pthread_create(&c, NULL, consumer, NULL);
    pthread_create(&p, NULL, producer, NULL);

    pthread_join(p, NULL);
    pthread_join(c, NULL);

    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
