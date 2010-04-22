#include "../general.h"

#define SERV_PORT		5193
#define MAXLINE		1024
/* Questo Client è usato per provare le funzioni di servizio del server */

main(int argc, char *argv[]) {

	int socketCl, numeroDatiRicevuti, i, numeroMessaggioInviato;
	char stringaInseritaDallutente[MAXLINE];
	char indirizzoIpDelServer[15];	
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char *cartellaDoveSalvareIfile = "fileCondivisi/";
	
	if(strlen(argv[1]) < 8) {
		printf("E' necessario specificare l'IP del server da contattare\n");
		exit(-1);
	}
	
	strcpy(indirizzoIpDelServer, argv[1]);
	
	createSocketStream(&socketCl);
	
	memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	
	if(inet_pton(AF_INET, indirizzoIpDelServer, &servaddr.sin_addr) <= 0) {
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
					
		FILE *fileDaLeggere;
		//prima di fare qualsiasi cosa svuoto le strutture dati di invio per evitare che siano "sporche" e contengano dati di precedenti invii
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Operazione da eseguire: \n");

		inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));

		pacchettoApplicativo.numeroMessaggio = numeroMessaggioInviato;
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);
		
		//se l'utente vuole leggere un file, chiedo il nome del file che vuole leggere
		//ATTENZIONE! Siccome Vienna sta lavorando sul client normale, scrivo questa funzione qua
		//che andrà spostata dentro il client normale
		if(strncmp("leggi file", stringaInseritaDallutente, 11) == 0) {
			printf("Inserire il nome del file che si intende leggere:\n");
			bzero(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			strcpy(pacchettoApplicativo.nomeFile, stringaInseritaDallutente);
		}
		
		if(strncmp("copia file", stringaInseritaDallutente, 11) == 0) {
			printf("Inserire il nome del file che si intende scrivere:\n");
			bzero(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			strcpy(pacchettoApplicativo.nomeFile, stringaInseritaDallutente);
			
			char nomeFileDaLeggere[500];
			
			strcpy(nomeFileDaLeggere, cartellaDoveSalvareIfile);
			strcat(nomeFileDaLeggere, pacchettoApplicativo.nomeFile);
			
			fileDaLeggere = fopen(nomeFileDaLeggere, "rb");
			
			//se non trovo il file spedisco un messaggio e avverto il client
			if(fileDaLeggere == NULL) {
				printf("  %d: File \'%s\'non trovato\n", getpid(), pacchettoApplicativo.nomeFile);
				bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
				strcpy(pacchettoApplicativo.tipoOperazione, "non inviare");
			}
		}		 
		
		if(strncmp("commit", stringaInseritaDallutente, 6) == 0) {
			
		}
		
		//evito di inviare il pacchetto se nel tipo operazione scrivo "non inviare". Ciò accade ad esempio quando l'utente inserisce un nome file da inviare
		//che non trovo nella mia cartella.
		if(strcmp(pacchettoApplicativo.tipoOperazione, "non inviare") != 0) {
			/* se l'invio del messaggio va a buon fine incremento di uno il numero di messaggio in modo tale che il prossimo messaggio abbia un numero progressivo già incrementato di uno */
			if(sendPacchetto(&socketCl, &pacchettoApplicativo) > 0)
				numeroMessaggioInviato++;
	
			printf("Dati inviati. Attendo la ricezione di dati dal server\n");

			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
			receivePacchetto(&socketCl, &pacchettoApplicativo, sizeof(pacchettoApplicativo), 0);
		}
		
		//se il server ha trovato il file me lo comunica e comincio a scriverlo
		if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file, trovato") == 0) {
			
			char nomeFileDaScrivereConPercorso[sizeof(cartellaDoveSalvareIfile) + sizeof(pacchettoApplicativo.nomeFile)];
			strcpy(nomeFileDaScrivereConPercorso, cartellaDoveSalvareIfile);
			strcat(nomeFileDaScrivereConPercorso, pacchettoApplicativo.nomeFile);
			
			riceviFile(&socketCl, nomeFileDaScrivereConPercorso, &pacchettoApplicativo);
		}
		
		//se il server è pronto a ricevere il file me lo comunica, insieme all'id transazione e inizio l'invio
		if(strcmp(pacchettoApplicativo.tipoOperazione, "copia file, pronto a ricevere") == 0) {
			
			spedisciFile(&socketCl, fileDaLeggere, &pacchettoApplicativo);	
 			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
	
		}
		
		//se il server fa una qualunque altra operazione che non sia collegata al file trovato, stampo il messaggio che altrimenti sarebbe incomprensibile
		if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file, trovato") < 0) {
			printf("[%s] %s\n", pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
		}
		
		//operazione di uscita
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