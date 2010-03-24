#include "general.h"

#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	int socketCL, numeroDatiRicevuti, i;
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char stringaInseritaDallutente[MAXLINE];
		
	
	for( ; ; ){
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
	
		connectSocket(&sockfd, &servaddr);

		while((n = read(sockfd, recvline, MAXLINE)) > 0) {
			recvline[n] = 0;
			if(fputs(recvline, stdout) == EOF) {
				fprintf(stderr, "Errore in fputs");
				exit (-1);
			}
		}
		
		if(n < 0)
			perror("Errore nella read");

		close(sockfd);
		sleep(1);
	}
	exit(0);
}