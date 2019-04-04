/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./arpget.c -o ./arpget
 * Usage:        ./arpget IFNAME IP
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

// zeby to przetestowac to mozna zrobic arp -n i sprawdzic jakie mamy wpisy w
// tablicy arp. wybrac jakis adres ip i wykorzystac go, zeby zdobyc pozadany
// adres mac

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int fd;
  struct arpreq arq;
  struct sockaddr_in* sin;
  unsigned char* mac;

  memset(&arq, 0, sizeof(arq));
  strncpy(arq.arp_dev, argv[1], strlen(argv[1]) + 1);
  sin = (struct sockaddr_in*)&arq.arp_pa;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = inet_addr(argv[2]);
  fd = socket(PF_INET, SOCK_DGRAM, 0);
  ioctl(fd, SIOCGARP, &arq);
  printf("%s\t", inet_ntoa(sin->sin_addr));
  if (arq.arp_flags &
      ATF_COM) {  // pozyskamy adres mac jesli jest ustaiwona flaga complete
                  // jezeli jej nie ma to wyswietlamy komunikat incomplete.
    mac = (unsigned char*)&arq.arp_ha
              .sa_data[0];  // adres mac znajduje sie w sa_data (6 bajtow)
    printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3],
           mac[4], mac[5]);
    printf("\tC");
    if (arq.arp_flags & ATF_PERM) printf("M");  // pernament
    if (arq.arp_flags & ATF_PUBL) printf("P");  // publish
    if (arq.arp_flags & ATF_USETRAILERS) printf("T");
    if (arq.arp_flags & ATF_NETMASK) printf("N");
  } else
    printf("(incomplete)");
  printf("\n");
  close(fd);
  return EXIT_SUCCESS;
}
