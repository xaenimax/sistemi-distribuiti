/* Server iterativo dell'applicazione echo che utilizza la select
   Ultima revisione: 8 ottobre 2008 */

#include "basic.h"


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



      //accedo alla struttura cliaddr e ne prlevo indirizzo ip e porta
      /*
      printf("Indirizzo del Client: indirizzo IP  %s, porta %d\n",
              inet_ntop(AF_INET,
              &cliaddr.sin_addr, buff, sizeof(buff)),
              ntohs(cliaddr.sin_port) ); */

      char comando[70] = "links -dump \"http://api.hostip.info/get_html.php?ip=";
      //char indirizzo[16];  //conterrà l'ip del client

      //char indirizzo_ip[] = "51.160.80.2";
      //char indirizzo_ip[] =
    	  //inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff));
     // printf ("%s \n", indirizzo_ip);

      const char *indirizzo;

      indirizzo = inet_ntop(AF_INET,
              &cliaddr.sin_addr, buff, sizeof(buff));

      printf("Ricevuta richiesta da %s \n", indirizzo);


      strcat(comando, indirizzo);
      strcat(comando, "\"");


      //printf ("prova segmentation fault: comando concatenato %s \n", comando);   OK

      /* prova comando shell lanciato da c
      system("links -dump \"http://api.hostip.info/get_html.php?ip=51.160.80.2\""); */ //  OK




      //system(comando);  **********

      char c = ' ';

          FILE *stream_comando = popen(comando, "r");

          FILE *stream_testo = fopen("testo","w+t");

          char risposta[200];


          while ((fscanf(stream_comando,"%c",&c)) == 1) // ripete il loop finche' fscan(); legge un carattere

              {
        	  char temp[ 2 ];
        	  temp[ 0 ] = c;
        	  temp[ 1 ] = '\0';

                  printf("%c",c);   // scrive il carattere
                  strcat(risposta, temp);

                                  }
          printf("\n");

          printf("%s", risposta);
              pclose(stream_comando);
              fclose(stream_testo);




          //char mystring[5000];
          //fgets(mystring,5000,stdout);
          	//printf("\n\n\n\n\n%s",mystring);

          //getdelim(&s, NULL, '\0', p);
          //pclose(stream_comando);
          //printf("%s \n", s);
          //return 0;







      /*
      while ( fgets( line, sizeof line, fp))
        {
          printf("%s", line);
        }
        pclose(fp);
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
