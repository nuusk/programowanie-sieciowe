/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./ipv6-tcp-client.c -o ./ipv6-tcp-client
 * Usage:        ./ipv6-tcp-client SERVER PORT
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int sfd, rc;
  char buf[128];
  struct sockaddr_in6 saddr;

  sfd = socket(PF_INET6, SOCK_STREAM, 0);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin6_family = AF_INET6;
  saddr.sin6_port = htons(atoi(argv[2]));
  inet_pton(AF_INET6, argv[1], &saddr.sin6_addr);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  rc = read(sfd, buf, 128);
  write(1, buf, rc);
  close(sfd);
  return EXIT_SUCCESS;
}
