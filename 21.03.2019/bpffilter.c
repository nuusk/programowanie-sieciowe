/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./bpffilter.c -o ./bpffilter
 * Usage:        ./bpffilter INTERFACE
 * NOTE #1:      This program requires root privileges.
 * NOTE #2:      Use FILTER below to switch between filters.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <linux/filter.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define FILTER dns_filter

int sfd;
struct ifreq ifr;

struct sock_filter dns_filter[] = {
  { 0x28, 0, 0, 0x0000000c },
  { 0x15, 0, 4, 0x000086dd },
  { 0x30, 0, 0, 0x00000014 },
  { 0x15, 0, 11, 0x00000011 },
  { 0x28, 0, 0, 0x00000038 },
  { 0x15, 8, 9, 0x00000035 },
  { 0x15, 0, 8, 0x00000800 },
  { 0x30, 0, 0, 0x00000017 },
  { 0x15, 0, 6, 0x00000011 },
  { 0x28, 0, 0, 0x00000014 },
  { 0x45, 4, 0, 0x00001fff },
  { 0xb1, 0, 0, 0x0000000e },
  { 0x48, 0, 0, 0x00000010 },
  { 0x15, 0, 1, 0x00000035 },
  { 0x6, 0, 0, 0x00040000 },
  { 0x6, 0, 0, 0x00000000 }
};

struct sock_filter arp_filter[] = {  /* tcpdump -dd arp */
  { 0x28, 0, 0, 0x0000000c },
  { 0x15, 0, 1, 0x00000806 },
  { 0x06, 0, 0, 0x0000ffff },
  { 0x06, 0, 0, 0x00000000 }
};

struct sock_filter icmp_filter[] = {  /* tcpdump -dd icmp */
  { 0x28, 0, 0, 0x0000000c },
  { 0x15, 0, 3, 0x00000800 },
  { 0x30, 0, 0, 0x00000017 },
  { 0x15, 0, 1, 0x00000001 },
  { 0x06, 0, 0, 0x0000ffff },
  { 0x06, 0, 0, 0x00000000 }
};

struct sock_fprog bpf = {
  .len = (sizeof(FILTER) / sizeof(FILTER[0])),
  .filter = FILTER
};

void cleanup() {
  ifr.ifr_flags &= ~IFF_PROMISC;
  ioctl(sfd, SIOCSIFFLAGS, &ifr);
  close(sfd);
}

void stop(int signo) { exit(EXIT_SUCCESS); }

const static char* etype(unsigned int ethertype) {
  static char buf[16];
  switch (ntohs(ethertype) & 0xFFFFU) {
    case 0x0001: return "802_3";
    case 0x0002: return "AX25";
    case 0x0003: return "ALL";
    case 0x0060: return "LOOP";
    case 0x0800: return "IP";
    case 0x0806: return "ARP";
    case 0x8100: return "8021Q";
    case 0x88A8: return "8021AD";
    default:     snprintf(buf, sizeof(buf), "0x%04x",
                          ntohs(ethertype) & 0xFFFFU);
                 return (const char*) buf;
  }
}

static const char *ptype(const unsigned int pkttype) {
  static char buf[16];
  switch (pkttype) {
    case PACKET_HOST:      return "INCOMING";
    case PACKET_BROADCAST: return "BROADCAST";
    case PACKET_MULTICAST: return "MULTICAST";
    case PACKET_OTHERHOST: return "OTHERHOST";
    case PACKET_OUTGOING:  return "OUTGOING";
    default:               snprintf(buf, sizeof(buf), "0x%02x", pkttype);
                           return (const char*) buf;
  }
}

void printframe(char* frame, int len, struct sockaddr_ll* sall) {
  int i, j;
  unsigned char ch;
  struct ethhdr* fhead;

  fhead = (struct ethhdr*) frame;
  printf("[%dB] %02x:%02x:%02x:%02x:%02x:%02x -> ", (int)len,
         fhead->h_source[0], fhead->h_source[1], fhead->h_source[2],
         fhead->h_source[3], fhead->h_source[4], fhead->h_source[5]);
  printf("%02x:%02x:%02x:%02x:%02x:%02x | ",
         fhead->h_dest[0], fhead->h_dest[1], fhead->h_dest[2],
         fhead->h_dest[3], fhead->h_dest[4], fhead->h_dest[5]);
  printf("%s (%s)\n", etype(sall->sll_protocol), ptype(sall->sll_pkttype));
  for (i = 0; i < len; i += 16) {
    printf("0x%04x:  ", i);
    for (j = 0; j < 16; j++) {
      ch = (i + j < len) ? frame[i + j] : 0;
      if (i + j < len) printf("%02x ", ch);
      else printf("   ");
    }
    printf(" ");
    for (j = 0; j < 16; j++) {
      ch = (i + j < len) ? frame[i + j] : ' ';
      if (!isprint(ch)) ch = '.';
      printf("%c", ch);
    }
    printf("\n");
  }
  printf( "\n");
}

int main(int argc, char** argv) {
  socklen_t sl;
  ssize_t len;
  char* frame;
  struct sockaddr_ll sall;

  atexit(cleanup);
  signal(SIGINT, stop);
  sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  setsockopt(sfd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
  strncpy(ifr.ifr_name, argv[1], IFNAMSIZ);
  ioctl(sfd, SIOCGIFFLAGS, &ifr);
  ifr.ifr_flags |= IFF_PROMISC;
  ioctl(sfd, SIOCSIFFLAGS, &ifr);
  while(1) {
    frame = malloc(ETH_FRAME_LEN);
    memset(frame, 0, ETH_FRAME_LEN);
    memset(&sall, 0, sizeof(struct sockaddr_ll));
    sl = sizeof(struct sockaddr_ll);
    len = recvfrom(sfd, frame, ETH_FRAME_LEN, 0, (struct sockaddr*)&sall, &sl);
    printframe(frame, len, &sall);
    free(frame);
  }
}
