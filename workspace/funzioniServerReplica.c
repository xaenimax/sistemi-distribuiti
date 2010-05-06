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
	
	printf("  %d [%s]Creaaazione dei percorsi file \n",getpid(),pacchettoApplicativo->tipoOperazione);
	
	unsigned long int dimensioneFile=0,numeroByteLetti=0;
	int numeroDiPartiDaLeggere;
	char buffer[500], stringaImmessa[100];
	char percorsoOrigine[100], percorsoDestinazione[100];
	FILE *fileDiScritturaMomentanea=fopen(percorsoDestinazione,"a");
	FILE *fileOriginaleDaCopiare=fopen(percorsoOrigine,"rb");
	
	strcpy(percorsoOrigine,"./fileCondivisi/");
	strcpy(percorsoDestinazione, "fileCondivisi/");
	printf("fa la strcat\n");
	strcat(IDgenerato,".marina");
	strcat(percorsoDestinazione,IDgenerato);
	printf("%s \n",percorsoDestinazione);
	strcat(percorsoOrigine,nomeFileDaSostituireConPercorso);
	printf("%s \n",percorsoOrigine);
			
	printf("  %d [%s] Apro il primo file temporaneo %s\n", getpid(),pacchettoApplicativo->tipoOperazione,percorsoDestinazione);
	// apro i file con relativi controlli di errore
	if ((fileDiScritturaMomentanea<0)||(fileDiScritturaMomentanea==NULL)){
		printf("  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo->tipoOperazione, percorsoDestinazione);
		bzero(pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		strcpy(pacchettoApplicativo->tipoOperazione, "scrivi file");
		strcpy(pacchettoApplicativo->messaggio, "File non trovato\n");
						
		sendPacchetto(destinazione, pacchettoApplicativo);
						
		bzero(pacchettoApplicativo, sizeof(pacchettoApplicativo));
		receivePacchetto(destinazione, pacchettoApplicativo, sizeof(pacchettoApplicativo));
		perror("Errore nella creazione del file temporaneo");
		exit(-1);
	}
	
	printf("Apro il secondo file da copiare\n");
	
	if(fileOriginaleDaCopiare<0){
		printf("  %d [%s] Errore nell'apertura del file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,pacchettoApplicativo->nomeFile);
		perror("");
		exit(-1);
	}
	
	if(fileOriginaleDaCopiare==NULL){
		printf("  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo->tipoOperazione, pacchettoApplicativo->nomeFile);
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		strcpy(pacchettoApplicativo->tipoOperazione, "scrivi file");
		strcpy(pacchettoApplicativo->messaggio, "File non trovato\n");
						
		sendPacchetto(destinazione, pacchettoApplicativo);
						
		bzero(pacchettoApplicativo, sizeof(pacchettoApplicativo));
		receivePacchetto(destinazione, pacchettoApplicativo, sizeof(pacchettoApplicativo));
	}
					
	//Se trovo il file lo spedisco al client.
	else {
		printf("  %d:[%s] File \'%s\' trovato!\n",getpid(), pacchettoApplicativo->tipoOperazione, nomeFileDaSostituireConPercorso);
		}
	

	fseek(fileOriginaleDaCopiare,0L,SEEK_END);
	dimensioneFile= ftell(fileOriginaleDaCopiare);
	fseek(fileOriginaleDaCopiare,0L,SEEK_SET);
	
	printf("  %d [%s] copia in corso\n",getpid(),pacchettoApplicativo->tipoOperazione);
	
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
	
	printf("  %d [%s]Copia completata\n",getpid(),pacchettoApplicativo->tipoOperazione);
	
// prende le cose scritte dall'utente e le aggiunge al file temporaneo
	//svuoto il buffer di invio
	bzero(pacchettoApplicativo, sizeof(pacchettoApplicativo));
	strcpy(pacchettoApplicativo->idTransazione,percorsoDestinazione);
	strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
	strcpy(pacchettoApplicativo->messaggio,"inserisci le modifiche, scrivi commit per effettuarle, abort per annullare\n");
	
	while((strcmp(pacchettoApplicativo->messaggio,"commit\n")!=0)&&(strcmp(pacchettoApplicativo->messaggio,"abort\n")!=0))
	{
		
		sendPacchetto(destinazione,pacchettoApplicativo);
		receivePacchetto(destinazione, pacchettoApplicativo, sizeof(struct pacchetto));
				
		/*inserisciTesto(stringaImmessa,sizeof(stringaImmessa));
		printf("Stai scrivendo %s\n",stringaImmessa);
		strcat(stringaImmessa,"\n");
		if(strcmp(stringaImmessa,"commit\n")!=0)
			fwrite(stringaImmessa,1,strlen(stringaImmessa),fileDiScritturaMomentanea);*/	
		

		
		if(strcmp(pacchettoApplicativo->messaggio,"commit\n")==0)
		{
// 		richiama il metodo con l'algoritmo di agrawala
// dopo l'ack rendeil fileDiScritturaMomentanea quello fisso
		
			if(remove(nomeFileDaSostituireConPercorso)<0)
			{
				printf("  %d [%s]Errore di cancellazione del file originale da sostituire\n",getpid(),pacchettoApplicativo->tipoOperazione);
				perror("");
				exit(-1);
			}
		
			if(rename(IDgenerato,nomeFileDaSostituireConPercorso)<0)
			{
				printf("  %d [%s]Errore nella rinomina del file\n",getpid(),pacchettoApplicativo->tipoOperazione);
				perror("");
				exit(-1);
			}
		printf("  %d [%s]Operazione di scrittura terminata con successo\n",getpid(),pacchettoApplicativo->tipoOperazione);
		}
		
		else if(strcmp(pacchettoApplicativo->messaggio,"abort\n")==0)
		{
			if(remove(IDgenerato)<0){
				printf("  %d [%s]Errore nella cancellazione del file momentaneo abortito in scrittura",getpid(),pacchettoApplicativo->tipoOperazione);
				perror("");
				exit(-1);
			}
			printf("  %d [%s]Operazione annullata\n",getpid(),pacchettoApplicativo->tipoOperazione);
		}
		
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
		strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
		strcpy(pacchettoApplicativo->messaggio,"inserisci le modifiche, scrivi commit per effettuarle, abort per annullare\n");
	}
	fclose(fileDiScritturaMomentanea);
	fclose(fileOriginaleDaCopiare);
	return 1;
}