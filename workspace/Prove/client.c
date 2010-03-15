#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	int sockfd, n;
	char recvline[MAXLINE+1];
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Errore nell'apertura del socket");
		exit(-1);
	}
	
	memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	
	if(inet_pton(AF_INET, IP_ADDRESS, &servaddr.sin_addr) <= 0) {
		perror("Errore nella conversione dell'indirizzo");
		exit(-1);
	}
	
	if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("Errore nell'apertura della connessione");
		exit(-1);
	}
	
	strcpy(recvline, "Cazzo!");
	
	printf("Invio i dati: %s\n", recvline);
	
  if (send(sockfd, recvline, strlen(recvline),0) != strlen(recvline)) {
    perror("errore in write"); 
    exit(1);
  }

	bzero(recvline, MAXLINE);

	printf("Ricevo i dati\n");

	while((n = recv(sockfd, recvline, MAXLINE, 0)) > 0) {
		recvline[n] = 0;
		if(fputs(recvline, stdout) == EOF) {
			fprintf(stderr, "Errore in fputs");
			exit (-1);
		}
	}

	
	if(n < 0)
		perror("Errore nella read");

	close(sockfd);
	exit(0);
}