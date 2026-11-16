#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1(void *arg) {
    printf("Thread 1: waiting for m1\n");
    pthread_mutex_lock(&m1);
    printf("Thread 1: acquired m1\n");

    sleep(1);

    printf("Thread 1: waiting for m2\n");
    pthread_mutex_lock(&m2);
    printf("Thread 1: acquired m2\n");

    printf("Thread 1: critical section\n");

    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);

    return NULL;
}

void *thread2(void *arg) {
    printf("Thread 2: waiting for m1\n");
pthread_mutex_lock(&m1);
printf("Thread 2: acquired m1\n");

sleep(1);

printf("Thread 2: waiting for m2\n");
pthread_mutex_lock(&m2);
printf("Thread 2: acquired m2\n");
    printf("Thread 2: critical section\n");

    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);

    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&m1);
    pthread_mutex_destroy(&m2);

    printf("Finished without deadlock\n");

    return 0;
}
