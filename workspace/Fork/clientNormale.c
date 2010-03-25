#include "general.h"

#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	int socketCL, numeroDatiRicevuti, i, numeroMessaggioInviato;
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char stringaInseritaDallutente[MAXLINE];
		
	numeroMessaggioInviato = 1;
	
	createSocketStream(&socketCL);
	
	memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	
	inetPton(&servaddr, IP_ADDRESS);

	connectSocket(&socketCL, &servaddr);

	while(1) {
		
		bzero(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Operazione da eseguire:\n");
		inserisciTesto(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		printf("%s\n", stringaInseritaDallutente);
		
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);
		pacchettoApplicativo.numeroMessaggio = numeroMessaggioInviato;
		
		printf("Invio i dati...\n");
		
		if(sendPacchetto(&socketCL, &pacchettoApplicativo) > 0)
			numeroMessaggioInviato++;
		
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		receivePacchetto(&socketCL, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Operazione ricevuta: %s\n", pacchettoApplicativo.tipoOperazione);
		
		if(strcmp(pacchettoApplicativo.tipoOperazione, "Arrivederci") == 0) {
			printf("Chiudo la connessione\n");
			closeSocket(&socketCL);
			break;
		}
		
		else if(strcmp(pacchettoApplicativo.tipoOperazione, "Sconosciuta") == 0) {
			printf("%s\n", pacchettoApplicativo.messaggio);
		}
	}
	exit(0);
}