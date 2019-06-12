/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./tcp-client.c -o ./tcp-client
 * Usage:        ./tcp-client SERVER PORT
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

#define USTALONA_STALA_WARTOSC_NA_LABACH 128

int _read(int sfd, char *buf, int bufsize) {
  do {
    int i = read(sfd, buf, bufsize);
    bufsize -= i;
    buf += i;
  } while(bufsize > 0);

  return USTALONA_STALA_WARTOSC_NA_LABACH;
}

int main(int argc, char** argv) {
  int sfd, rc;
  char buf[USTALONA_STALA_WARTOSC_NA_LABACH];
  struct sockaddr_in saddr;
  struct hostent* addrent;

  addrent = gethostbyname(argv[1]);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(atoi(argv[2]));
  memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
  connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr));

  // zad 3
  // sleep(1); //dodajemy sleepa zeby klient odebral wiele writów jednym readem
  // rc = read(sfd, buf, USTALONA_STALA_WARTOSC_NA_LABACH);
  // write(1, buf, rc);
  
  // zad 4
  // // pierwsza wiadomosc
  // rc = read(sfd, buf, USTALONA_STALA_WARTOSC_NA_LABACH);
  // printf("%s\n", buf);
  // // druga wiadomosc
  // rc = read(sfd, buf, USTALONA_STALA_WARTOSC_NA_LABACH);
  // printf("%s\n", buf);

  // zad 5
  write(sfd, buf, 1000000);
  shutdown(sfd, SHUT_WR);
  while(1) {
    rc = read(sfd, tmpbuf,sizeof(tmpbuf));
    if(rc < 0) {
      perror("reading");
      exit(1);
    }
    if(rc == 0)
      break;
  }

  close(sfd);
  return EXIT_SUCCESS;
}
