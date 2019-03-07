#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int sfd;
  struct ifreq ifr;
  sfd = socket(PF_INET, SOCK_DGRAM, 0);

  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);

  ioctl(sfd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_PROMISC;
  ioctl(sfd, SIOCSIFFLAGS, &ifr);

  ifr.ifr_flags &= ~IFF_PROMISC;
  ioctl(sfd, SIOCSIFFLAGS, &ifr);

  return 0;
}
