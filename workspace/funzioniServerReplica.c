#include "general.h"
// #include "funzioniServerReplica.h"
#include "funzioniGeneriche.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

int richiestaScritturaFile(char *IDgenerato, char *nomeFileDaSostituireConPercorso,struct pacchetto *pacchettoApplicativo,int *destinazione){
	
	FILE 	*fileDiScritturaMomentanea=fopen(IDgenerato,"a");
	FILE *fileOriginaleDaCopiare=fopen(nomeFileDaSostituireConPercorso,"rb");
	
	unsigned long int dimensioneFile=0,numeroByteLetti=0;
	int numeroDiPartiDaLeggere;
	
	char buffer[500], stringaImmessa[100];
	
			
	printf("Apro il primo file temporaneo\n");
	// apro i file con relativi controlli di errore
	if (fileDiScritturaMomentanea<0){
		perror("Errore nella creazione del file temporaneo");
		exit(-1);
	}
	printf("Apro il secondo file da copiare\n");
	if(fileOriginaleDaCopiare<0){
		perror("Errore nell'apertura del file da copiare");
		exit(-1);
	}
	fseek(fileOriginaleDaCopiare,0L,SEEK_END);
	dimensioneFile= ftell(fileOriginaleDaCopiare);
	fseek(fileOriginaleDaCopiare,0L,SEEK_SET);
	
	printf("copia in corso\n");
	
// 	copia un file su quello temporaneo
	numeroDiPartiDaLeggere=dimensioneFile/(sizeof(buffer));
	
	while(numeroDiPartiDaLeggere!=0){
		fread( buffer,1, sizeof(buffer), fileOriginaleDaCopiare);
		fwrite( buffer, 1, sizeof(buffer), fileDiScritturaMomentanea);
		numeroDiPartiDaLeggere--;
	}
	if(numeroDiPartiDaLeggere==0){
		int numeroDiPartiDaLeggereAncora=dimensioneFile % sizeof(buffer);
		fread( buffer,1, numeroDiPartiDaLeggereAncora, fileOriginaleDaCopiare);
		fwrite( buffer, 1, numeroDiPartiDaLeggereAncora, fileDiScritturaMomentanea);
	}
	
	printf("Copia completata\n");
	
// prende le cose scritte dall'utente e le aggiunge al file temporaneo
	//svuoto il buffer di invio
	bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
	strcpy(pacchettoApplicativo->messaggio,"inserisci le modifiche, scrivi commit per effettuarle, abort per annullare\n");
	
	while((strcmp(pacchettoApplicativo->messaggio,"commit\n")!=0)&&(strcmp(pacchettoApplicativo->messaggio,"abort\n")!=0)){
		sendPacchetto(&destinazione,&pacchettoApplicativo);
		bzero(pacchettoApplicativo->messaggio,sizeof(pacchettoApplicativo->messaggio));
				
		/*inserisciTesto(stringaImmessa,sizeof(stringaImmessa));
		printf("Stai scrivendo %s\n",stringaImmessa);
		strcat(stringaImmessa,"\n");
		if(strcmp(stringaImmessa,"commit\n")!=0)
			fwrite(stringaImmessa,1,strlen(stringaImmessa),fileDiScritturaMomentanea);*/	
		
	}
	printf("Esce dal while di ricezione dei messaggi utente \n");
	if(strcmp(stringaImmessa,"commit\n")==0){
// 		richiama il metodo con l'algoritmo di agrawala
// dopo l'ack rendeil fileDiScritturaMomentanea quello fisso
		
		if(remove(nomeFileDaSostituireConPercorso)<0){
			perror("Errore di cancellazione del file originale da sostituire");
			exit(-1);
		}
		
		if(rename(IDgenerato,nomeFileDaSostituireConPercorso)<0){
			perror("Errore nella rinomina del file\n");
			exit(-1);
		}
		printf("Operazione di scrittura terminata con successo\n");
	}
	
	if(strcmp(stringaImmessa,"abort\n")==0){
		if(remove(IDgenerato)<0){
			perror("Errore nella cancellazione del file momentaneo abortito in scrittura");
			exit(-1);
		}
		printf("Operazione annullata\n");
	}
	
	fclose(fileDiScritturaMomentanea);
	fclose(fileOriginaleDaCopiare);
	return 1;
}
