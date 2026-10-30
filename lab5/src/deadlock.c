#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Worker(void *arg) {
  (void)arg;
  puts("Worker: waiting for mutex");
  pthread_mutex_lock(&mutex);
  puts("Worker: mutex acquired");
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(void) {
  pthread_t thread;

  pthread_mutex_lock(&mutex);
  puts("Main: mutex acquired");

  if (pthread_create(&thread, NULL, Worker, NULL) != 0) {
    fprintf(stderr, "pthread_create failed\n");
    return 1;
  }

  puts("Main: waiting for worker");
  pthread_join(thread, NULL);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
  return 0;
}
