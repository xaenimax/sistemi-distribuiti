#include "general.h"
#include "funzioniGeneriche.h"

//Serve ad inserire del testo da stdin. Salva il testo in buffer NON prende \n
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

/*spedisce un file al client creando una copia*/
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
// 			printf("  %d: Letti: %ld / %ld byte. N: %d\n", getpid(), numeroDiByteLetti, dimensioneDelFile, numeroDiPartiCompleteDaInviare);
		}
		
		if(numeroDiPartiCompleteDaInviare == 0) {
			int dimensioneUltimaParteDaInviare = dimensioneDelFile % (sizeof(pacchettoApplicativo->messaggio));
			numeroDiByteLetti = numeroDiByteLetti + fread(pacchettoApplicativo->messaggio, 1, dimensioneUltimaParteDaInviare, fileDaLeggere);
// 			printf("  %d: Letti: %ld / %ld byte. N: %d\n", getpid(), numeroDiByteLetti, dimensioneDelFile, numeroDiPartiCompleteDaInviare);
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

//Riceve un file spedito con la funzione spedisci file. Prima di richiamare questa funzione bisogna fare in modo che il A avvisi B che vuole spedire un e B risponda che è pronto a ricevere. Dopodichè bisogna fare un'ulteriore receive perchè A comunicherà a B la dimensione del file che per inviare
void riceviFile(int *socketConnesso, char *nomeFileDaScrivereConPercorso, struct pacchetto *pacchettoApplicativo) {	
	
	FILE *fileDaScrivere = fopen(nomeFileDaScrivereConPercorso, "wb");
	
	unsigned long int dimensioneDelFileRicevuto = atoi(pacchettoApplicativo->messaggio);
	int numeroDiPartiCompleteDaRicevere = dimensioneDelFileRicevuto / (sizeof(pacchettoApplicativo->messaggio));
	unsigned long int numeroDiDatiScritti = 0;
	
	while(numeroDiPartiCompleteDaRicevere >= 0) {
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		
		receivePacchetto(socketConnesso, pacchettoApplicativo, sizeof(struct pacchetto), 0);
// 				receiveData(&socketCl, pacchettoApplicativo.messaggio, sizeof(pacchettoApplicativo.messaggio));
		
// 		printf("  %d: [%s]Ricevuta parte numero %d.\n",getpid(), pacchettoApplicativo->tipoOperazione, numeroDiPartiCompleteDaRicevere);
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

//copia un file in un altro. Se apriFile è > 0 apre i file prima di copiarli, altrimenti il file va aperto prima di richiamare la funzione
//Se scritturaConLock > 0 la funzione prevede in ingresso un descrittore file fileOriginaleDaCopiare di cui bisogna fare la fopen fuori se apriFile è = 0. Inoltre, bisogna passare percorsoFileDiDestinazione per la destinazione perchè la funzione che fa la scrittura atomica ha bisogno del percorso del file da scrivere e non del descrittore.
int copiaFile(FILE *fileOriginaleDaCopiare, FILE *fileDiDestinazione, char *percorsoFileOriginale, char *percorsoFileDiDestinazione, int apriFile, int scritturaConLock) {
	int dimensioneFile, numeroDiPartiDaLeggere, descrittoreFilePerWriteLock;
	char bufferTemporaneo[500];
	
	if(apriFile > 0) {
		fileOriginaleDaCopiare=fopen(percorsoFileOriginale, "rb");
		fileDiDestinazione=fopen(percorsoFileDiDestinazione, "wb");
		
		if(fileOriginaleDaCopiare < 0 || fileDiDestinazione < 0) {
			if(fileOriginaleDaCopiare < 0) {
				perror("\n");
				printf("  %d: Errore durante la copia del file \'%s\'\n", getpid(), percorsoFileOriginale);
			}
			if(fileDiDestinazione < 0) {
				perror("\n");
				printf("  %d: Errore durante la copia del file \'%s\'", getpid(), percorsoFileDiDestinazione);
			}
			return -1;
		}
	}
	
	if(scritturaConLock > 0)
		descrittoreFilePerWriteLock = open(percorsoFileDiDestinazione, O_WRONLY|O_APPEND, 0666);
	
	fseek(fileOriginaleDaCopiare,0L,SEEK_END);
	dimensioneFile= ftell(fileOriginaleDaCopiare);
	fseek(fileOriginaleDaCopiare,0L,SEEK_SET);
		
// 	copia un file su quello temporaneo
	numeroDiPartiDaLeggere=dimensioneFile/(sizeof(bufferTemporaneo));
	
	while(numeroDiPartiDaLeggere!=0){
		bzero(bufferTemporaneo,sizeof(bufferTemporaneo));
		fread( bufferTemporaneo,1, sizeof(bufferTemporaneo), fileOriginaleDaCopiare);
		if(scritturaConLock > 0)
			writeFileWithLock(descrittoreFilePerWriteLock, bufferTemporaneo,0,0);
		else
			fwrite( bufferTemporaneo, 1, sizeof(bufferTemporaneo), fileDiDestinazione);
		numeroDiPartiDaLeggere--;
	}
	if(numeroDiPartiDaLeggere==0){
		bzero(bufferTemporaneo,sizeof(bufferTemporaneo));
		int numeroDiPartiDaLeggereAncora=dimensioneFile % sizeof(bufferTemporaneo);
		fread( bufferTemporaneo,1, numeroDiPartiDaLeggereAncora, fileOriginaleDaCopiare);
		if(scritturaConLock > 0)
			writeFileWithLock(descrittoreFilePerWriteLock, bufferTemporaneo,0,0);
		else
			fwrite( bufferTemporaneo, 1, sizeof(bufferTemporaneo), fileDiDestinazione);
	}
	
	return 1;
}

void svuotaStrutturaListaFile(struct fileApertiDalServer *listaFile) {
	int i;
	for(i = 0; i < 10; i++)
		bzero((listaFile+i), sizeof(struct fileApertiDalServer));
}

void stampaIpEporta(struct sockaddr_in *indirizzoIP) {
		printf("%s:%d", inet_ntoa(indirizzoIP->sin_addr), ntohs(indirizzoIP->sin_port));
}


void writeFileWithLock(int descrittoreFile, char *contenutoDaScrivere, int stampaAvideo, int aggiungiData) {

	char stringaFinaleDaScrivere[600];
	
	if(aggiungiData > 0) {
		time_t dataEoraTimeT;
		struct tm * dataEora;

		time(&dataEoraTimeT);
		dataEora = localtime(&dataEoraTimeT);

		strftime(stringaFinaleDaScrivere,80,"%d/%m/%y %X ",dataEora);
		strcat(stringaFinaleDaScrivere, contenutoDaScrivere);
	}
	else
		strcpy(stringaFinaleDaScrivere, contenutoDaScrivere);
	
	struct flock opzioniDiFileLock = { F_WRLCK, SEEK_SET, 0, 0, 0 };
	struct flock opzioniDiFileUnlock = { F_UNLCK, SEEK_SET, 0, 0, 0 };
	
	fcntl(descrittoreFile, F_SETLKW, &opzioniDiFileLock);
	write(descrittoreFile,stringaFinaleDaScrivere,strlen(stringaFinaleDaScrivere));
	fcntl(descrittoreFile, F_SETLKW, &opzioniDiFileUnlock);
	
	if(stampaAvideo > 0)
		printf("%s", stringaFinaleDaScrivere);
}
