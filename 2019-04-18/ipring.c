/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./ipring.c -o ./ipring
 * Usage:        ./ipring NEXT_IP_ADDR
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define IPPROTO_CUSTOM 222

int main(int argc, char **argv) {
  int sfd, rc, next_sock;
  char buf[65536], saddr[16], daddr[16];
  char *data;
  socklen_t sl;
  struct sockaddr_in addr;
  struct sockaddr_in next_addr;
  struct iphdr *ip;

  sfd = socket(PF_INET, SOCK_RAW, IPPROTO_CUSTOM);

  memset(&addr, 0, sizeof(addr));
  sl = sizeof(addr);
  rc = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &sl);
  // do zmiennej sl zostanie wpisany rozmiar struktury adresowej
  // nalezy pamietac ze nieptrzetworzonymi gniazdami odbieramy pakiety z
  // naglowkiem
  ip = (struct iphdr *)&buf;
  // poczatek bufora jest rzutowany na strukture iphdr
  while (1) {
    if (ip->protocol == IPPROTO_CUSTOM) {
      // kopiuje adres ip nadawcy
      inet_ntop(AF_INET, &ip->saddr, (char *)&saddr, 16);
      // i odbiorcy
      inet_ntop(AF_INET, &ip->daddr, (char *)&daddr, 16);
      data = (char *)ip + (ip->ihl * 4);
      printf("[%dB] %s -> %s | %s\n", rc - (ip->ihl * 4), saddr, daddr, data);

      next_sock = socket(PF_INET, SOCK_RAW, IPPROTO_CUSTOM);
      memset(&next_addr, 0, sizeof(next_addr));
      next_addr.sin_family = AF_INET;
      next_addr.sin_port = 0;
      next_addr.sin_addr.s_addr = inet_addr(argv[1]);
      sendto(next_sock, buf, strlen(buf), 0, (struct sockaddr *)&next_addr,
             sizeof(next_addr));
      // strlen(argv[2] + 1) -> liczba bajtow do wyslania + 1 (koncowe 0
      // stringa)
      close(next_sock);
    }
  }

  close(sfd);
  return EXIT_SUCCESS;
}
