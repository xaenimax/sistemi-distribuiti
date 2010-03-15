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

	int socketCl, n, i;
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
			
			//se voglio sapere chi mi manda la richiesta..
		getsockname(socketCl, (struct sockaddr *) &servaddr, &lunghezzaAddr);
		printf("%d: Il socket ha indirizzo: %s:%d.\n", getpid(), (char*)inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
		
		snprintf(bufferDiInvio, sizeof(bufferDiInvio), "A cazzo di cane\r\n");

		printf("Invio i dati al server:\n");
		
		/* scrive sul socket di connessione il contenuto di buff */
		if (send(socketCl, bufferDiInvio, strlen(bufferDiInvio), 0) < 0) {
			printf("%d: ", getpid());
			perror("errore in write del figlio\n"); 
			exit(-1);
		}
	
 		printf("Dati inviati. Attendo la ricezione di dati dal server\n");

		bzero(recvline, MAXLINE);		
		
		while((n = recv(socketCl, recvline, MAXLINE, 0)) > 0) {
			recvline[n] = 0;
		}
		
		printf("Dati ricevuti: %s\n", recvline);
		
		if(n < 0)
			perror("Errore nella read");

		close(socketCl);
		//sleep(1);
// 	}
	exit(0);
}