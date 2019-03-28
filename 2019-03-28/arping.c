/*
 * Compilation:  gcc -Wall ./arping.c -o ./arping -lnet
 * Usage:        ./arping IFNAME HOST
 * NOTE:         This program requires root privileges.
 */

#include <libnet.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

char *errbuf_pcap;
pcap_t *handle;

struct arphdr {
  u_int16_t ftype;
  u_int16_t ptype;
  u_int8_t flen;
  u_int8_t plen;
  u_int16_t opcode;
  u_int8_t sender_mac_addr[6];
  u_int8_t sender_ip_addr[4];
  u_int8_t target_mac_addr[6];
  u_int8_t target_ip_addr[4];
};

void cleanup() {
  printf("\nFinished.\n");
  pcap_close(handle);
  free(errbuf_pcap);
}

void stop(int signo) { exit(EXIT_SUCCESS); }

int main(int argc, char** argv) {
  atexit(cleanup);
  signal(SIGINT, stop);
  printf("Starting\n");

  libnet_t *ln;
  u_int32_t target_ip_addr, src_ip_addr;
  u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
           zero_hw_addr[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  struct libnet_ether_addr* src_hw_addr;
  char errbuf[LIBNET_ERRBUF_SIZE];
  struct ethhdr *fheader;
  struct pcap_pkthdr **pheader;
  const u_char **frame;
  struct arphdr *aheader;
  int cap, tx = 0;


  ln = libnet_init(LIBNET_LINK, argv[1], errbuf);
  src_ip_addr = libnet_get_ipaddr4(ln);
  src_hw_addr = libnet_get_hwaddr(ln);
  target_ip_addr = libnet_name2addr4(ln, argv[2], LIBNET_RESOLVE);
  libnet_autobuild_arp(
    ARPOP_REQUEST,                   /* operation type       */
    src_hw_addr->ether_addr_octet,   /* sender hardware addr */
    (u_int8_t*) &src_ip_addr,        /* sender protocol addr */
    zero_hw_addr,                    /* target hardware addr */
    (u_int8_t*) &target_ip_addr,     /* target protocol addr */
    ln);                             /* libnet context       */
  libnet_autobuild_ethernet(
    bcast_hw_addr,                   /* ethernet destination */
    ETHERTYPE_ARP,                   /* ethertype            */
    ln);                             /* libnet context       */

  // libnet_destroy(ln);

  errbuf_pcap = malloc(PCAP_ERRBUF_SIZE);
  handle = pcap_create(argv[1], errbuf_pcap);

  // pcap_set_promisc(handle, 1);
  // pcap_set_snaplen(handle, 65535);
  pcap_activate(handle);
  printf("Started ARPING %s via %s\n", argv[2], argv[1]);

  while(1) {
    // gettimeofday()
    libnet_write(ln);
    tx++;
    while(1) {
      cap = pcap_next_ex(handle, &pheader, &frame);
      if (cap != 1) continue;
      fheader = (struct ethhdr *)frame;
      if (ntohs(fheader->h_proto) != ETH_P_ARP) continue;
      aheader = (struct arphdr*)(frame + ETH_HLEN);
      if (ntohs(aheader->opcode) != ARPOP_REPLY || memcmp(aheader->sender_ip_addr, (u_int8_t*)&target_ip_addr, 4) != 0) continue;
      
      printf("%d bytes from %02x:%02x:%02x:%02x:%02x:%02x (%s)\n", )

    }
  }

  return EXIT_SUCCESS;
}
