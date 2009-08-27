/* Cercasi volontario che sistema gli autori qui :-D */

#include "basic.h"
#include "funzioni_io.h"


int main(int argc, char **argv)
{
  int			listensd, connsd, socksd;
  int			i, maxi, maxd;
  int			ready, client[FD_SETSIZE];
  char			buff[MAXLINE];
  fd_set		rset, allset;

  struct sockaddr_in	servaddr, cliaddr;
  socklen_t		len;

  //apre socket in ascolto
  if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("errore in socket");
    exit(1);
  }

  memset((void *)&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port	   = htons(SERV_PORT);

  if ((bind(listensd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
    perror("errore in bind");
    exit(1);
  }

  if (listen(listensd, BACKLOG) < 0 ) {
    perror("errore in listen");
    exit(1);
  }

  /* Inizializza il numero di descrittori */
  maxd = listensd;
  maxi = -1;
  /* Inizializza l'array di interi client contenente i descrittori utilizzati */
  for (i = 0; i < FD_SETSIZE; i++)
    client[i] = -1;

  FD_ZERO(&allset); /* Inizializza a zero l'insieme dei descrittori */
  FD_SET(listensd, &allset); /* Inserisce il descrittore di ascolto */

  for ( ; ; ) {
    rset = allset;  /* Setta il set di descrittori per la lettura */
    /* ready � il numero di descrittori pronti */
    if ((ready = select(maxd+1, &rset, NULL, NULL, NULL)) < 0) { //select(maxconnessioni, 3liste-socket, timeout)
      perror("errore in select");
      exit(1);
    }

    /* Se � arrivata una richiesta di connessione, il socket di ascolto
       � leggibile: viene invocata accept() e creato un socket di connessione */
    if (FD_ISSET(listensd, &rset)) {
      len = sizeof(cliaddr);
      if ((connsd = accept(listensd, (struct sockaddr *)&cliaddr, &len)) < 0) {
        perror("errore in accept");
        exit(1);
      }




      char comando_links[70] = "links -dump \"http://api.hostip.info/get_html.php?ip=";


      const char *indirizzo;

      indirizzo = inet_ntop(AF_INET,
              &cliaddr.sin_addr, buff, sizeof(buff));

      printf("Ricevuta richiesta da %s \n", indirizzo);


      strcat(comando_links, indirizzo);
      strcat(comando_links, "\"");


              char * nazione_client = (char *)readStdout(comando_links);
              printf(" stampa GEO da selettore %s \n", nazione_client);

// -------------------TEST PING ----------------------//   OK

              char *comando_ping = "ping -c 4 74.125.43.105";
              char * tempo_ping = (char *)readStdout(comando_ping);
                            printf("stampa PING da selettore %s \n", tempo_ping);

/*


	char c2 = ' ';

    FILE *stream_comandoping = popen(comandoping, "r");

    char tempi_risposta_ping[1500];

    while ((fscanf(stream_comandoping,"%c",&c2)) == 1) // ripete il loop finche' fscan(); legge un carattere

                  {
            	  //rendo il carettere "appendibile" alla stringa
            	  char temp2[ 2 ];
            	  temp2[ 0 ] = c2;
            	  temp2[ 1 ] = '\0';

                      //printf("%c",c);   //controllo se riesco a salvare lo stdout in una variabile OK
                      strcat(tempi_risposta_ping, temp2); //salvo lo stdout in una stringa

                                      }
              printf("\n");

              printf("%s", tempi_risposta_ping);
                  pclose(stream_comandoping);
*/

      /* Inserisce il descrittore del nuovo socket nel primo posto
         libero di client */
      for (i=0; i<FD_SETSIZE; i++)
        if (client[i] < 0) {
          client[i] = connsd;
          break;
        }
      /* Se non ci sono posti liberi in client, errore */
      if (i == FD_SETSIZE) {
        fprintf(stderr, "errore in accept\n");
        exit(1);
      }
      /* Altrimenti inserisce connsd tra i descrittori da controllare
         ed aggiorna maxd */
      FD_SET(connsd, &allset);
      if (connsd > maxd) maxd = connsd;
      if (i > maxi) maxi = i;
      if (--ready <= 0) /* Cicla finch� ci sono ancora descrittori
                           leggibili da controllare */
        continue;
    }

    /* Controlla i socket attivi per controllare se sono leggibili */
    for (i = 0; i <= maxi; i++) {
      if ((socksd = client[i]) < 0 )
        /* se il descrittore non � stato selezionato viene saltato */
        continue;

    }
  }
}
