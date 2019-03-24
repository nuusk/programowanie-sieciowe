/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./ifaddrs.c -o ./ifaddrs
 * Usage:        ./ifaddrs
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int family, n;
  char host[NI_MAXHOST];
  struct ifaddrs *ifaddr, *ifa;

  getifaddrs(&ifaddr);
  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
    if (ifa->ifa_addr == NULL)
      continue;
    family = ifa->ifa_addr->sa_family;
    printf("%-8s %s (%d)\n",
           ifa->ifa_name,
           (family == AF_PACKET) ? "AF_PACKET" :
           (family == AF_INET) ? "AF_INET" :
           (family == AF_INET6) ? "AF_INET6" : "???",
           family);
    if (family == AF_INET || family == AF_INET6) {
      getnameinfo(ifa->ifa_addr,
                  (family == AF_INET) ? sizeof(struct sockaddr_in) :
                  sizeof(struct sockaddr_in6),
                  host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      printf("\t\taddress: <%s>\n", host);
    } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
      struct rtnl_link_stats *stats = ifa->ifa_data;
      printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
             "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
             stats->tx_packets, stats->rx_packets,
             stats->tx_bytes, stats->rx_bytes);
    }
  }
  freeifaddrs(ifaddr);
  exit(EXIT_SUCCESS);
}
