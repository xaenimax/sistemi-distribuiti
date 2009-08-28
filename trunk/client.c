/*
*  C Implementation: da decidere
*
* Description:
*
* Author:
* Vienna Codeluppi <viecode@gmail.com>,
* Alessandro Pacca <alessandro.pacca@gmail.com>,
* Marina Dorelli  <aenima.rm@gmail.com> (C) 2009
*
* Copyright: See COPYING file that comes with this distribution
*
*/


#include "basic.h"


void str_cli_echo_sel(FILE *fd, int sockfd);

int main(int argc, char **argv)
{
  int			sockfd;
  struct sockaddr_in	servaddr;

  if (argc != 2) {
    fprintf(stderr, "utilizzo: echo_client_sel <indirizzo IP server>\n");
    exit(1);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("errore in socket");
    exit(1);
  }

  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERV_PORT);
  if (inet_aton(argv[1], &servaddr.sin_addr) <= 0) {
    fprintf(stderr, "errore in inet_aton per %s\n", argv[1]);
    exit(1);
  }

  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    perror("errore in connect");
    exit(1);
  }

  str_cli_echo_sel(stdin, sockfd);	/* svolge il lavoro del client */

  close(sockfd);

  exit(0);
}

/******/
void str_cli_echo_sel(FILE *fd, int sockfd)
{
  int		maxd;
  fd_set	rset;


  FD_ZERO(&rset);	/* inizializza a 0 il set dei descrittori in lettura */
  for ( ; ; ) {
    FD_SET(fileno(fd), &rset);	/* inserisce il descrittore del file */
    FD_SET(sockfd, &rset); /* inserisce il descrittore del socket */
    maxd = (fileno(fd) < sockfd) ? (sockfd+1): (fileno(fd)+1);
    if (select(maxd, &rset, NULL, NULL, NULL) < 0 ) {
      perror("errore in select");
      exit(1);
    }




  }
}
