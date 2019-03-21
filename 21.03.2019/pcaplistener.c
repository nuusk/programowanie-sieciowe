/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./pcaplistener.c -o ./pcaplistener -lpcap
 * Usage:        ./pcaplistener INTERFACE
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <pcap.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>


char* errbuf;
pcap_t* handle;

void cleanup() {
  pcap_close(handle);
  free(errbuf);
}

void stop(int signo) {
  exit(EXIT_SUCCESS);
}

int ip_counter = 0;
int arp_counter = 0;
int tcp_counter = 0;
int upd_counter = 0;
int other_counter = 0;

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

/* Bytes - wskaznik na cala ramke, mamy bufor z ramka 
  rzutowac na strukture ethhdr jest w jakims smiesznym pliku zdefiniowana niestety linux
  odczytac h_proto czyli ethertype uuu jest zapisywany w porzadku sieciowym 
  ntohs na h_proto w takim razie trzeba zasotsowac
  zeby sie dowiedziec co to sa to mamy te smieszne cyferki 0x0800 ip 0x0806 arp etc.
  Jezeli to sie okaze ze to jest IP to trzeba sie przeniesc o rozmiar struktury ethhdr 
  i znowu zrzutowac na iphdr i teraz interesuje nas pole protocol z struktury iphdr.
  6- > TCP , 17 -> UDP, wszystko inne liczyc jako others
*/


void trap(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
  struct ethhdr *eth = (struct ethhdr*) bytes;
  if (memcmp(etype(eth->h_proto), "IP", sizeof(eth->h_proto))) {
    ip_counter++;

    struct iphdr *iphdr = (struct iphdr*) bytes + sizeof(eth);
    if(memcmp(iphdr->protocol, "6"))
  } else if(memcmp(etype(eth->h_proto), "ARP", sizeof(eth->h_proto))) {
    arp_counter ++;
  } else {
    other_counter++;
  }
  printf("[%dB of %dB]\n", h->caplen, h->len);
}

int main(int argc, char** argv) {
  atexit(cleanup);
  signal(SIGINT, stop);
  errbuf = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(argv[1], errbuf);
  pcap_set_promisc(handle, 1);
  pcap_set_snaplen(handle, 65535);
  pcap_activate(handle);
  pcap_loop(handle, -1, trap, NULL);
}