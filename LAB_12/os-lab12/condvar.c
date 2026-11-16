#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int message_ready = 0;
int message = 0;

void *producer(void *arg) {
    sleep(1);

    pthread_mutex_lock(&lock);

    message = 42;
    message_ready = 1;

    printf("Producer: message is ready\n");

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}

void *consumer(void *arg) {
    pthread_mutex_lock(&lock);

    while (message_ready == 0) {
        printf("Consumer: waiting for a message...\n");
        pthread_cond_wait(&cond, &lock);
    }

    printf("Consumer: received message %d\n", message);

    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void) {
    pthread_t p, c;

    pthread_create(&c, NULL, consumer, NULL);
    pthread_create(&p, NULL, producer, NULL);

    pthread_join(p, NULL);
    pthread_join(c, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}
