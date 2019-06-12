/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./ipping.c -o ./ipping
 * Usage:        ./ipping DST_IP_ADDR
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

// mozna tym pingowac dowolne adresy w internecie (nie tylko laboratorium)

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

char *argv1;
int tx = 0, rx = 0;

void cleanup() {
  printf("\n--- %s statistics ---\n", argv1);
  printf("%d packets transmitted, %d packets received, %d%% packet loss\n", tx,
         rx, ((tx - rx) * 100) / tx);
}

void stop(int signo) { exit(EXIT_SUCCESS); }

void tdiff(struct timeval *t1, struct timeval *t2) {
  t1->tv_sec = t2->tv_sec - t1->tv_sec;
  if ((t1->tv_usec = t2->tv_usec - t1->tv_usec) < 0) {
    t1->tv_sec--;
    t1->tv_usec += 1000000;
  }
}

uint16_t chksum(uint16_t *addr, int len) {
  int nleft = len, sum = 0;
  uint16_t *w = addr, u = 0, result;

  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(u_char *)&u = *(u_char *)w;
    sum += u;
  }
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

int main(int argc, char **argv) {
  int sfd, rc;
  long rtt;
  socklen_t sl;
  char buf[2048];
  struct timeval out, in;
  struct sockaddr_in snd, rcv;

  // strukture req chcemy wypelnic i wstawic do pola IP zeby wyslac
  struct icmphdr req;

  struct icmphdr *rep;
  struct iphdr *ip;

  atexit(cleanup);
  signal(SIGINT,
         stop);  // do przerwania z klawiatury (ctrl-c. bedzie to funkcja stop)
  argv1 = argv[1];
  sfd = socket(PF_INET, SOCK_RAW,
               IPPROTO_ICMP);  // tworzymy nieprzetworzone gniazdo sieciowe
  memset(&snd, 0, sizeof(snd));
  snd.sin_family = AF_INET;
  snd.sin_port = 0;
  snd.sin_addr.s_addr = inet_addr(argv[1]);
  memset(&req, 0, sizeof(req));
  req.type = ICMP_ECHO;  // ping dziala na tej zasadzie, ze jego typ to icmp
                         // echo, a odpowiedzi echo reply
  req.code = 0;          // "nie pamietam co oznaczalo, ale musi byc 0"
  req.un.echo.id = htons(1234);
  req.un.echo.sequence = tx;
  printf("IPPING %s\n", argv[1]);
  while (1) {
    tx++;
    req.un.echo.sequence = htons(
        tx);  // zawsze zmianeiamy, to kolejny numer sekwencyjny tego zadania
    req.checksum = 0;
    req.checksum =
        chksum((uint16_t *)&req,
               sizeof(req));  // obliczamy sume kontrolna tego naglowka
    gettimeofday(&out, NULL);
    sendto(sfd, &req, sizeof(req), 0, (struct sockaddr *)&snd, sizeof(snd));
    sl = sizeof(rcv);

    // jezeli odbierzemy jakis pakiet ip, to trafia on do bufora buf
    rc = recvfrom(sfd, &buf, sizeof(buf), 0, (struct sockaddr *)&rcv, &sl);
    ip = (struct iphdr *)&buf;
    rep = (struct icmphdr *)((char *)buf + (ip->ihl * 4));
    gettimeofday(&in, NULL);

    // sprawdzamy czy adres ip nadawcy to ten do
    // ktorego wysylalismy
    if (rcv.sin_addr.s_addr == snd.sin_addr.s_addr) {
      // printf("[rep->type]: %d\n", rep->type);
      if (rep->type != ICMP_ECHOREPLY) continue;
      tdiff(&out, &in);
      rtt = out.tv_sec * 1000000 + out.tv_usec;
      rx++;

      // zeby zajrzec do odpowiedzi icmp to msuimy sie
      // przesunac o wartosc ihl * 4 (rozmiar naglowka jest
      // podawany w slowach - 4 bajtach)

      printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%ld.%ld sec\n",
             rc - (ip->ihl * 4), argv[1], ntohs(rep->un.echo.sequence), ip->ttl,
             rtt / 1000000, rtt % 1000);
    }
  }
  close(sfd);
  return EXIT_SUCCESS;
}
