#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *do_one_thing(void *);
void *do_another_thing(void *);
void do_wrap_up(int);
int common = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int main(void) {
  pthread_t thread1, thread2;

  if (pthread_create(&thread1, NULL, do_one_thing,
                     &common) != 0) {
    fprintf(stderr, "pthread_create failed\n");
    exit(1);
  }

  if (pthread_create(&thread2, NULL, do_another_thing,
                     &common) != 0) {
    fprintf(stderr, "pthread_create failed\n");
    exit(1);
  }

  if (pthread_join(thread1, NULL) != 0) {
    fprintf(stderr, "pthread_join failed\n");
    exit(1);
  }

  if (pthread_join(thread2, NULL) != 0) {
    fprintf(stderr, "pthread_join failed\n");
    exit(1);
  }

  do_wrap_up(common);

  return 0;
}

void *do_one_thing(void *arg) {
  int *pnum_times = arg;
  int i;
  unsigned long k;
  int work;
  for (i = 0; i < 50; i++) {
#ifdef USE_MUTEX
    pthread_mutex_lock(&mut);
#endif
    printf("doing one thing\n");
    work = *pnum_times;
    printf("counter = %d\n", work);
    work++;
    for (k = 0; k < 500000; k++)
      ;
    *pnum_times = work;
#ifdef USE_MUTEX
    pthread_mutex_unlock(&mut);
#endif
  }
  return NULL;
}

void *do_another_thing(void *arg) {
  int *pnum_times = arg;
  int i;
  unsigned long k;
  int work;
  for (i = 0; i < 50; i++) {
#ifdef USE_MUTEX
    pthread_mutex_lock(&mut);
#endif
    printf("doing another thing\n");
    work = *pnum_times;
    printf("counter = %d\n", work);
    work++;
    for (k = 0; k < 500000; k++)
      ;
    *pnum_times = work;
#ifdef USE_MUTEX
    pthread_mutex_unlock(&mut);
#endif
  }
  return NULL;
}

void do_wrap_up(int counter) {
  printf("All done, counter = %d\n", counter);
}
