/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./rtget.c -o ./rtget
 * Usage:        ./rtget
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
#include <sys/types.h>
#include <unistd.h>

struct reqhdr {
  struct nlmsghdr nl;  // to jest w kazdym komunikacie zawsze
  struct rtmsg rt;     // to rowniez (w tej kolejnosci)
};

// po tych dwoch strukturach moze byc wiele struktur rtattr

int main(int argc, char **argv) {
  int attr_counter = 0;
  int header_counter = 0;
  int sfd, rclen, nllen, atlen;
  char *ptr;
  char buf[8192];
  char dst[32], msk[32], gwy[32], dev[32];
  struct sockaddr_nl snl;
  struct reqhdr req;
  struct nlmsghdr *nlp;
  struct rtmsg *rtp;
  struct rtattr *atp;

  sfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  memset(&snl, 0, sizeof(struct sockaddr_nl));
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = 0;  // pid 0 identyfikator jądra
  memset(&req, 0, sizeof(struct reqhdr));
  req.nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.nl.nlmsg_type = RTM_GETROUTE;  // chcemy pobrac tablice routingu
  req.nl.nlmsg_flags =
      NLM_F_REQUEST |  // to jest zadanie na ktore jadro ma nam odpowiedziec
      NLM_F_ROOT;
  req.nl.nlmsg_seq = 0;
  req.nl.nlmsg_pid = getpid();       // podajemy pid procesu
  req.rt.rtm_family = AF_INET;       // ipv4
  req.rt.rtm_table = RT_TABLE_MAIN;  // z domyslnej tablicy main
  sendto(sfd, (void *)&req, sizeof(struct reqhdr), 0, (struct sockaddr *)&snl,
         sizeof(struct sockaddr_nl));  // po wypelnieniu struktur mozemy wyslac
                                       // je przez nasze gniazdo

  // teraz musimy zaczac odbierac zadanie

  memset(&buf, 0, sizeof(buf));
  ptr = buf;
  nllen = 0;
  do {
    rclen = recv(sfd, ptr, sizeof(buf) - nllen, 0);
    nlp = (struct nlmsghdr *)ptr;
    ptr += rclen;
    nllen += rclen;
  } while (nlp->nlmsg_type ==
           NLMSG_DONE);  // na samym koncu strumienia struktur
                         // znajduje sie komunikat NLMSG_DONE

  nlp = (struct nlmsghdr *)buf;
  for (; NLMSG_OK(nlp, nllen); nlp = NLMSG_NEXT(nlp, nllen)) {
    // printf("header\t\t[%d]\n", header_counter++);
    rtp = (struct rtmsg *)NLMSG_DATA(nlp);
    if (rtp->rtm_table != RT_TABLE_MAIN) continue;
    atp = (struct rtattr *)RTM_RTA(rtp);
    atlen = RTM_PAYLOAD(nlp);
    memset(dst, 0, sizeof(dst));
    memset(msk, 0, sizeof(msk));
    memset(gwy, 0, sizeof(gwy));
    memset(dev, 0, sizeof(dev));
    for (; RTA_OK(atp, atlen); atp = RTA_NEXT(atp, atlen)) {
      // printf("attribute\t[%d]\n", attr_counter++);
      switch (atp->rta_type) {
        case RTA_DST:  // adres ip sieci docelowej. kopiujemy go do zmiennej dst
                       // (to bufor w ktorej mam adres ip sieci docelowej)
          inet_ntop(AF_INET, RTA_DATA(atp), dst, sizeof(dst));
          break;
        case RTA_GATEWAY:  // adres ip bramy
          inet_ntop(AF_INET, RTA_DATA(atp), gwy, sizeof(gwy));
          break;
        case RTA_OIF:  // znajduje sie tam numer interfejsu, przez ktory jest
                       // dostepna ta brama (nie nazwy interfejsu, tylko numer)
          sprintf(dev, "%d", *((int *)RTA_DATA(atp)));
          break;
      }
    }
    sprintf(msk, "%d", rtp->rtm_dst_len);
    if (strlen(dst) == 0)  // jezeli dst == to mamy brame domyslna
      printf("default via %s dev %s\n", gwy, dev);
    else if (strlen(gwy) == 0)  // to siec bezposrednio dostepna
      printf("%s/%s dev %s\n", dst, msk, dev);
    else  // tutaj mamy jakas trase
      printf("dst %s/%s gwy %s dev %s\n", dst, msk, gwy, dev);
  }

  close(sfd);
  return EXIT_SUCCESS;
}
