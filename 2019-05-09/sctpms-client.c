/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./sctpms-client.c -o ./sctpms-client -lsctp
 * Usage:        ./sctpms-client SRVIP
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define LOCAL_TIME_STREAM 0
#define GREENWICH_MEAN_TIME_STREAM 1

int main(int argc, char **argv) {
  int cfd, flags, i, rc;
  struct sockaddr_in saddr;
  struct sctp_sndrcvinfo sndrcvinfo;
  struct sctp_event_subscribe events;
  char buf[1024];

  cfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(1234);
  saddr.sin_addr.s_addr = inet_addr(argv[1]);
  connect(cfd, (struct sockaddr*) &saddr, sizeof(saddr));
  memset((void*) &events, 0, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt(cfd, SOL_SCTP, SCTP_EVENTS, (const void*) &events,
             sizeof(events));
  for (i = 0; i < 2; i++) {
    flags = 0;
    rc = sctp_recvmsg(cfd, (void*) buf, sizeof(buf), (struct sockaddr*) NULL,
                      0, &sndrcvinfo, &flags);
    buf[rc] = 0;
    if (sndrcvinfo.sinfo_stream == LOCAL_TIME_STREAM) {
      printf("(Local) %s", buf);
    } else if (sndrcvinfo.sinfo_stream == GREENWICH_MEAN_TIME_STREAM) {
      printf("(GMT)   %s", buf);
    }
  }
  close(cfd);
  return EXIT_SUCCESS;
}
