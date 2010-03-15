/* daytime_serverTCP.c - code for daytime server program that uses TCP 
   Tratto da W.R. Stevens, "Network Programming Vol. 1" 
   Ultima revisione: 14 gennaio 2008 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SERV_PORT   5193
#define BACKLOG       10
#define MAXLINE     1024

int main(int argc, char **argv)
{
  int           listensd, connsd, n;
  struct sockaddr_in servaddr;
  struct sockaddr_in ricevutoSuAddr;
  char          buff[MAXLINE];
	char rec[MAXLINE];
  time_t        ticks;

  if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* crea il socket */
    perror("errore in socket");
    exit(1);
  }
  
  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* il server accetta 
        connessioni su una qualunque delle sue intefacce di rete */
  servaddr.sin_port = htons(SERV_PORT); /* numero di porta del server */

  /* assegna l'indirizzo al socket */
  if ((bind(listensd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) { 
    perror("errore in bind");
    exit(1);
  }

  if (listen(listensd, BACKLOG) < 0 ) {
    perror("errore in listen");
    exit(1);
  }

  for ( ; ; ) {
    if ((connsd = accept(listensd, (struct sockaddr *)NULL, NULL)) < 0) {
      perror("errore in accept");
      exit(1);
    }
      
    memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
    
    int lunghezzaAddr = sizeof(ricevutoSuAddr);
    
    getsockname(connsd, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
    
    printf("Ricevuta richiesta sull'indirizzo IP: %s:%d\n", (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));
      
    /* accetta una connessione con un client */
    ticks = time(NULL); /* legge l'orario usando la chiamata di sistema time */
    /* scrive in buff l'orario nel formato ottenuto da ctime; 
       snprintf impedisce l'overflow del buffer. */
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); /* ctime trasforma la data 
        e lora da binario in ASCII. \r\n per carriage return e line feed*/

		
		printf("Leggo i dati.\n");
		
		while((n = recv(connsd, rec, MAXLINE, 0)) > 0) {
		rec[n] = 0;
		if(fputs(rec, stdout) == EOF) {
			fprintf(stderr, "Errore in fputs");
			exit (-1);
		}

		printf("\nInvio i dati.\n");

    /* scrive sul socket di connessione il contenuto di buff */
    if (send(connsd, buff, strlen(buff), 0) != strlen(buff)) {
      perror("errore in write"); 
      exit(1);
    }

		printf("\nDati inviati!\n");
		
		close(connsd);
		exit(1);
	}

    if (close(connsd) == -1) {  /* chiude la connessione */
      perror("errore in close");
      exit(1);
    }      
  }
  exit(0);
}
