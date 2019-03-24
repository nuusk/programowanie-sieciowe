/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./ethsend.c -o ./ethsend
 * Usage:        ./ethsend INTERFACE DST_HW_ADDR DATA
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

// przesyłając po ethernecie ramki uzupełniane są zerami
// po wifi nie ma to miejsca

#define ETH_P_CUSTOM 0x8888

int main(int argc, char** argv) {
  int sfd, ifindex;
  char* frame;
  char* fdata;
  struct ethhdr* fhead;
  struct ifreq ifr;
  struct sockaddr_ll sall;

  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CUSTOM));
  frame = malloc(ETH_FRAME_LEN);
  memset(frame, 0, ETH_FRAME_LEN);
  fhead = (struct ethhdr*) frame;
  fdata = frame + ETH_HLEN;
  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
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
  sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &sall.sll_addr[0], &sall.sll_addr[1], &sall.sll_addr[2],
         &sall.sll_addr[3], &sall.sll_addr[4], &sall.sll_addr[5]);
  memcpy(fhead->h_dest, &sall.sll_addr, ETH_ALEN);
  memcpy(fhead->h_source, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);
  fhead->h_proto = htons(ETH_P_CUSTOM);
  memcpy(fdata, argv[3], strlen(argv[3]) + 1);
  sendto(sfd, frame, ETH_HLEN + strlen(argv[3]) + 1, 0,
         (struct sockaddr*) &sall, sizeof(struct sockaddr_ll));
  free(frame);
  close(sfd);
  return EXIT_SUCCESS;
}
