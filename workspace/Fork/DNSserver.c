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



void fornisciDNS();
void acceptFiglioNormale();
void acceptClientDNS();
void interrompi();

int pid, pidServizio, i;
int connsd, listensdDiServizio, connsdDiServizio, connessioneNormale;
struct sockaddr_in servaddrDiServizio;
struct sockaddr_in ricevutoSuAddr;

static char** lista_server;
int server_scelto = 0; //ogni volta che un client si connette viene incrementato per selezionare sempre un server diverso
				   //con strategia ad anello

	
main() {
  
	printf("%d: Avvio del server...\n", getpid());
	
	createSocketStream(&listensdDiServizio);

	inizializza_memset(&servaddrDiServizio, SERVICE_PORT);
	
	bindSocket(&listensdDiServizio, &servaddrDiServizio);
	
	listenSocket(&listensdDiServizio, BACKLOG);
	


	lista_server = (char**)inizializza_lista();   //allocazione memoria per gli indirizzi

	prendi_indirizzi(lista_server);    //prelievo e memorizzazione indirizzi da file LISTA_SERVER

	//printf("prova funzioni ok %s", lista_server[4]);

	//--------------------------------------------------------




	pid = fork();  //creo un server figlio
	
	acceptClientDNS();  //che accetta richieste DNS
	

	if(pid > 0) {  //il padre è >0 (se è 0 è il figlio)

		(void) signal(SIGINT, interrompi); //Gestisce l'interruzione con ctrl-c
		
			
		printf("%d: Server avviato\n", getpid());
		
		// -1 sta per aspetto qualasiasi figlio che termina, 0 sta per nessuna opzione, rimango bloccato fino a che non muore qualche figlio.
		waitpid(-1, &pid, 0);
		
		printf("%d: Il server è stato arrestato per qualche errore!\n", getpid());
		exit(0);
	}

}   //-------------------------end main----------------


void acceptClientDNS() {

	if(pid == 0) {
		
		printf(" %d: In attesa di una richiesta di servizio...\n", getpid());
		
		while(1) {
			acceptSocket(&connessioneNormale, &listensdDiServizio);
			
					//se è stata accettata una connessione normale...
			if(connessioneNormale != 0) {
				printf("  %d: Creazione di un figlio di servizio in corso...\n", getpid());
				server_scelto = scegli_server(server_scelto);
				pid = fork();
			
		// 				printf("%d: Figlio creato\n", getpid());
				
				fornisciDNS();

				closeSocket(&connessioneNormale);
			}
		}
	}
}

void fornisciDNS() {

		if(pid == 0) 
		{
			int n;
			char buff[MAXLINE];
			char recvline[MAXLINE];

			closeSocket(&listensdDiServizio);
			
//############snprintf(buff, sizeof(buff), lista_server[3]);  //bufferizzo l'ip del server replica scelto, per poterlo inviare
			

			//printf("  %d: Presa in consegna richiesta di servizio.\n", getpid());
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			
			//se voglio sapere chi mi manda la richiesta..
			getpeername(connessioneNormale, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			
			printf("  %d: Ricevuta richiesta dall'indirizzo IP: %s : %d. Elaboro la richiesta di servizio...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));

			while((n = recv(connessioneNormale, recvline, MAXLINE, 0)) > 0) {
				printf("  %d: Messaggio ricevuto: %s.\n", getpid(), recvline, n);
			
				//if(strcmp(recvline, "Lista File") == 0) {
				if(recvline) {
				/*	int numeroDiFileTrovati = 0;
					struct direct **fileTrovati;
					
					printf("  %d: Invio la lista file, come richiesto.\n", getpid());
				
// 					chdir(directoryDeiFile);
					
					numeroDiFileTrovati = scandir(directoryDeiFile, &fileTrovati, NULL, NULL);

						// If no files found, make a non-selectable menu item
					if 		(numeroDiFileTrovati <= 0) {
						printf("  %d: Nessun file trovato!\n", getpid());
					}
					
					printf("  %d: Numero di file trovati: %d\n", getpid(), numeroDiFileTrovati);
					
					for (i=1;i<numeroDiFileTrovati+1;++i)
							printf("  %d: %s  \n", getpid(), fileTrovati[i-1]->d_name);
					printf("\n"); */


					snprintf(buff, sizeof(buff), lista_server[server_scelto]);

					sendData(&connessioneNormale, &buff);
				}
				
				//se non riconosco nessuna delle richieste che mi è giunta chiudo la connessione con il client
				else
					closeSocket(&connessioneNormale);
			}
			
			printf("  %d: Richiesta elaborata!\n\n", getpid());
			
			exit(0);
		}
}
//--------------------------------------------------
void interrompi() {
	printf("%d: Il server è stato terminato da console\n", getpid());
	exit(0);
}
