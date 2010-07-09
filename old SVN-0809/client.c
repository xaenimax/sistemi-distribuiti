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
  char			risposta_selettore[MAXLINE];
  int byte_letti = 0;

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

  if ((byte_letti = recv(sockfd, risposta_selettore, sizeof(risposta_selettore) - 1, 0)) != 0) {
  		printf("\nTutto ok: ho ricevuto risposta (%d byte) \n", byte_letti);
  		printf("Server scelto: %s\n",risposta_selettore);
  		//strcpy(risposta_selettore,"");
  		}
  else fprintf(stderr, "Problema con la recv()");


  printf("Chiudo la connessione col selettore... \n\n");

  close(sockfd);

  exit(0);
}

