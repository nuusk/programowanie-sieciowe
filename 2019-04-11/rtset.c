/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./rtset.c -o ./rtset
 * Usage:        ./rtset NETIP MASK GWIP
 * NOTE:         This program requires root privileges.
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEV_NUMBER 6  // domyslnie bylo 2, my zmienilismy na 6

struct reqhdr {
  struct nlmsghdr nl;
  struct rtmsg rt;
  char buf[8192];
};

int main(int argc, char **argv) {
  int dev = DEV_NUMBER;
  int sfd, rtlen;
  struct sockaddr_nl snl;
  struct reqhdr req;
  struct rtattr *atp;

  sfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  memset(&snl, 0, sizeof(struct sockaddr_nl));
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = 0;

  memset(&req, 0, sizeof(struct reqhdr));
  rtlen = sizeof(struct rtmsg);
  atp = (struct rtattr *)req.buf;
  atp->rta_type = RTA_DST;
  atp->rta_len = sizeof(struct rtattr) + 4;
  inet_pton(AF_INET, argv[1], ((char *)atp) + sizeof(struct rtattr));
  rtlen += atp->rta_len;

  atp = (struct rtattr *)(((char *)atp) + atp->rta_len);
  atp->rta_type = RTA_GATEWAY;
  atp->rta_len = sizeof(struct rtattr) + 4;
  inet_pton(AF_INET, argv[3], ((char *)atp) + sizeof(struct rtattr));
  rtlen += atp->rta_len;

  atp = (struct rtattr *)(((char *)atp) + atp->rta_len);
  atp->rta_type = RTA_OIF;
  atp->rta_len = sizeof(struct rtattr) + 4;
  memcpy(((char *)atp) + sizeof(struct rtattr), &dev, 4);
  rtlen += atp->rta_len;

  req.nl.nlmsg_len = NLMSG_LENGTH(rtlen);
  req.nl.nlmsg_type = RTM_NEWROUTE;
  req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
  req.nl.nlmsg_seq = 0;
  req.nl.nlmsg_pid = getpid();

  req.rt.rtm_family = AF_INET;
  req.rt.rtm_dst_len = atoi(argv[2]);
  req.rt.rtm_table = RT_TABLE_MAIN;
  req.rt.rtm_protocol =
      RTPROT_STATIC;  // trasa zostala skonfigurowana statycznie (ip route
                      // domyslnie nie ustawia tego)
  req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
  req.rt.rtm_type = RTN_UNICAST;

  sendto(sfd, (void *)&req, req.nl.nlmsg_len, 0, (struct sockaddr *)&snl,
         sizeof(struct sockaddr_nl));  // to wysylamy to jadra

  close(sfd);
  return EXIT_SUCCESS;
}
