#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  int nread;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if (argc != 3) {
    printf("Usage: %s <port> <buffer_size>\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  int buffer_size = atoi(argv[2]);

  if (port < 1 || port > 65535) {
    printf("Port must be between 1 and 65535\n");
    return 1;
  }

  if (buffer_size <= 0) {
    printf("Buffer size must be positive\n");
    return 1;
  }

  char buf[buffer_size];

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(lfd, (struct sockaddr *)&servaddr, kSize) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(lfd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  while (1) {
    socklen_t clilen = sizeof(cliaddr);

    if ((cfd = accept(lfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      exit(1);
    }
    printf("connection established\n");

    while ((nread = read(cfd, buf, buffer_size)) > 0) {
      write(1, &buf, nread);
    }

    if (nread == -1) {
      perror("read");
      exit(1);
    }
    close(cfd);
  }
}
