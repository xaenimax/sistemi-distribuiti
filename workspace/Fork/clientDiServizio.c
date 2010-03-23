#include "general.h"

#define SERV_PORT		6000
#define MAXLINE		1024

main() {

	int socketCl, numeroDatiRicevuti, i, numeroMessaggioInviato;
	char stringaInseritaDallutente[MAXLINE];
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	
	createSocketStream(&socketCl);
	
	memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	
	if(inet_pton(AF_INET, IP_ADDRESS, &servaddr.sin_addr) <= 0) {
		perror("Errore nella conversione dell'indirizzo");
		exit(-1);
	}

	connectSocket(&socketCl, &servaddr);
	
// 	int lunghezzaAddr = sizeof(servaddr);
// 		
		//se voglio sapere a chi mando la richiesta..
// 		getsockname(socketCl, (struct sockaddr *) &servaddr, &lunghezzaAddr);
// 		printf("%d: Il socket ha indirizzo: %s:%d.\n", getpid(), (char*)inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
	
// 		strcpy(bufferDiInvio, "Lista File");
	
	numeroMessaggioInviato = 1;
	
	while(1) {
		
		//prima di fare qualsiasi cosa svuoto le strutture dati di invio per evitare che siano "sporche" e contengano dati di precedenti invii
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Operazione da eseguire: \n");

		fflush(stdout);
		if ( fgets(stringaInseritaDallutente, sizeof(stringaInseritaDallutente), stdin) != NULL ) {
			
			char *newline = strchr(stringaInseritaDallutente, '\n'); /* search for newline character */
			
			if ( newline != NULL ) {
				*newline = '\0'; /* overwrite trailing newline */
			}
		}

		pacchettoApplicativo.numeroMessaggio = numeroMessaggioInviato;
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);
		
		/* se l'invio del messaggio va a buon fine incremento di uno il numero di messaggio in modo tale che il prossimo messaggio abbia un numero progressivo già incrementato di uno */
		if(sendPacchetto(&socketCl, &pacchettoApplicativo) > 0)
			numeroMessaggioInviato++;
	
		printf("Dati inviati. Attendo la ricezione di dati dal server\n");

		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		numeroDatiRicevuti = receivePacchetto(&socketCl, &pacchettoApplicativo, sizeof(pacchettoApplicativo), 0);
		
		printf("Dati ricevuti: [%s] \n%s\n", pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
		
		if(strcmp(pacchettoApplicativo.tipoOperazione, "Arrivederci") == 0) {
			closeSocket(&socketCl);
			break;
		}
		
	}
	
// 		close(socketCl);
	
	//sleep(1);
// 	}
	exit(0);
}