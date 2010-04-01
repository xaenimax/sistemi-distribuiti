#include "../general.h"

void spedisciFile(int *socketConnesso, FILE *fileDaLeggere, struct pacchetto *pacchettoApplicativo, int *numeroDatiRicevuti) {
	unsigned long int dimensioneDelFile = 0, numeroDiByteLetti = 0;
	char bufferFileLetto[sizeof(pacchettoApplicativo->messaggio)];
	char nomeFileDaLeggere[sizeof(pacchettoApplicativo->nomeFile)];
	
	strcpy(nomeFileDaLeggere, pacchettoApplicativo->nomeFile);
	
	//la dimensione mi serve per capire quando ho finito di inviare il file
	fseek(fileDaLeggere, 0L, SEEK_END);
	dimensioneDelFile = ftell(fileDaLeggere);
	fseek(fileDaLeggere, 0L, SEEK_SET);
	
	bzero(pacchettoApplicativo, sizeof(struct pacchetto));
	
	strcpy(pacchettoApplicativo->tipoOperazione, "leggi file, trovato");
	sprintf(pacchettoApplicativo->messaggio, "%ld", dimensioneDelFile);
	strcpy(pacchettoApplicativo->nomeFile, nomeFileDaLeggere);
	
	sendPacchetto(socketConnesso, pacchettoApplicativo);
	
	int numeroDiPartiCompleteDaInviare = dimensioneDelFile / (sizeof(pacchettoApplicativo->messaggio)-5);
	
	while(numeroDiPartiCompleteDaInviare >= 0) {
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		bzero(bufferFileLetto, sizeof(bufferFileLetto));
		
		strcpy(pacchettoApplicativo->tipoOperazione, "leggi file, trovato");
		
		if(numeroDiPartiCompleteDaInviare > 0) {
			numeroDiByteLetti = numeroDiByteLetti + fread(pacchettoApplicativo->messaggio, 1, (sizeof(pacchettoApplicativo->messaggio)-5), fileDaLeggere);
			printf("  %d: Letti: %ld / %ld byte. N: %d\n", getpid(), numeroDiByteLetti, dimensioneDelFile, numeroDiPartiCompleteDaInviare);
		}
		
		if(numeroDiPartiCompleteDaInviare == 0) {
			int dimensioneUltimaParteDaInviare = dimensioneDelFile % (sizeof(pacchettoApplicativo->messaggio)-5);
			numeroDiByteLetti = numeroDiByteLetti + fread(pacchettoApplicativo->messaggio, 1, dimensioneUltimaParteDaInviare, fileDaLeggere);
			printf("  %d: Letti: %ld / %ld byte. N: %d\n", getpid(), numeroDiByteLetti, dimensioneDelFile, numeroDiPartiCompleteDaInviare);
		}

		strcpy(pacchettoApplicativo->tipoOperazione, "leggi file, trovato");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
// 		printf("i: %s\n", pacchettoApplicativo->messaggio); //se si vuole controllare cosa si spedisce al client decommentare questa riga!!
		numeroDiPartiCompleteDaInviare--;
	}
	
	printf("  %d: Terminata lettura del file, letti %ld / %ld byte\n", getpid(), numeroDiByteLetti, dimensioneDelFile);
	
// 	bzero(pacchettoApplicativo, sizeof(struct pacchetto));
// 	
// 	strcpy(pacchettoApplicativo->tipoOperazione, "leggi file, trovato");
// 	strcpy(pacchettoApplicativo->messaggio, "esci");
// 	
// 	sendPacchetto(socketConnesso, pacchettoApplicativo);
}


//Effettua la "dir" nella cartella del filesystem distribuito e la invia al client connesso al socket
void inviaListaFile(int *socketConnesso, char *directoryDeiFile) {
	int numeroDiFileTrovati = 0;
	int i;
	struct direct **fileTrovati;
	struct pacchetto pacchettoDaInviare;
	char stringaDiFileTrovati[1024];
	
	//svuoto il buffer di invio
	bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
	
	printf("  %d: Invio la lista file, come richiesto.\n", getpid());

	numeroDiFileTrovati = scandir(directoryDeiFile, &fileTrovati, NULL, alphasort);
		/* If no files found, make a non-selectable menu item */
	if 		(numeroDiFileTrovati <= 0) {
		strcpy(pacchettoDaInviare.tipoOperazione, "lista file");
		strcpy(pacchettoDaInviare.messaggio, "Nessun file trovato!");
		printf("  %d: Nessun file trovato!\n", getpid());
	}
	else {
		for (i=1;i<numeroDiFileTrovati+1;++i) {
				sprintf(stringaDiFileTrovati, "%s\r\n", fileTrovati[i-1]->d_name);
				strcat(pacchettoDaInviare.messaggio, stringaDiFileTrovati);
		}
	}
	
	sendPacchetto(socketConnesso, &pacchettoDaInviare, sizeof(pacchettoDaInviare), 0);
}