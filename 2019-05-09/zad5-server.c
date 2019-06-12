/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./sctpmh-server.c -o ./sctpmh-server -lsctp
 * Usage:        ./sctpmh-server
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define LOCAL_TIME_STREAM 0
#define GREENWICH_MEAN_TIME_STREAM 1

int main(int argc, char **argv) {
  int sfd, cfd, on = 1, no, i;
  socklen_t sl;
  char buf[1024];
  time_t now;
  struct sctp_event_subscribe events;
  struct sctp_paddrparams heartbeat;
  struct sctp_rtoinfo rtoinfo;
  struct sockaddr_in *laddrs[5];
  struct sockaddr_in addr, raddr;

  sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(1234);
  memset(&heartbeat, 0, sizeof(heartbeat));
  heartbeat.spp_flags = SPP_HB_ENABLE;
  heartbeat.spp_hbinterval = 2000;
  heartbeat.spp_pathmaxrxt = 1;
  setsockopt(sfd, SOL_SCTP, SCTP_PEER_ADDR_PARAMS , &heartbeat,
             sizeof(heartbeat));
  memset(&rtoinfo, 0, sizeof(rtoinfo));
  rtoinfo.srto_max = 2000;
  setsockopt(sfd, SOL_SCTP, SCTP_RTOINFO , &rtoinfo, sizeof(rtoinfo));
  memset((void*) &events, 0, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt(sfd, SOL_SCTP, SCTP_EVENTS, (void*) &events, sizeof(events));
  bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
  listen(sfd, 5);
  no = sctp_getladdrs(sfd, 0, (struct sockaddr**) laddrs);
  for(i = 0; i < no; i++)
    printf("ADDR %d: %s:%d\n", i + 1, inet_ntoa((*laddrs)[i].sin_addr),
           ntohs((*laddrs)[i].sin_port));
  sctp_freeladdrs((struct sockaddr*) *laddrs);
  memset(&raddr, 0, sizeof(raddr));
  memset(&buf, 0, sizeof(buf));
  sl = sizeof(raddr);
  cfd = accept(sfd, (struct sockaddr*) &raddr, &sl);
  printf("new connection: %s:%d\n", inet_ntoa(raddr.sin_addr),
         raddr.sin_port);
  while (1) {
    now = time(NULL);
    snprintf(buf, 1024, "%s", ctime(&now));
    sctp_sendmsg(cfd, (void*) buf, (size_t) strlen(buf) + 1, NULL, 0, 0, 0,
                 LOCAL_TIME_STREAM, 0, 0);
    snprintf(buf, 1024, "%s", asctime(gmtime(&now)));
    sctp_sendmsg(cfd, (void*) buf, (size_t) strlen(buf) + 1, NULL, 0, 0, 0,
                 GREENWICH_MEAN_TIME_STREAM, 0, 0);
    sleep(1);
  }
  close(cfd);
  close(sfd);
}
