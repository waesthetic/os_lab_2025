#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

static pid_t *child_pids = NULL;
static int child_count = 0;
static volatile sig_atomic_t timed_out = 0;

static void on_timeout(int sig) {
  (void)sig;
  timed_out = 1;
}

static void KillChildren(void) {
  for (int i = 0; i < child_count; i++) {
    kill(child_pids[i], SIGKILL);
  }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed is a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size is a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum is a positive number\n");
              return 1;
            }
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout is a positive number\n");
              return 1;
            }
            break;
          case 4:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"] \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  child_pids = malloc(sizeof(pid_t) * pnum);
  child_count = pnum;
  GenerateArray(array, array_size, seed);  
  int active_child_processes = 0;
  int chunk = array_size / pnum;
  int pipefd[2];
  char filename[32];

  if (!with_files && pipe(pipefd) == -1) {
    printf("Pipe failed!\n");
    return 1;
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      if (child_pid == 0) {
        unsigned int begin = i * chunk;
        unsigned int end = (i == pnum - 1) ? array_size : begin + chunk;
        struct MinMax local = GetMinMax(array, begin, end);

        if (with_files) {
          sprintf(filename, "min_max_%d.txt", i);
          FILE *file = fopen(filename, "w");
          fprintf(file, "%d %d\n", local.min, local.max);
          fclose(file);
        } else {
          close(pipefd[0]);
          write(pipefd[1], &local, sizeof(local));
          close(pipefd[1]);
        }
        return 0;
      }
      child_pids[i] = child_pid;

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (timeout > 0) {
    signal(SIGALRM, on_timeout);
    alarm(timeout);
  }

  if (!with_files) close(pipefd[1]);

  bool children_killed = false;
  while (active_child_processes > 0) {
    int status;
    pid_t child = waitpid(-1, &status, WNOHANG);

    if (child > 0) {
      if (WIFSIGNALED(status)) {
        printf("Child %d killed by signal %d\n", child, WTERMSIG(status));
      }
      active_child_processes -= 1;
    } else if (child == 0) {
      if (timed_out && !children_killed) {
        KillChildren();
        children_killed = true;
      }
      usleep(1000);
    } else {
      perror("waitpid");
      break;
    }
  }

  if (timeout > 0) alarm(0);

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;
  int received = 0;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      sprintf(filename, "min_max_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file != NULL) {
        if (fscanf(file, "%d %d", &min, &max) == 2) {
          received += 1;
        } else {
          min = INT_MAX;
          max = INT_MIN;
        }
        fclose(file);
        remove(filename);
      }
    } else {
      struct MinMax local;
      ssize_t bytes = read(pipefd[0], &local, sizeof(local));
      if (bytes == (ssize_t)sizeof(local)) {
        min = local.min;
        max = local.max;
        received += 1;
      }
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  if (received == 0) {
    printf("No results: all child processes were killed by timeout\n");
  } else {
    if (received < pnum) {
      printf("Warning: only %d of %d processes finished, result is partial\n",
             received, pnum);
    }
    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
  }
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
