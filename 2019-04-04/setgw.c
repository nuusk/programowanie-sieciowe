/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./setgw.c -o ./setgw
 * Usage:        ./setgw GWIP
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int sfd;
  struct rtentry route;
  struct sockaddr_in* addr;

  sfd = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&route, 0, sizeof(route));
  addr = (struct sockaddr_in*)&route.rt_gateway;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(argv[1]);
  addr = (struct sockaddr_in*)&route.rt_dst;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr = (struct sockaddr_in*)&route.rt_genmask;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  route.rt_flags = RTF_UP | RTF_GATEWAY;
  route.rt_metric = 0;
  ioctl(sfd, SIOCADDRT, &route);
  close(sfd);
  return EXIT_SUCCESS;
}

// zeby usunac tak dodany wpis mozna wykorzystac
// $ route del default gw <IP>