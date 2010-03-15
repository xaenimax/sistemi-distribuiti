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

//==================cerca continente vicino nel db===========================//

char trovaContinente(char * nazione) {
    char continente;
	FILE * elenco_nazioni;
	              elenco_nazioni = fopen("db_nazioni", "r");
	              if (elenco_nazioni == NULL)
	              {
	            	  fprintf(stderr, "Problemi con l'apertura del file\n");
	              }
	              else
	              {
	            	  char  tmp[256]={0x0};
	            	  while(elenco_nazioni != NULL && fgets(tmp, sizeof(tmp), elenco_nazioni)!=NULL)
	            	          {
	            	          if (strstr(tmp, nazione)) {
	            	          //printf("Il continente piu vicino a %s ha il codice %c \n\n", nazione, tmp[3]);
	            	          continente = tmp[3];
	            	          }

	            	          }
	            	          if(elenco_nazioni!=NULL) fclose(elenco_nazioni);


	            	  //fclose(elenco_nazioni);
	              }


	return continente;
}




//======================== ottiene la nazione del client ===========================/
	/* prende in argomento l'output della shell (precedentemente catturato
	 * con leggiStdout(..)) e ne estrae il parametro di interesse
	 */


char * estraiPaese(char * risposta_hostinfo) {
    char * sottostringa;
    char * sottostringa2;
    char paese[3];

    if( (sottostringa = strstr(risposta_hostinfo, "Private Address")) != NULL)
    	{
    	//indirizzo IP privato/sconosciuto
    	return NULL;
    	}
    else if( ((sottostringa = strstr(risposta_hostinfo, "(")) == NULL) || ((sottostringa2 = strstr(risposta_hostinfo, ")")) == NULL)  )
    	{
    	//indirizzo IP privato/sconosciuto
    	return NULL;
    	}
    else {
    	paese[0] = sottostringa[1];
    	paese[1] = sottostringa[2];

    	return (paese);
        }
    printf("ora paese vale %s \n", paese);
    return NULL;
 }







//================== estrae il tempo di ping ==============================/


char * estraiTempo(char * risposta_ping) {
	char * sottostringa;
	char * sottostringa2;
	int lunghezza, i = 0;

	if( (sottostringa = strstr(risposta_ping, "=")) == NULL) return NULL;
	else if( ((sottostringa = strstr(sottostringa, "/")) == NULL) || ((sottostringa2 = strstr(++sottostringa, "/")) == NULL)  ) return NULL; //il ping non ha dato risposta
	else {
		//printf("visualizza tempi (tutti) %s", sottostringa);
		lunghezza = sottostringa2 - sottostringa;
		//printf("lungo %d \n", lunghezza);
		char tempo[++lunghezza];
		for (i = 0; i < (lunghezza-1); i++)
			{ tempo[i] = sottostringa[i]; } //sto estraendo i millisecondi...
        tempo[lunghezza-1] = '\0';
		//printf("tempo: %s millisecondi", tempo);
        return (tempo);
	     }


	return NULL;
}



//fare





/********* legge l'output di un programma dallo stdout della shell
 *            e lo salva in una variabile locale (stringa)         ********/

             char * leggiStdout(char * comando) {

            	 char output_bash[2001]; //salvo il risultato in una nuova stringa e svuoto quella temporanea (perchè mi servirà successive volte)
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


              strcpy(output_bash, output_temp);
              strcpy(output_temp, " ");

              return (output_bash);
              }
/**********fine lettura stdout ******************/

             char output_bash[2001]; //salvo il risultato in una nuova stringa e svuoto quella temporanea (perchè mi servirà successive volte)

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
