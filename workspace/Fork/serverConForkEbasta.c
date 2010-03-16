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
void mainDelFiglioDiServizio();
void acceptFiglioNormale();
void acceptFiglioDiServizio();
void interrompi();

int pid, pidServizio, i;
int listensd, connsd, listensdDiServizio, connsdDiServizio;
struct sockaddr_in servaddr, servaddrDiServizio;
struct sockaddr_in ricevutoSuAddr;
fd_set descrittoriDiLettura, tuttiIdescrittori;
	
main() {

	printf("%d: Avvio del server...\n", getpid());
	
	createSocketStream(&listensd);
	createSocketStream(&listensdDiServizio);

	inizializza_memset(&servaddr, SERV_PORT);
	inizializza_memset(&servaddrDiServizio, SERVICE_PORT);
	
	bindSocket(&listensd, &servaddr);
	bindSocket(&listensdDiServizio, &servaddrDiServizio);
	
	listenSocket(&listensd, BACKLOG);
	listenSocket(&listensdDiServizio, BACKLOG);
	
	pid = fork();
	
	acceptFiglioNormale();
	
	if(pid != 0) {
			//Gestisce l'interruzione con ctrl-c
		(void) signal(SIGINT, interrompi);
		
		//il padre crea un altro figlio per le richieste di servizio
		pid = fork();
		acceptFiglioDiServizio();
			
		printf("%d: Server avviato\n", getpid());
		
		// -1 sta per aspetto qualasiasi figlio che termina, 0 sta per nessuna opzione, rimango bloccato fino a che non muore qualche figlio.
		waitpid(-1, &pid, 0);
		
		printf("%d: Il server è stato arrestato per qualche errore!\n", getpid());
		exit(0);
	}

}

void acceptFiglioNormale() {
	if(pid == 0) {
		while(1) {
			acceptSocket(&connsd, &listensd);
					//se è stata accettata una connessione normale...
			if(connsd != 0) {
				printf("%d: Creazione di un figlio in corso...\n", getpid());
				
				pid = fork();
			
				mainDelFiglio();
			
				int ritornoPid;
				
				//aspetto che un figlio termini
				waitpid(-1, &pid, WNOHANG);
				
				//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
				
				closeSocket(&connsd);
				
			}
		}
	}
}

void acceptFiglioDiServizio() {

	if(pid == 0) {
		while(1) {
			acceptSocket(&connsd, &listensdDiServizio);
			
					//se è stata accettata una connessione normale...
			if(connsd != 0) {
				printf("%d: Creazione di un figlio di servizio in corso...\n", getpid());
				
				pid = fork();
			
		// 				printf("%d: Figlio creato\n", getpid());
				
				mainDelFiglioDiServizio();
				
				//aspetto che un figlio termini
				waitpid(-1, &pidServizio, WNOHANG);
				
				//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
				
				closeSocket(&connsd);
			}
		}
	}
}

void mainDelFiglio() {
	
		if(pid == 0) 
		{
			time_t        ticks;
			char          buff[MAXLINE];
			
			printf("%d: Presa in consegna richiesta normale\n", getpid());
			
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

void mainDelFiglioDiServizio() {
	
		if(pid == 0) 
		{
			int n;
			char buff[MAXLINE];
			char recvline[MAXLINE];
			int socketDiScrittura;
			
			createSocketStream(&socketDiScrittura);

// 			strcpy(buff, "Risposta a cazzo di cane");
			snprintf(buff, sizeof(buff), "Dai dai dai..");
			
			printf("%d: Presa in consegna richiesta di servizio \n", getpid());
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			
			//se voglio sapere chi mi manda la richiesta..
			getpeername(connsd, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			
			printf("%d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta di servizio...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));

			while((n = recv(connsd, recvline, MAXLINE, 0)) > 0) {
				recvline[n] = 0;
			}

			printf("%d: Ho ricevuto: %s\n", getpid(), recvline);
			
// 			/* scrive sul socket di connessione il contenuto di buff */
			if (send(connsd, buff, strlen(buff), 0) != strlen(buff)) {
				printf("%d: ", getpid());
				perror("errore in write del figlio\n"); 
				exit(-1);
			}
			
			printf("%d: Richiesta elaborata!\n", getpid());
			
			close(connsd);
			exit(0);
		}
}

void interrompi() {
	printf("%d: Il server è stato terminato da console\n", getpid());
	
	exit(0);
}
