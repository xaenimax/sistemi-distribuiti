#include "basic.h"



/*******ottiene il tempo di ping*******/


//fare


/********* ottiene la nazione del client ********/


//fare

/********* legge l'output di un programma dallo stdout della shell
 *            e lo salva in una variabile locale (stringa)         ********/




             char * readStdout(char * comando) {

            	  char c = ' ';

           //printf("mi è arrivato il comando %s \n \n", comando);
            	            FILE *stream_comando = popen(comando, "r");



            	            char output_bash[3000];

						    //printf("attualmente output_b contiene questo \n %s", output_bash);
						    //printf("*******");

            	            while ((fscanf(stream_comando,"%c",&c)) == 1) // ripete il loop finche' fscan(); legge un carattere

            	                {
            	          	  //rendo il carettere "appendibile" alla stringa
            	          	  char temp[ 2 ];
            	          	  temp[ 0 ] = c;
            	          	  temp[ 1 ] = '\0';

            	          	//salvo lo stdout in una stringa (concatenando carattere per carattere)
            	                    strcat(output_bash, temp);

            	                                    }


            	            //printf("%s", nazione_client);
            	                pclose(stream_comando);


              //printf("sto inviando al selettore l'output \n %s \n", output_bash);
              char output2[3000];
              strcpy(output2, output_bash);
              //printf("sto ridando %s \n", output2);
              strcpy(output_bash, " ");

              return (output2);
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
