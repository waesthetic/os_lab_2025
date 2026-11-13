#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "modulo.h"

struct Server {
  char ip[255];
  int port;
};

struct ServerTask {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  if (str[0] < '0' || str[0] > 
  '9') {
    return false;
  }

  char *end = NULL;
  
  errno = 0;
  unsigned long long number = strtoull(str, &end, 10);
  if (errno != 0 || *end != '\0' || number > UINT64_MAX) {
    return false;
  }

  *val = number;

  return true;
}

void *ThreadServer(void *args) {
  struct ServerTask *task = args;

  struct sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_port = htons(task->server.port);

  if (inet_pton(AF_INET, task->server.ip, &server.sin_addr) != 1) {
    fprintf(stderr, "Invalid server IP\n");
    exit(1);
  }

  int sck = socket(AF_INET, SOCK_STREAM, 0);

  if (sck < 0) {
    fprintf(stderr, "Socket creation failed\n");
    exit(1);
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    close(sck);
    exit(1);
  }

  uint64_t request[3];
  request[0] = task->begin;
  request[1] = task->end;
  request[2] = task->mod;

  size_t sent = 0;

  while (sent < sizeof(request)) {
    ssize_t count = send(sck, (char *)request + sent, sizeof(request) - sent, 0);

    if (count <= 0) {
      fprintf(stderr, "Send failed\n");
      close(sck);
      exit(1);
    }

    sent += count;
  }

  size_t received = 0;

  while (received < sizeof(task->result)) {
    ssize_t count = recv(sck, (char *)&task->result + received, sizeof(task->result) - received, 0);

    if (count <= 0) {
      fprintf(stderr, "Receive failed\n");
      close(sck);
      exit(1);
    }

    received += count;
  }

  close(sck);

  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = UINT64_MAX;
  uint64_t mod = UINT64_MAX;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        if (!ConvertStringToUI64(optarg, &k)) {
          fprintf(stderr, "Invalid factorial argument\n");
          return 1;
        }

        break;
      case 1:
        if (!ConvertStringToUI64(optarg, &mod)) {
          fprintf(stderr, "Invalid modulo argument\n");
          return 1;
        }

        if (mod == 0) {
          fprintf(stderr, "Modulo must be positive");
          return 1;
        }

        break;
      case 2:
        if (strlen(optarg) >= sizeof(servers)) {
          fprintf(stderr, "Servers file path is too long\n");
          return 1;
        }

        strcpy(servers, optarg);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == UINT64_MAX || mod == UINT64_MAX || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  FILE *file = fopen(servers, "r");

  if (file == NULL) {
    fprintf(stderr, "Cannot open servers file\n");
    return 1;
  }

  int servers_num = 0;
  char line[512];

  while (fgets(line, sizeof(line), file) != NULL) {
    servers_num++;
  }

  if (servers_num == 0) {
    fprintf(stderr, "Servers file is empty\n");
    fclose(file);
    return 1;
  }

  rewind(file);

  struct Server *to = malloc(sizeof(struct Server) * servers_num);

  if (to == NULL) {
    fprintf(stderr, "Cannot allocate memory for servers\n");
    fclose(file);
    return 1;
  }

  for (int i = 0; i < servers_num; i++) {
    if (fgets(line, sizeof(line), file) == NULL) {
      fprintf(stderr, "Cannot read server address\n");
      free(to);
      fclose(file);
      return 1;
    }

    if (sscanf(line, "%254[^:]:%d", to[i].ip, &to[i].port) != 2) {
      fprintf(stderr, "Invalid server address\n");
      free(to);
      fclose(file);
      return 1;
    }

    if (to[i].port < 1 || to[i].port > 65535) {
      fprintf(stderr, "Invalid server port\n");
      free(to);
      fclose(file);
      return 1;
    }
  }

  fclose(file);

  pthread_t threads[servers_num];
  struct ServerTask tasks[servers_num];

  uint64_t part = k / servers_num;
  uint64_t remainder = k % servers_num;
  uint64_t current = 1;

  for (int i = 0; i < servers_num; i++) {
    uint64_t size = part;

    if ((uint64_t)i < remainder) {
      size++;
    }

    tasks[i].server = to[i];
    tasks[i].begin = 1;
    tasks[i].end = 0;
    tasks[i].mod = mod;

    if (size > 0) {
      tasks[i].begin = current;
      tasks[i].end = current + size - 1;
      current += size;
    }

    if (pthread_create(&threads[i], NULL, ThreadServer, &tasks[i]) != 0) {
      fprintf(stderr, "Error: pthread_create failed\n");
      exit(1);
    }
  }

  uint64_t total = 1 % mod;

  for (int i = 0; i < servers_num; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error: pthread_join failed\n");
      exit(1);
    }

    total = MultModulo(total, tasks[i].result, mod);
  }

  printf("Result: %llu\n", (unsigned long long)total);

  free(to);

  return 0;
}
