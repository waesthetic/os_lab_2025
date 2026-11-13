#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "modulo.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1 % args->mod;
  
  for (uint64_t i = args->begin; i <= args->end; i++) {
    ans = MultModulo(ans, i, args->mod);
  }

  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = args;

  fargs->result = Factorial(fargs);

  return NULL;
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);

        if (port < 1 || port > 65535) {
          fprintf(stderr, "Port must be between 1 and 65535\n");
          return 1;
        }

        break;
      case 1:
        tnum = atoi(optarg);
        
        if (tnum <= 0) {
          fprintf(stderr, "Thread count must be positive\n");
          return 1;
        }
        
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }

  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);

  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (true) {
      char from_client[sizeof(uint64_t) * 3];
      size_t received = 0;

      while(received < sizeof(from_client)) {
        ssize_t count = recv(client_fd, from_client + received, sizeof(from_client) - received, 0);

        if (count <= 0) {
          break;
        }

        received += count;
      }

      if (received != sizeof(from_client)) {
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      if (mod == 0) {
        fprintf(stderr, "Modulo must be positive\n");
        break;
      }

      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);

      struct FactorialArgs args[tnum];
      for (int i = 0; i < tnum; i++) {
        uint64_t count = 0;

        if (begin <= end) {
          count = end - begin + 1;
        }

        uint64_t part = count / tnum;
        uint64_t remainder = count % tnum;
        uint64_t offset = i * part;

        if ((uint64_t)i < remainder) {
          offset += i;
        } else {
          offset += remainder;
        }

        uint64_t size = part;

        if ((uint64_t)i < remainder) {
          size++;
        }

        args[i].begin = 1;
        args[i].end = 0;
        args[i].mod = mod;

        if (size > 0) {
          args[i].begin = begin + offset;
          args[i].end = args[i].begin + size - 1;
        }

        if (pthread_create(&threads[i], NULL, ThreadFactorial,
                           (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1 % mod;
      for (int i = 0; i < tnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
          fprintf(stderr, "Error: pthread_join failed!\n");
          return 1;
        }

        total = MultModulo(total, args[i].result, mod);
      }

      printf("Total: %llu\n", total);

      size_t sent = 0;

      while (sent < sizeof(total)) {
        ssize_t count = send(client_fd, (char *)&total + sent, sizeof(total) - sent, 0);

        if (count <= 0) {
          break;
        }

        sent += count;
      }

      if (sent != sizeof(total)) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}
