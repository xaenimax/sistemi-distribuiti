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

//Vedere perche' il client e il server si bloccano dopo essersi scambiati i primi dati. Probabilmente bisogna vedere la readn e la writen

main() {

	int socketCl, n, i;
	char recvline[MAXLINE], bufferDiInvio[MAXLINE];
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
		

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
			
		getsockname(socketCl, (struct sockaddr *) &servaddr, &lunghezzaAddr); //se voglio sapere chi mi manda la richiesta..

		printf("%d: Avviato client - Indirizzo: %s Porta: %d.\n", getpid(), (char*)inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
		
		strcpy(bufferDiInvio, "Richiesto indirizzo di un server replica");

		printf("Invio i richiesta al server:\n");

		/* scrive sul socket di connessione il contenuto di buff */
		if (send(socketCl, bufferDiInvio, strlen(bufferDiInvio), 0) < 0) {  //invio la richiesta
			printf("%d: ", getpid());
			perror("errore in write del figlio\n");
			exit(-1);
		}

 		printf("Dati inviati. Attendo la ricezione di dati dal server\n");

		bzero(recvline, MAXLINE); //svuota array
		
		while((n = recv(socketCl, recvline, MAXLINE, 0)) > 0) {

			printf("Dati ricevuti: %s\n", recvline);
			
			if(n < 0)
				perror("Errore nella read");

			close(socketCl);
		}

		//sleep(1);
// 	}
	exit(0);
}
