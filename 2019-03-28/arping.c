/*
 * Compilation:  gcc -Wall ./arpping.c -o ./arpping -lnet -lpcap
 * Usage:        ./arpreq IFNAME HOST
 * NOTE:         This program requires root privileges.
 *
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <libnet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

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

char* errbuf;
pcap_t* handle;

int main(int argc, char** argv) {
  libnet_t* ln;
  u_int32_t target_ip_addr, src_ip_addr;
  u_int8_t bcast_hw_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
           zero_hw_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  struct libnet_ether_addr* src_hw_addr;
  char errbuf[LIBNET_ERRBUF_SIZE];

  ln = libnet_init(LIBNET_LINK, argv[1], errbuf);
  src_ip_addr = libnet_get_ipaddr4(ln);
  src_hw_addr = libnet_get_hwaddr(ln);
  target_ip_addr = libnet_name2addr4(ln, argv[2], LIBNET_RESOLVE);
  libnet_autobuild_arp(ARPOP_REQUEST,                 /* operation type       */
                       src_hw_addr->ether_addr_octet, /* sender hardware addr */
                       (u_int8_t*)&src_ip_addr,       /* sender protocol addr */
                       zero_hw_addr,                  /* target hardware addr */
                       (u_int8_t*)&target_ip_addr,    /* target protocol addr */
                       ln);                           /* libnet context       */
  libnet_autobuild_ethernet(bcast_hw_addr,            /* ethernet destination */
                            ETHERTYPE_ARP,            /* ethertype            */
                            ln);                      /* libnet context       */

  char* frame;
  struct ethhdr* fheader;
  struct arphdr* aheader;
  int cap, tx, rx, idx;
  struct timeval in, out;
  int rtt;
  struct pcap_pkthdr* pheader;

  handle = pcap_create(libnet_getdevice(ln), errbuf);
  pcap_activate(handle);
  while (1) {
    gettimeofday(&out, NULL);
    libnet_write(ln);
    tx++;
    while (1) {
      cap = pcap_next_ex(handle, &pheader, &frame);
      if (cap != 1) continue;
      fheader = (struct ethhdr*)frame;
      if (ntohs(fheader->h_proto) != ETH_P_ARP) continue;
      aheader = (struct arphdr*)(frame + ETH_HLEN);
      if (ntohs(aheader->opcode) != ARPOP_REPLY ||
          memcmp(aheader->sender_ip_addr, (u_int8_t*)&target_ip_addr, 4) != 0)
        continue;
      gettimeofday(&in, NULL);
      // tdiff(&out, &in);
      rtt = out.tv_sec * 1000000 + out.tv_usec;
      printf(
          "%d bytes from %02x:%02x:%02x:%02x:%02x:%02x (%s): index=%d "
          "time=%ld.%ld sec\n",
          pheader->len, aheader->sender_mac_addr[0],
          aheader->sender_mac_addr[1], aheader->sender_mac_addr[2],
          aheader->sender_mac_addr[3], aheader->sender_mac_addr[4],
          aheader->sender_mac_addr[5], argv[2], idx++, rtt / 1000000,
          rtt % 1000000);
      rx++;
      sleep(1);
      break;
    }
  }
  libnet_destroy(ln);
  return EXIT_SUCCESS;
}