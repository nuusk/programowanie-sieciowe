/*
 * Copyright (C) 2019 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./tcp-server.c -o ./tcp-server
 * Usage:        ./tcp-server
 *
 * Bug reports:  https://gitlab.cs.put.poznan.pl/mkalewski/ps-2019/issues
 *
 */

/*
echo '256 256 256' > /proc/sys/net/ipv4/tcp_rmem
*/

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define USTALONA_STALA_WARTOSC_NA_LABACH 128

int _write(int sfd, char *buf, int len) {
  while (len > 0) {
    int i = write(sfd, buf, len);
    len -= i;
    buf += i;
  }

  return 0;
}

int main(int argc, char** argv) {
  socklen_t sl;
  int sfd, cfd, on = 1;
  struct sockaddr_in saddr, caddr;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(1234);
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));
  bind(sfd, (struct sockaddr*) &saddr, sizeof(saddr));
  listen(sfd, 5);
  while(1) {
    memset(&caddr, 0, sizeof(caddr));
    sl = sizeof(caddr);
    cfd = accept(sfd, (struct sockaddr*) &caddr, &sl);
    
    // zadanie 3
    // write(cfd, "Hello World!\n", 14);
    // write(cfd, "Hello World 2!\n", 16); // teraz pytanie jak wymusic zeby klient odebral jednym readem wiele writów?
    
    // zadanie 3
    _write(cfd, "Hello World!\n", USTALONA_STALA_WARTOSC_NA_LABACH);
    _write(cfd, "Hello World 2!\n", USTALONA_STALA_WARTOSC_NA_LABACH);

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

    
    close(cfd);
  }
  close(sfd);
  return EXIT_SUCCESS;
}
