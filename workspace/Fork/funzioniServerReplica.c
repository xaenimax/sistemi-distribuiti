#include "general.h"

void spedisciFile(int *socketConnesso, FILE *fileDaLeggere, struct pacchetto *pacchettoApplicativo, int *numeroDatiRicevuti) {
	int dimensioneDelFile, numeroDiByteLetti;
	char bufferFileLetto[sizeof(pacchettoApplicativo->messaggio)];
	char nomeFileDaLeggere[sizeof(pacchettoApplicativo->nomeFile)];
	
	//la dimensione mi serve per capire quando ho finito di inviare il file
	fseek(fileDaLeggere, 0L, SEEK_END);
	dimensioneDelFile = ftell(fileDaLeggere);
	fseek(fileDaLeggere, 0L, SEEK_SET);

	bzero(pacchettoApplicativo, sizeof(struct pacchetto));
	
	strcpy(pacchettoApplicativo->tipoOperazione, "leggi file");
	strcpy(pacchettoApplicativo->messaggio, "File trovato");
	strcpy(pacchettoApplicativo->nomeFile, nomeFileDaLeggere);
	
	sendPacchetto(socketConnesso, pacchettoApplicativo);
	
	while(dimensioneDelFile != numeroDiByteLetti) {
		
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		bzero(bufferFileLetto, sizeof(bufferFileLetto));
		
		numeroDiByteLetti = numeroDiByteLetti + fread(bufferFileLetto, sizeof(char), sizeof(pacchettoApplicativo->messaggio), fileDaLeggere);
	
		memcpy(pacchettoApplicativo->messaggio, bufferFileLetto, sizeof(pacchettoApplicativo->messaggio));
		
		strcpy(pacchettoApplicativo->tipoOperazione, "leggi file");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
// 		printf("i: %s\n", pacchettoApplicativo->messaggio); //se si vuole controllare cosa si spedisce al client decommentare questa riga!!
		printf("  %d: Inviati: %d / %d byte\n", getpid(), numeroDiByteLetti, dimensioneDelFile);
	}
	
	printf("  %d: Terminata lettura del file, letti %d / %d byte\n", getpid(), numeroDiByteLetti, dimensioneDelFile);
	
	bzero(pacchettoApplicativo, sizeof(struct pacchetto));
	
	strcpy(pacchettoApplicativo->tipoOperazione, "leggi file");
	strcpy(pacchettoApplicativo->messaggio, "esci");
	
	sendPacchetto(socketConnesso, pacchettoApplicativo);
}