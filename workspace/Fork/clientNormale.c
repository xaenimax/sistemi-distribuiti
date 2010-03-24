#include "general.h"

#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	int socketCL, numeroDatiRicevuti, i;
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char stringaInseritaDallutente[MAXLINE];
		
	while(1) {

		createSocketStream(&socketCL);
		
		memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
		
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT);
		
		inetPton(&servaddr, IP_ADDRESS);
	
		connectSocket(&socketCL, &servaddr);

		while((n = read(socketCL, recvline, MAXLINE)) > 0) {
			recvline[n] = 0;
			if(fputs(recvline, stdout) == EOF) {
				fprintf(stderr, "Errore in fputs");
				exit (-1);
			}
		}
		
		if(n < 0)
			perror("Errore nella read");

		close(socketCL);
		sleep(1);
	}
	exit(0);
}