/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./socket-buffers.c -o ./socket-buffers
 * Usage:        ./socket-buffers
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int getbuffsize(int sfd, int buffname) {
  int s;
  socklen_t slt = (socklen_t)sizeof(s);
  getsockopt(sfd, SOL_SOCKET, buffname, (void*)&s, &slt);
  return s;
}

void buffsizes(int sfd, int *srb, int *ssb) {
  *srb = getbuffsize(sfd, SO_RCVBUF);
  *ssb = getbuffsize(sfd, SO_SNDBUF);
}

int main(int argc, char **argv) {
  int sfd, srb, ssb;

  sfd = socket(PF_INET, SOCK_STREAM, 0);
  buffsizes(sfd, &srb, &ssb);
  printf("TCP:  RCVBUF = %6d [B]  SNDBUF = %6d [B]\n", srb, ssb);
  close(sfd);
  sfd = socket(AF_INET, SOCK_DGRAM, 0);
  buffsizes(sfd, &srb, &ssb);
  printf("UDP:  RCVBUF = %6d [B]  SNDBUF = %6d [B]\n", srb, ssb);
  close(sfd);
  return EXIT_SUCCESS;
}
