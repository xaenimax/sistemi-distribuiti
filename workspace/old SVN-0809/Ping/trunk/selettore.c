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
#include "funzioni_io.h"


int main ( int argc, char **argv ) {
	int			listensd, connsd, socksd;
	int			i, maxi, maxd;
	int			ready, client[FD_SETSIZE];
	char			buff[MAXLINE];
	fd_set		rset, allset;

	struct sockaddr_in	servaddr, cliaddr;
	socklen_t		len;

	//apre socket in ascolto

	if ( ( listensd = socket ( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
		perror ( "errore in socket" );
		exit ( 1 );
	}

	memset ( ( void * ) &servaddr, 0, sizeof ( servaddr ) );

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl ( INADDR_ANY );
	servaddr.sin_port	   = htons ( SERV_PORT );

	if ( ( bind ( listensd, ( struct sockaddr * ) &servaddr, sizeof ( servaddr ) ) ) < 0 ) {
		perror ( "errore in bind" );
		exit ( 1 );
	}

	if ( listen ( listensd, BACKLOG ) < 0 ) {
		perror ( "errore in listen" );
		exit ( 1 );
	}

	/* Inizializza il numero di descrittori */
	maxd = listensd;

	maxi = -1;

	/* Inizializza l'array di interi client contenente i descrittori utilizzati */
	for ( i = 0; i < FD_SETSIZE; i++ )
		client[i] = -1;

	FD_ZERO ( &allset ); /* Inizializza a zero l'insieme dei descrittori */

	FD_SET ( listensd, &allset ); /* Inserisce il descrittore di ascolto */

	for ( ; ; ) {
		rset = allset;  /* Setta il set di descrittori per la lettura */
		/* ready � il numero di descrittori pronti */

		if ( ( ready = select ( maxd+1, &rset, NULL, NULL, NULL ) ) < 0 ) { //select(maxconnessioni, 3liste-socket, timeout)
			perror ( "errore in select" );
			exit ( 1 );
		}

		/* Se � arrivata una richiesta di connessione, il socket di ascolto
		   � leggibile: viene invocata accept() e creato un socket di connessione */
		if ( FD_ISSET ( listensd, &rset ) ) {
			len = sizeof ( cliaddr );

			if ( ( connsd = accept ( listensd, ( struct sockaddr * ) &cliaddr, &len ) ) < 0 ) {
				perror ( "errore in accept" );
				exit ( 1 );
			}




			char comando_links[70] = "links -dump \"http://api.hostip.info/get_html.php?ip=";

			char *datiGEO_client;
			char *sigla_nazione;

			const char *indirizzo;

			indirizzo = inet_ntop ( AF_INET,
			                        &cliaddr.sin_addr, buff, sizeof ( buff ) );

			printf ( "Ricevuta richiesta da %s \n", indirizzo );


			strcat ( comando_links, indirizzo );
			strcat ( comando_links, "\"" );


			//strcpy(comando_links, "links -dump \"http://api.hostip.info/get_html.php?ip=87.17.103.35\"");

			datiGEO_client = ( char * ) leggiStdout ( comando_links );


			printf ( " **stampa GEO da selettore** \n %s \n", datiGEO_client );


			printf ( "********* test estrazione PAESE *****************\n" );

			sigla_nazione = ( char * ) estraiPaese ( datiGEO_client );

			if ( sigla_nazione == NULL ) printf ( "indirizzo IP privato/sconosciuto" );
			else printf ( " **stampa PAESE %c%c ** \n", sigla_nazione[0], sigla_nazione[1] ); //non so perchè ma se provo a stamparlo come stringa non stampa niente

			printf ( "\n****************\n" );

// -------------------TEST PING ----------------------//   OK

			char *comando_ping = "ping -c 4 -q 74.125.43.105";

			char * tempo_ping = ( char * ) leggiStdout ( comando_ping );

			printf ( "**stampa PING da selettore** \n %s \n", tempo_ping );

			printf ( "********* test estrazione PING *****************\n" );

			estraiTempo ( tempo_ping );

			printf ( "\n****************\n" );

			/* Inserisce il descrittore del nuovo socket nel primo posto
			   libero di client */
			for ( i=0; i<FD_SETSIZE; i++ )
				if ( client[i] < 0 ) {
					client[i] = connsd;
					break;
				}

			/* Se non ci sono posti liberi in client, errore */
			if ( i == FD_SETSIZE ) {
				fprintf ( stderr, "errore in accept\n" );
				exit ( 1 );
			}

			/* Altrimenti inserisce connsd tra i descrittori da controllare
			   ed aggiorna maxd */
			FD_SET ( connsd, &allset );

			if ( connsd > maxd ) maxd = connsd;

			if ( i > maxi ) maxi = i;

			if ( --ready <= 0 ) /* Cicla finch� ci sono ancora descrittori
                           leggibili da controllare */
				continue;
		}

		/* Controlla i socket attivi per controllare se sono leggibili */
		for ( i = 0; i <= maxi; i++ ) {
			if ( ( socksd = client[i] ) < 0 )
				/* se il descrittore non � stato selezionato viene saltato */
				continue;

		}
	}
}
