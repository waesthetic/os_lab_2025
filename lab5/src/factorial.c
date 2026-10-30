#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct ThreadArgs {
  long long begin;
  long long end;
  int mod;
};

long long result;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Factorial(void *arg) {
  struct ThreadArgs *args = arg;
  long long product = 1 % args->mod;

  for (long long i = args->begin; i <= args->end; i++) {
    product = product * i % args->mod;
  }

  pthread_mutex_lock(&mutex);
  result = result * product % args->mod;
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(int argc, char **argv) {
  int k = -1;
  int pnum = 0;
  int mod = 0;
  struct option options[] = {
      {"k", required_argument, NULL, 'k'},
      {"pnum", required_argument, NULL, 'p'},
      {"mod", required_argument, NULL, 'm'},
      {NULL, 0, NULL, 0}};

  int option;
  while ((option = getopt_long(argc, argv, "k:", options, NULL)) != -1) {
    if (option != 'k' && option != 'p' && option != 'm') {
      return 1;
    }

    char *end;
    errno = 0;
    long value = strtol(optarg, &end, 10);
    if (errno != 0 || end == optarg || *end != '\0' || value < 0 ||
        value > INT_MAX) {
      fprintf(stderr, "Invalid number: %s\n", optarg);
      return 1;
    }

    switch (option) {
      case 'k':
        k = value;
        break;
      case 'p':
        pnum = value;
        break;
      case 'm':
        mod = value;
        break;
    }
  }

  if (k < 0 || pnum <= 0 || mod <= 0 || optind != argc) {
    fprintf(stderr, "Usage: %s -k 10 --pnum=4 --mod=10\n", argv[0]);
    return 1;
  }

  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);
  struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs) * pnum);
  if (threads == NULL || args == NULL) {
    fprintf(stderr, "Not enough memory\n");
    free(threads);
    free(args);
    return 1;
  }

  result = 1 % mod;
  for (int i = 0; i < pnum; i++) {
    args[i].begin = (long long)k * i / pnum + 1;
    args[i].end = (long long)k * (i + 1) / pnum;
    args[i].mod = mod;

    if (pthread_create(&threads[i], NULL, Factorial, &args[i]) != 0) {
      fprintf(stderr, "pthread_create failed\n");
      exit(1);
    }
  }

  for (int i = 0; i < pnum; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "pthread_join failed\n");
      exit(1);
    }
  }

  printf("Result: %lld\n", result);
  pthread_mutex_destroy(&mutex);
  free(threads);
  free(args);
  return 0;
}
