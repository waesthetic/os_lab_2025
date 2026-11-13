#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int sockfd, n;
  char ipadr[16];
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

  char mesg[buffer_size + 1];

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind problem");
    exit(1);
  }
  printf("SERVER starts...\n");

  while (1) {
    socklen_t len = sizeof(cliaddr);

    if ((n = recvfrom(sockfd, mesg, buffer_size, 0, (struct sockaddr *)&cliaddr, &len)) < 0) {
      perror("recvfrom");
      exit(1);
    }
    mesg[n] = 0;

    printf("REQUEST %s      FROM %s : %d\n", mesg,
           inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
           ntohs(cliaddr.sin_port));

    if (sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, len) < 0) {
      perror("sendto");
      exit(1);
    }
  }
}
