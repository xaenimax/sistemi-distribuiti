#include "general.h"

//Serve ad inserire del testo da stdin. Salva il testo in buffer
void inserisciTesto(char *bufferDoveInserireIlTesto, int dimensioneDelBufferDiTesto) {
	
	fflush(stdout);
	
	if ( fgets(bufferDoveInserireIlTesto, dimensioneDelBufferDiTesto, stdin) != NULL ) {
		
			char *newline = strchr(bufferDoveInserireIlTesto, '\n'); /* search for newline character */
			
			if ( newline != NULL ) {
				*newline = '\0'; /* overwrite trailing newline */
				}
	}
}

//genera 
void generaIDtransazione(char *idTransazione) {

	int i, numeroRandom;
	i = 0;
	srand(time(NULL));

	while(i < 10) {
		
		//Genera numeri RANDOM da 48 a 122. (122-48=74. Genero numeri random da 0 a 74 e aggiungo 48)
		numeroRandom = 48 + (rand()/(int)(((unsigned)RAND_MAX + 1) / 74));

		//Se il numero random generato è compreso tra questi valori ho i caratteri 0-9,a-z,A-Z
		if((numeroRandom >= 48 && numeroRandom <= 57) || (numeroRandom >= 65 && numeroRandom <= 90) || (numeroRandom >= 97 && numeroRandom <= 122)) {
			idTransazione[i] = numeroRandom;
			i++;
		}
	}
	
	idTransazione[10] = '\0';
}


void spedisciFile(int *socketConnesso, FILE *fileDaLeggere, struct pacchetto *pacchettoApplicativo) {
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
	
	int numeroDiPartiCompleteDaInviare = dimensioneDelFile / (sizeof(pacchettoApplicativo->messaggio));
	
	while(numeroDiPartiCompleteDaInviare >= 0) {
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		bzero(bufferFileLetto, sizeof(bufferFileLetto));
		
		char numeroParteInviata[100];
		strcpy(pacchettoApplicativo->tipoOperazione, "leggi file, trovato");
		sprintf(numeroParteInviata, " %d", numeroDiPartiCompleteDaInviare);
		strcat(pacchettoApplicativo->tipoOperazione, numeroParteInviata);
		
		if(numeroDiPartiCompleteDaInviare > 0) {
			numeroDiByteLetti = numeroDiByteLetti + fread(pacchettoApplicativo->messaggio, 1, (sizeof(pacchettoApplicativo->messaggio)), fileDaLeggere);
			printf("  %d: Letti: %ld / %ld byte. N: %d\n", getpid(), numeroDiByteLetti, dimensioneDelFile, numeroDiPartiCompleteDaInviare);
		}
		
		if(numeroDiPartiCompleteDaInviare == 0) {
			int dimensioneUltimaParteDaInviare = dimensioneDelFile % (sizeof(pacchettoApplicativo->messaggio));
			numeroDiByteLetti = numeroDiByteLetti + fread(pacchettoApplicativo->messaggio, 1, dimensioneUltimaParteDaInviare, fileDaLeggere);
			printf("  %d: Letti: %ld / %ld byte. N: %d\n", getpid(), numeroDiByteLetti, dimensioneDelFile, numeroDiPartiCompleteDaInviare);
		}
		
		sleep(1);
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

void riceviFile(int *socketConnesso, char *nomeFileDaScrivereConPercorso, struct pacchetto *pacchettoApplicativo) {	
	
	FILE *fileDaScrivere = fopen(nomeFileDaScrivereConPercorso, "wb");
	
	unsigned long int dimensioneDelFileRicevuto = atoi(pacchettoApplicativo->messaggio);
	int numeroDiPartiCompleteDaRicevere = dimensioneDelFileRicevuto / (sizeof(pacchettoApplicativo->messaggio));
	unsigned long int numeroDiDatiScritti = 0;
	
	while(numeroDiPartiCompleteDaRicevere >= 0) {
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		
		receivePacchetto(socketConnesso, pacchettoApplicativo, sizeof(struct pacchetto), 0);
// 				receiveData(&socketCl, pacchettoApplicativo.messaggio, sizeof(pacchettoApplicativo.messaggio));
		
		printf("[%s]Ricevuta parte numero %d.\n", pacchettoApplicativo->tipoOperazione, numeroDiPartiCompleteDaRicevere);
		
		if(numeroDiPartiCompleteDaRicevere != 0) {					
			numeroDiDatiScritti = numeroDiDatiScritti + fwrite(pacchettoApplicativo->messaggio, 1, (sizeof(pacchettoApplicativo->messaggio)), fileDaScrivere);
		}
		
		if(numeroDiPartiCompleteDaRicevere == 0) {
			int dimensioneUltimaParteDaRicevere = dimensioneDelFileRicevuto % (sizeof(pacchettoApplicativo->messaggio));
			numeroDiDatiScritti = numeroDiDatiScritti + fwrite(pacchettoApplicativo->messaggio, 1, dimensioneUltimaParteDaRicevere, fileDaScrivere);
		}
		
		numeroDiPartiCompleteDaRicevere--;
	}
	
	printf("[%s] Ho scritto %ld dati.\n", pacchettoApplicativo->tipoOperazione, numeroDiDatiScritti);
	
	fclose(fileDaScrivere);
}