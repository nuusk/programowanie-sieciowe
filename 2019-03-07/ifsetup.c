/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./ifsetup.c -o ./ifsetup
 * Usage:        ./ifsetup IFNAME IP
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// ./a.out <interfejs> <adres ip> <maska>

int main(int argc, char** argv) {
  int sfd;
  struct ifreq ifr;
  struct sockaddr_in* sin;

  if (argc != 4) {
    printf("Invalid arguments. Use:\n%s <interface> <ip address> <mask>", argv[0], argv[0]);
    exit(1);
  }

  sfd = socket(PF_INET, SOCK_DGRAM, 0);
  strncpy(ifr.ifr_name, argv[1], strlen(argv[1]) + 1);

  sin = (struct sockaddr_in*) &ifr.ifr_addr;
  memset(sin, 0, sizeof(struct sockaddr_in));
  sin->sin_family = AF_INET;
  sin->sin_port = 0;
  sin->sin_addr.s_addr = inet_addr(argv[2]);
  ioctl(sfd, SIOCSIFADDR, &ifr);

  sin->sin_addr.s_addr = inet_addr(argv[3]);
  ioctl(sfd, SIOCSIFNETMASK, &ifr);

  ioctl(sfd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
  ioctl(sfd, SIOCSIFFLAGS, &ifr);
  close(sfd);
  return EXIT_SUCCESS;
}
