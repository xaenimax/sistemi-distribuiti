#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERV_PORT		6000
#define MAXLINE		1024

//Vedere perché il client e il server si bloccano dopo essersi scambiati i primi dati. Probabilmente bisogna vedere la readn e la writen

main() {

	int socketCl, numeroDatiRicevuti, i;
	char recvline[MAXLINE], bufferDiInvio[MAXLINE];
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	
// 	for( ; ; ){
// 		if((socketCl = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
// 			perror("Errore nell'apertura del socket");
// 			exit(-1);
// 		}
		createSocketStream(&socketCl);
		
		memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
		
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT);
		
		if(inet_pton(AF_INET, IP_ADDRESS, &servaddr.sin_addr) <= 0) {
			perror("Errore nella conversione dell'indirizzo");
			exit(-1);
		}

		connectSocket(&socketCl, &servaddr);
		
		int lunghezzaAddr = sizeof(servaddr);
			
			//se voglio sapere a chi mando la richiesta..
// 		getsockname(socketCl, (struct sockaddr *) &servaddr, &lunghezzaAddr);
// 		printf("%d: Il socket ha indirizzo: %s:%d.\n", getpid(), (char*)inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
		
// 		strcpy(bufferDiInvio, "Lista File");
		
		//finchè il socket rimane aperto...
		while(1) {
			
			printf("Operazione da eseguire: \n");
// 			bzero(bufferDiInvio, sizeof(bufferDiInvio));
			//%[^\n] vuol dire che accetto tutto in ingresso escluso \n. Mi serve per includere anche gli spazi che digita l'utente

// 			scanf("%s%*[^\n]", bufferDiInvio);
				fflush(stdout);
			if ( fgets(bufferDiInvio, sizeof(bufferDiInvio), stdin) != NULL ) {
				
					char *newline = strchr(bufferDiInvio, '\n'); /* search for newline character */
					
					if ( newline != NULL ) {
						*newline = '\0'; /* overwrite trailing newline */
						}
			}
			
// 			printf("Invio \'%s\' al server:\n", bufferDiInvio);
			
			/* scrive sul socket di connessione il contenuto di buff */
			sendData(&socketCl, &bufferDiInvio);
		
			printf("Dati inviati. Attendo la ricezione di dati dal server\n");

			bzero(recvline, sizeof(recvline));		
			
			numeroDatiRicevuti = receiveData(&socketCl, &recvline, MAXLINE);
			
			printf("Dati ricevuti: \n%s\n", recvline);
			
			if(strcmp(recvline, "Arrivederci") == 0) {
				closeSocket(&socketCl);
				break;
			}
			
		}
		
// 		close(socketCl);
		
		//sleep(1);
// 	}
	exit(0);
}