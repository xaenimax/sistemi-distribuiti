#include "../general.h"

#define SERV_PORT		5193
#define MAXLINE		1024
/* Questo Client è usato per provare le funzioni di servizio del server */

main() {

	int socketCl, numeroDatiRicevuti, i, numeroMessaggioInviato;
	char stringaInseritaDallutente[MAXLINE];
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char *cartellaDoveSalvareIfile = "fileCondivisi/";
	
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

		inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));

		pacchettoApplicativo.numeroMessaggio = numeroMessaggioInviato;
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);
		
		//se l'utente vuole leggere un file, chiedo il nome del file che vuole leggere
		//ATTENZIONE! Siccome Vienna sta lavorando sul client normale, scrivo questa funzione qua
		//che andrà spostata dentro il client normale
		if(strncmp("leggi file", stringaInseritaDallutente, 10) == 0) {
			printf("Inserire il nome del file che si intende leggere:\n");
			bzero(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			strcpy(pacchettoApplicativo.nomeFile, stringaInseritaDallutente);
		}
		 
		/* se l'invio del messaggio va a buon fine incremento di uno il numero di messaggio in modo tale che il prossimo messaggio abbia un numero progressivo già incrementato di uno */
		if(sendPacchetto(&socketCl, &pacchettoApplicativo) > 0)
			numeroMessaggioInviato++;
	
		printf("Dati inviati. Attendo la ricezione di dati dal server\n");

		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		receivePacchetto(&socketCl, &pacchettoApplicativo, sizeof(pacchettoApplicativo), 0);
		
		if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file, trovato") == 0) {
			
			char nomeFileDaScrivereConPercorso[sizeof(cartellaDoveSalvareIfile) + sizeof(pacchettoApplicativo.nomeFile)];
			strcpy(nomeFileDaScrivereConPercorso, cartellaDoveSalvareIfile);
			strcat(nomeFileDaScrivereConPercorso, pacchettoApplicativo.nomeFile);
			
			FILE *fileDaScrivere = fopen(nomeFileDaScrivereConPercorso, "wb");
			
			unsigned long int dimensioneDelFileRicevuto = atoi(pacchettoApplicativo.messaggio);
			int numeroDiPartiCompleteDaRicevere = dimensioneDelFileRicevuto / (sizeof(pacchettoApplicativo.messaggio)-5);
			unsigned long int numeroDiDatiScritti = 0;
			
			while(numeroDiPartiCompleteDaRicevere >= 0) {
				bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
				
				receivePacchetto(&socketCl, &pacchettoApplicativo, sizeof(pacchettoApplicativo), 0);
				
				if(numeroDiPartiCompleteDaRicevere != 0) {					
					numeroDiDatiScritti = numeroDiDatiScritti + fwrite(pacchettoApplicativo.messaggio, 1, (sizeof(pacchettoApplicativo.messaggio)-5), fileDaScrivere);
					printf("[%s] Scritti %ld dati.\n", pacchettoApplicativo.tipoOperazione, numeroDiDatiScritti);
				}
				
				if(numeroDiPartiCompleteDaRicevere == 0) {
					int dimensioneUltimaParteDaRicevere = dimensioneDelFileRicevuto % (sizeof(pacchettoApplicativo.messaggio)-5);
					numeroDiDatiScritti = numeroDiDatiScritti + fwrite(pacchettoApplicativo.messaggio, 1, dimensioneUltimaParteDaRicevere, fileDaScrivere);
					printf("[%s] Scritti %ld dati.\n", pacchettoApplicativo.tipoOperazione, numeroDiDatiScritti);
				}
				numeroDiPartiCompleteDaRicevere--;
			}
			
			printf("[%s] Ho scritto %ld dati.\n", pacchettoApplicativo.tipoOperazione, numeroDiDatiScritti);
			
			fclose(fileDaScrivere);
		}
		
		if(strcmp(pacchettoApplicativo.tipoOperazione, "Arrivederci") == 0) {
			printf("[%s] %s\n", pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
			closeSocket(&socketCl);
			break;
		}
		
	}
	
// 		close(socketCl);
	
	//sleep(1);
// 	}
	exit(0);
}