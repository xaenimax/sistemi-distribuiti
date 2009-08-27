#include "basic.h"

/********* ottiene la nazione del client ********/
	/* prende in argomento l'output della shell catturato con leggiStdout(..) *
	 * e ne estrae il parametro di interesse                                  */


int estraiPaese(char * risposta_hostinfo) {  //TODO chiamarla dal posto giusto
    char * sottostringa;         //TODO e cambiare il tipo restituito in *char
    char * sottostringa2;

    if( (sottostringa = strstr(risposta_hostinfo, "Private Address")) != NULL) return 0; //printf("IP non pubblico");
    else if( ((sottostringa = strstr(risposta_hostinfo, "(")) == NULL) || ((sottostringa2 = strstr(risposta_hostinfo, ")")) == NULL)  ) return 0; //non ho trovato il paese

    else printf("paese: %c%c", sottostringa[1], sottostringa[2]); //printf("%c%c", sottostringa[1], sottostringa[2]);
    return 0;
 }


/******* estrae il tempo di ping *******/


int estraiTempo(char * risposta_ping) { //TODO cambia tipo restituito e funzioni chiamanti + converti char->int
	char * sottostringa;
	char * sottostringa2;
	int lunghezza = 0, i = 0;

	if( (sottostringa = strstr(risposta_ping, "=")) == NULL) return 0;
	else if( ((sottostringa = strstr(sottostringa, "/")) == NULL) || ((sottostringa2 = strstr(++sottostringa, "/")) == NULL)  ) return 0; //il ping non ha dato risposta
	else {
		//printf("visualizza tempi (tutti) %s", sottostringa);
		lunghezza = sottostringa2 - sottostringa;
		//printf("lungo %d \n", lunghezza);
		char tempo[lunghezza];
		for (i = 0; i < lunghezza; i++)
			{ tempo[i] = sottostringa[i]; } //sto estraendo i millisecondi...

		printf("tempo: %s millisecondi", tempo);
	     }

	return 0;



	return 0;
}



//fare





/********* legge l'output di un programma dallo stdout della shell
 *            e lo salva in una variabile locale (stringa)         ********/

             char * leggiStdout(char * comando) {

            	  char c = ' ';

            	  FILE *stream_comando = popen(comando, "r");

            	  char output_temp[2000]; //dove verra' salvato l'output

            	  while ((fscanf(stream_comando,"%c",&c)) == 1) // ripete il loop finche' fscan(); legge un carattere

            	                {
            	          	  char temp[ 2 ];  //rendo il carettere "appendibile" alla stringa
            	          	  temp[ 0 ] = c;
            	          	  temp[ 1 ] = '\0';
            	              strcat(output_temp, temp); //salvo lo stdout in una stringa (concatenando carattere per carattere)
            	                 }  //fine while

            	  pclose(stream_comando);

              char output_bash[2001]; //salvo il risultato in una nuova stringa e svuoto quella temporanea (perchè mi servirà successive volte)
              strcpy(output_bash, output_temp);
              strcpy(output_temp, " ");

              return (output_bash);
              }
/**********fine lettura stdout ******************/



/******/

ssize_t writen(int fd, const void *buf, size_t n)
{
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = buf;
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
       if ((nwritten < 0) && (errno == EINTR)) nwritten = 0;
       else return(-1);	    /* errore */
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return(n-nleft);
}

/******/
int readline(int fd, void *vptr, int maxlen)
{
  int  n, rc;
  char c, *ptr;

  ptr = vptr;
  for (n = 1; n < maxlen; n++) {
    if ((rc = read(fd, &c, 1)) == 1) {
      *ptr++ = c;
      if (c == '\n') break;
   }
   else
      if (rc == 0) {		/* read ha letto l'EOF */
 	 if (n == 1) return(0);	/* esce senza aver letto nulla */
 	 else break;
      }
      else return(-1);		/* errore */
  }

  *ptr = 0;	/* per indicare la fine dell'input */
  return(n);	/* restituisce il numero di byte letti */
}
