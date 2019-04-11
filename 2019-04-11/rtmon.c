/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./rtmon.c -o ./rtmon
 * Usage:        ./rtmon
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int sfd, rclen, nllen, atlen;
  char buf[8192];
  char *ptr;
  char dst[32], msk[32], gwy[32], dev[32];
  struct sockaddr_nl snl;
  struct nlmsghdr *nlp;
  struct rtmsg *rtp;
  struct rtattr *atp;

  sfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);  // tworzymy gniazdo
  memset(&snl, 0, sizeof(struct sockaddr_nl));
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = getpid();  // ustawiamy nasz wlasny adres
  snl.nl_groups =
      RTMGRP_IPV4_ROUTE |
      RTMGRP_NOTIFY;  // chcemy zapisac sie do grupy dotyczacej tras w sieci
                      // ipv4 i chcemy byc informowani za kzdym razem gdy w tych
                      // trasach sie cos zmieni
  bind(sfd, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));

  while (1) {
    memset(&buf, 0, sizeof(buf));
    ptr = buf;
    nllen = 0;
    do {  // odbiramy komunikaty od jadra
      rclen = recv(sfd, ptr, sizeof(buf) - nllen, 0);
      nlp = (struct nlmsghdr *)ptr;
      ptr += rclen;
      nllen += rclen;
    } while (nlp->nlmsg_type == NLMSG_DONE);

    nlp = (struct nlmsghdr *)buf;
    for (; NLMSG_OK(nlp, nllen); nlp = NLMSG_NEXT(nlp, nllen)) {
      rtp = (struct rtmsg *)NLMSG_DATA(nlp);
      if (rtp->rtm_table != RT_TABLE_MAIN) continue;
      atp = (struct rtattr *)RTM_RTA(rtp);
      atlen = RTM_PAYLOAD(nlp);
      memset(dst, 0, sizeof(dst));
      memset(msk, 0, sizeof(msk));
      memset(gwy, 0, sizeof(gwy));
      memset(dev, 0, sizeof(dev));
      for (; RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) {
        switch (atp->rta_type) {
          case RTA_DST:
            inet_ntop(AF_INET, RTA_DATA(atp), dst, sizeof(dst));
            break;
          case RTA_GATEWAY:
            inet_ntop(AF_INET, RTA_DATA(atp), gwy, sizeof(gwy));
            break;
          case RTA_OIF:
            sprintf(dev, "%d", *((int *)RTA_DATA(atp)));
            break;
        }
      }
      sprintf(msk, "%d", rtp->rtm_dst_len);
      if (nlp->nlmsg_type == RTM_DELROUTE)
        printf("[DEL] ");
      else if (nlp->nlmsg_type == RTM_NEWROUTE)
        printf("[ADD] ");
      if (strlen(dst) == 0)
        printf("default via %s dev %s\n", gwy, dev);
      else if (strlen(gwy) == 0)
        printf("%s/%s dev %s\n", dst, msk, dev);
      else
        printf("dst %s/%s gwy %s dev %s\n", dst, msk, gwy, dev);
    }
  }

  close(sfd);
  return EXIT_SUCCESS;
}
