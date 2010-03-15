// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// 
// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
// #include <signal.h>
// #include <sys/wait.h>
#include "general.h"

#define SERV_PORT   5193
#define SERVICE_PORT	6000
#define BACKLOG       10
#define MAXLINE     1024
//Stavo vedendo come fare in modo che ci siano due socket, uno che ascolta le richieste normali e uno quelle di servizio
//vedere articolo sui socket e sulle FDSET per settare le opzioni dei socket
//non si fa con la accept!

void mainDelFiglio();
void interrompi();

int pid, i;
int listensd, connsd, listensdDiServizio, connsdDiServizio;
struct sockaddr_in servaddr, servaddrDiServizio;
struct sockaddr_in ricevutoSuAddr;
	
main() {
	
	//Gestisce l'interruzione con ctrl-c
	(void) signal(SIGINT, interrompi);

	printf("%d: Avvio del server...\n", getpid());
	
	//listen per i processi normali
	if ((listensd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* crea il socket */
		printf("%d: ", getpid());
		perror("errore in socket\n");
		exit(1);
	}

	//listen per i messaggi di servizio
	if ((listensdDiServizio = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* crea il socket */
		printf("%d: ", getpid());
		perror("errore in socket\n");
		exit(1);
	}

	inizializza_memset(&servaddr, SERV_PORT);
	inizializza_memset(&servaddrDiServizio, SERVICE_PORT);
	
	/* assegna l'indirizzo al socket normale */
	if ((bind(listensd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) { 
		printf("%d: ", getpid());
		perror("errore in bind");
		exit(-1);
	}

	/* assegna l'indirizzo al socket di servizio */
	if ((bind(listensdDiServizio, (struct sockaddr *) &servaddrDiServizio, sizeof(servaddrDiServizio))) < 0) { 
		printf("%d: ", getpid());
		perror("errore in bind");
		exit(-1);
	}

	//listen normale
	if (listen(listensd, BACKLOG) < 0 ) {
		printf("%d: ", getpid());
		perror("errore in listen");
		exit(-1);
	}

	//listen di servizio
	if (listen(listensdDiServizio, BACKLOG) < 0 ) {
		printf("%d: ", getpid());
		perror("errore in listen");
		exit(-1);
	}
	
	for ( ; ; ) {
	
		//connessione accettata normale
		if ((connsd = accept(listensd, (struct sockaddr *)NULL, NULL)) < 0) {
			printf("%d: ", getpid());
			perror("errore in accept");
			exit(-1);
		}
		
		//se è stata accettata una connessione normale...
		if(connsd != 0) {
			printf("%d: Creazione di un figlio in corso...\n", getpid());
			
			pid = fork();
		
			mainDelFiglio();
		
			int ritornoPid;
			
			//aspetto che un figlio termini
			waitpid(-1, &ritornoPid, WNOHANG);
			
			//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
			
			if (close(connsd) == -1) {  /* chiude la connessione */
				printf("%d: ", getpid());
				perror("errore in close");
				exit(-1);
			}
		}
	}
			
exit(0);
}

void mainDelFiglio() {
	
		if(pid == 0) 
		{
			time_t        ticks;
			char          buff[MAXLINE];
			
			printf("%d: Presa in consegna richiesta\n", getpid());
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			
			getpeername(connsd, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			
			printf("%d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));
				
			/* accetta una connessione con un client */
			ticks = time(NULL); /* legge l'orario usando la chiamata di sistema time */
			/* scrive in buff l'orario nel formato ottenuto da ctime; 
				snprintf impedisce l'overflow del buffer. */
			snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); /* ctime trasforma la data 
					e lora da binario in ASCII. \r\n per carriage return e line feed*/

			/* scrive sul socket di connessione il contenuto di buff */
			if (write(connsd, buff, strlen(buff)) != strlen(buff)) {
				printf("%d: ", getpid());
				perror("errore in write del figlio\n"); 
				exit(-1);
			}
			
			printf("%d: Richiesta elaborata!\n", getpid());
			
			exit(0);
		}

}

void interrompi() {
	printf("%d: Il server è stato terminato da console\n", getpid());

	exit(0);

}
