/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./sctpmh-client.c -o ./sctpmh-client -lsctp
 * Usage:        ./sctpmh-client SRVIP
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
#include <unistd.h>

#define LOCAL_TIME_STREAM 0
#define GREENWICH_MEAN_TIME_STREAM 1


int main(int argc, char **argv) {
  int sfd, no, i, flags;
  socklen_t sl;
  char buf[1024];
  struct sctp_event_subscribe events;
  struct sctp_initmsg initmsg;
  struct sctp_paddrparams heartbeat;
  struct sctp_rtoinfo rtoinfo;
  struct sockaddr_in *paddrs[5];
  struct sockaddr_in saddr, raddr;
 struct sctp_sndrcvinfo sndrcvinfo;

  sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = PF_INET;
  saddr.sin_port = htons(1234);
  saddr.sin_addr.s_addr = inet_addr(argv[1]);
  memset(&initmsg, 0, sizeof(initmsg));
  initmsg.sinit_num_ostreams = 2;
  initmsg.sinit_max_instreams = 2;
  initmsg.sinit_max_attempts = 1;
  setsockopt(sfd, SOL_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));
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
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  no = sctp_getpaddrs(sfd, 0, (struct sockaddr**) paddrs);
  for (i = 0; i < no; i++)
    printf("ADDR %d: %s:%d\n", i + 1, inet_ntoa((*paddrs)[i].sin_addr),
           ntohs((*paddrs)[i].sin_port));
  sctp_freepaddrs((struct sockaddr*)*paddrs);
  while (1) {
	  flags = 0;
	  int rc = sctp_recvmsg(sfd, (void*) buf, sizeof(buf), (struct sockaddr*) NULL,0, &sndrcvinfo, &flags);
	buf[rc] = 0;
	printf("%d", rc);
	  if (sndrcvinfo.sinfo_stream == LOCAL_TIME_STREAM) {
      printf("(Local) %s", buf);
    } else if (sndrcvinfo.sinfo_stream == GREENWICH_MEAN_TIME_STREAM) {
      printf("(GMT)   %s", buf);
    }

    memset(&raddr, 0, sizeof(raddr));
    memset(&buf, 0, sizeof(buf));
    sl = sizeof(raddr);
  }
  close(sfd);
}
