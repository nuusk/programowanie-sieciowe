/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./irsend.c -o ./irsend
 * Usage:        ./irsend MAC INAME IADDR | ./irsend MAC RTDST RTMSK RTGIP
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IFNAME "em1" /* IFNAME */
#define ETH_P_CUSTOM 0x8888
#define IRI_T_ADDRESS 0
#define IRI_T_ROUTE   1

struct ifrtinfo {
  int iri_type;
  char iri_iname[16];
  struct sockaddr_in iri_iaddr; /* IP address */
  struct sockaddr_in iri_rtdst; /* dst. IP address */
  struct sockaddr_in iri_rtmsk; /* dst. netmask */
  struct sockaddr_in iri_rtgip; /* gateway IP */
};

int main(int argc, char** argv) {
  int sfd, ifindex;
  char* frame;
  char* fdata;
  struct ethhdr* fhead;
  struct ifreq ifr;
  struct sockaddr_ll sall;
  struct ifrtinfo iri;

  if (argc == 1) {
    printf("usage: mac iname iaddr\tOR\tmac rtdst rtmsk rtgip\n");
    return EXIT_SUCCESS;
  }
  memset(&iri, 0, sizeof(iri));
  if (argc == 4) {
    iri.iri_type = IRI_T_ADDRESS;
    strncpy(iri.iri_iname, argv[2], 16);
    iri.iri_iaddr.sin_family = AF_INET;
    iri.iri_iaddr.sin_addr.s_addr = inet_addr(argv[3]);
    iri.iri_iaddr.sin_port = 0;
  }
  if (argc == 5) {
    iri.iri_type = IRI_T_ROUTE;
    iri.iri_rtdst.sin_family = AF_INET;
    iri.iri_rtdst.sin_addr.s_addr = inet_addr(argv[2]);
    iri.iri_rtdst.sin_port = 0;
    iri.iri_rtmsk.sin_family = AF_INET;
    iri.iri_rtmsk.sin_addr.s_addr = inet_addr(argv[3]);
    iri.iri_rtmsk.sin_port = 0;
    iri.iri_rtgip.sin_family = AF_INET;
    iri.iri_rtgip.sin_addr.s_addr = inet_addr(argv[4]);
    iri.iri_rtgip.sin_port = 0;
  }
  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
  frame = malloc(ETH_FRAME_LEN);
  memset(frame, 0, ETH_FRAME_LEN);
  fhead = (struct ethhdr*) frame;
  fdata = frame + ETH_HLEN;
  strncpy(ifr.ifr_name, IFNAME, IFNAMSIZ);
  ioctl(sfd, SIOCGIFINDEX, &ifr);
  ifindex = ifr.ifr_ifindex;
  ioctl(sfd, SIOCGIFHWADDR, &ifr);
  memset(&sall, 0, sizeof(struct sockaddr_ll));
  sall.sll_family = AF_PACKET;
  sall.sll_protocol = htons(ETH_P_CUSTOM);
  sall.sll_ifindex = ifindex;
  sall.sll_hatype = ARPHRD_ETHER;
  sall.sll_pkttype = PACKET_OUTGOING;
  sall.sll_halen = ETH_ALEN;
  sscanf(argv[1], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &sall.sll_addr[0], &sall.sll_addr[1], &sall.sll_addr[2],
         &sall.sll_addr[3], &sall.sll_addr[4], &sall.sll_addr[5]);
  memcpy(fhead->h_dest, &sall.sll_addr, ETH_ALEN);
  memcpy(fhead->h_source, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);
  fhead->h_proto = htons(ETH_P_CUSTOM);
  memcpy(fdata, &iri, sizeof(iri));
  sendto(sfd, frame, ETH_HLEN + sizeof(iri), 0,
         (struct sockaddr*) &sall, sizeof(struct sockaddr_ll));
  free(frame);
  close(sfd);
  return EXIT_SUCCESS;
}
