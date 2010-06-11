#include "general.h"
// #include "funzioniServerReplica.h"
#include "funzioniGeneriche.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//Effettua la "dir" nella cartella del filesystem distribuito e la invia al client connesso al socket

void chiediTuttiGliIpAlDNS(struct sockaddr_in *arrayDoveSalvareIndirizziDeiServer, char *indirizzoDNSinStringa, int portaDNS, int idNumericoServerCheFaLaRichiesta);

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
		/* If no files found, make a non-selectable menu item5AA091C9A091ABCF */
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

int richiestaScritturaFile(char *IDgenerato, struct pacchetto *pacchettoApplicativo,int *socketConnesso, int idSegmentoMemCond, int idServer){
	
	printf("  %d [%s]Creazione dei percorsi file \n",getpid(),pacchettoApplicativo->tipoOperazione);
	
	unsigned long int dimensioneFile=0,numeroByteLetti=0;
	int numeroDiPartiDaLeggere;
	char *buffer, *stringaImmessa, *percorsoOrigine, *percorsoDestinazione,*nomeFileTemporaneo,*nomeDaSostituire, *nomeFileDaSostituire;
	FILE *fileOriginaleDaCopiare, *fileDiScritturaMomentanea;
	
	nomeDaSostituire=malloc(100*sizeof(char));
	nomeFileDaSostituire=malloc(100*sizeof(char));
	buffer=malloc(500*sizeof(char));
	stringaImmessa=malloc(100*sizeof(char));
	percorsoDestinazione=malloc(100*sizeof(char));
	nomeFileTemporaneo=malloc(100*sizeof(char));
	percorsoOrigine=malloc(100*sizeof(char));
	
	strcpy(nomeFileDaSostituire, pacchettoApplicativo->nomeFile);
	
	//preparazione dei percorsi dei file da utilizzare
	strcpy(nomeDaSostituire,nomeFileDaSostituire);
	strcpy(percorsoDestinazione, "/tmp/");
	strcpy(nomeFileTemporaneo,IDgenerato);
	strcat(nomeFileTemporaneo,".marina");
	strcat(percorsoDestinazione,nomeFileTemporaneo);
	strcpy(percorsoOrigine,"fileCondivisi/");
	strcat(percorsoOrigine,nomeFileDaSostituire);
	
	printf("  %d: [%s] Apro il primo file temporaneo %s\n", getpid(),pacchettoApplicativo->tipoOperazione,percorsoDestinazione);
	fileDiScritturaMomentanea=fopen(percorsoDestinazione,"a");
	// apro i file con relativi controlli di errore
	if ((fileDiScritturaMomentanea<0)){
		printf("  %d [%s] Errore nell'apertura del file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,percorsoDestinazione);
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione,"chiudi connessione");
		strcpy(pacchettoApplicativo->messaggio,"Errore nella creazione del file temporaneo");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
		perror("Errore nella creazione del file temporaneo");
		return -1;
	}
	
	printf("  %d [%s] Apro il secondo file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,percorsoOrigine);
	fileOriginaleDaCopiare = fopen(percorsoOrigine,"a");		
	
	if(fileOriginaleDaCopiare<0){
		printf("  %d [%s] Errore nell'apertura del file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,pacchettoApplicativo->nomeFile);
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione,"chiudi connessione");
		strcpy(pacchettoApplicativo->messaggio,"Errore nell'apertura del file originale");
		sendPacchetto(socketConnesso,pacchettoApplicativo);
		perror("Errore nell'apertura del file originale");
		return(-1);
	}
	
	if(fileOriginaleDaCopiare==NULL){
		printf("  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo->tipoOperazione, pacchettoApplicativo->nomeFile);
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione, "scrivi file");
		strcpy(pacchettoApplicativo->messaggio, "File non trovato\n");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
		printf("File non trovato\n");
		return -1;
	}
					
	//Se trovo il file lo spedisco al client.
	else {
		printf("  %d:[%s] File \'%s\' trovato!\n",getpid(), pacchettoApplicativo->tipoOperazione, nomeFileDaSostituire);
	}
/*
	if(copiaFile(fileOriginaleDaCopiare, fileDiScritturaMomentanea, NULL, NULL, 0) > 0)
		printf("  %d [%s]Copia completata\n",getpid(),pacchettoApplicativo->tipoOperazione);
	else
		printf("  %d [%s] Errore nella copia del file!", getpid(), pacchettoApplicativo->tipoOperazione);*/
	
// prende le cose scritte dall'utente e le aggiunge al file temporaneo
	//svuoto il buffer di invio
	bzero(pacchettoApplicativo, sizeof(struct pacchetto));
	strncpy(pacchettoApplicativo->idTransazione,IDgenerato,strlen(IDgenerato));
	strncpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto",strlen("scrivi file, pronto"));
	strncpy(pacchettoApplicativo->messaggio,"Inserisci le modifiche da effettuare, scrivere commit per terminare, abort per annullare",strlen("Inserisci le modifiche da effettuare, scrivere commit per terminare, abort per annullare"));
	
	while((strcmp(pacchettoApplicativo->messaggio,"commit")!=0)&&(strcmp(pacchettoApplicativo->messaggio,"abort")!=0))
	{
		printf("  %d [%s] Invio messaggio al client\n",getpid(),pacchettoApplicativo->tipoOperazione);
		
		sendPacchetto(socketConnesso,pacchettoApplicativo);
		printf("  %d [%s] In attesa di ricezione dal client\n",getpid(),pacchettoApplicativo->tipoOperazione);
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		receivePacchetto(socketConnesso, pacchettoApplicativo,sizeof(struct pacchetto));
		printf("  %d [%s] Messaggio ricevuto: %s\n",getpid(),pacchettoApplicativo->tipoOperazione,pacchettoApplicativo->messaggio);

		if(strcmp(pacchettoApplicativo->messaggio,"commit")==0)
		{
			// 		richiama il metodo con l'algoritmo di agrawala
			struct fileApertiDalServer *listaFile;
			char percorsoFileFifo[50], contenutoFileFifo[100];
			int descrittoreFileFifo;
			listaFile = malloc(15*sizeof(struct fileApertiDalServer));
			
			svuotaStrutturaListaFile(listaFile);

			listaFile = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);
			
			printf("  %d:[%s] Commit in esecuzione! File: \'%s\'\n", getpid(), pacchettoApplicativo->tipoOperazione, nomeFileDaSostituire);
			int i;
			//cerco nell'array dei file, la prima posizione vuota e vado ad inserire il mio file
			for(i = 0; i < 10 && strlen(listaFile[i].nomeFile) != 0; i++) {
// 				printf("  %d:[%s, DEBUG] Cerco una posizione vuota dove inserire il mio file.\n", getpid(), pacchettoApplicativo->tipoOperazione);
				if(i == 9)
					i = -1;
			}
			
// 			printf("  %d:[%s, DEBUG] Posizione vuota: %d\n", getpid(), pacchettoApplicativo->tipoOperazione, i);
			//Scrivendo in listaFile, che è la memoria dinamica condivisa, avviso il figlio di agrawala che dovrà cominciare a fare agrawala
			strcpy(listaFile[i].nomeFile, nomeFileDaSostituire);
			strcpy(listaFile[i].idTransazione, IDgenerato);
			
			//----------Ora comincio a vedere se il figlio di agrawala ha finito di ricevere le conferme
			strcpy(percorsoFileFifo, "/tmp/");
			strcat(percorsoFileFifo, IDgenerato);
			
			while(strcmp(contenutoFileFifo, "Ok") != 0) {
				sleep(1);
				descrittoreFileFifo = open(percorsoFileFifo, O_RDONLY);
				read(descrittoreFileFifo, contenutoFileFifo, sizeof(contenutoFileFifo));
				close(descrittoreFileFifo);
			}
			//----------------
			
			printf("  %d: Agrawala ha detto si'. :D Avviso il client che il commit è andato a buon fine\n", getpid());
			
			bzero(pacchettoApplicativo, sizeof(struct pacchetto));
			strcpy(pacchettoApplicativo->tipoOperazione, "commit eseguito");
			strcpy(pacchettoApplicativo->messaggio, "Commit eseguito con successo!");
			sendPacchetto(socketConnesso, pacchettoApplicativo);
			
			printf("  %d: Spedita la conferma al client.\n", getpid());
			
			strcpy(nomeFileTemporaneo,IDgenerato);
			strcat(nomeFileTemporaneo,".marina");

			//Lo chiudo perchè dato che ci stavo scrivendo, il puntatore al file è alla fine e non copierebbe niente nel file originale
			fclose(fileDiScritturaMomentanea);
			fopen(percorsoDestinazione, "r");
			copiaFile(fileDiScritturaMomentanea, fileOriginaleDaCopiare, NULL, NULL, 0);
			
			spedisciAggiornamentiAiServer(fileDiScritturaMomentanea, nomeDaSostituire,IDgenerato,idServer);
			
			if(copiaFile > 0)
				remove(percorsoDestinazione);
			
			bzero(pacchettoApplicativo, sizeof(struct pacchetto));
			strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
			strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
			strcpy(pacchettoApplicativo->messaggio,"commit");
			printf("  %d [%s] Operazione di scrittura terminata con successo\n",getpid(),pacchettoApplicativo->tipoOperazione);
		}
		
		else if(strcmp(pacchettoApplicativo->messaggio,"abort")==0)
		{
			if(remove(percorsoDestinazione)<0){
				printf("  %d [%s]Errore nella cancellazione del file momentaneo abortito in scrittura",getpid(),pacchettoApplicativo->tipoOperazione);
				perror("");
				return(-1);
			}
			printf("  %d [%s]Operazione annullata\n",getpid(),pacchettoApplicativo->tipoOperazione);
					
			bzero(pacchettoApplicativo, sizeof(struct pacchetto));
			strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
			strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
			strcpy(pacchettoApplicativo->messaggio,"abort");
		}
		else{
			strcat(pacchettoApplicativo->messaggio,"\n");
			fwrite(pacchettoApplicativo->messaggio,1, strlen(pacchettoApplicativo->messaggio),fileDiScritturaMomentanea); 
			bzero(pacchettoApplicativo, sizeof(struct pacchetto));
			strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
			strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
		}
	}
	printf("chiudo i file in uso\n");
	if(fclose(fileDiScritturaMomentanea)<0){
		printf("%d [%s]Errore nella chiusura del file temporaneo\n",getpid(),pacchettoApplicativo->tipoOperazione);
		perror("");
	}
	if(fclose(fileOriginaleDaCopiare)<0){
		printf("%d [%s]Errore nella chiusura del file da copiare\n",getpid(),pacchettoApplicativo->tipoOperazione);
		perror("");
	}
	
	//ci vuole il free di tutte le malloc
	return (0);
}

//Spedisce il file temporaneo con gli aggiornamenti agli altri server
int spedisciAggiornamentiAiServer(FILE* fileConAggiornamenti, char* nomeFileDaAggiornare, char* idTransazione, int idServer) {
	int socketPerAggiornamenti, i;
	struct sockaddr_in indirizzoServer[NUMERODISERVERREPLICA];
	struct pacchetto pacchettoApplicativo;
	//mi porto all'inizio del file
	fseek(fileConAggiornamenti, 0L, SEEK_SET);
	printf("IDSERVER : %d", idServer);
	chiediTuttiGliIpAlDNS(indirizzoServer, stringaIndirizzoDNS, PORTADNS, idServer);
	
	for(i = 0; i < NUMERODISERVERREPLICA; i++) {
		
		//Se è = 0 vuol dire che in questa posizione ci dovrebbe essere il mio indirizzo. Non devo contattare me stesso per fare agrawala
		if(indirizzoServer[i].sin_port==0)
			i++;
		createSocketStream(&socketPerAggiornamenti);
// 		printf("   %d: Sto per connettermi all'ip: ", getpid());
		connectSocket(&socketPerAggiornamenti, &indirizzoServer[i]);
		
		if(errno==111){
			printf("  %d: Non riesco a mandare l'aggiornamento al server",getpid());
			stampaIpEporta(&indirizzoServer[i]);
			printf("\n");
			
			closeSocket(&socketPerAggiornamenti);
		}
		else{
		
			bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
			strcpy(pacchettoApplicativo.tipoOperazione, "aggiorna file");
			strcpy(pacchettoApplicativo.nomeFile, nomeFileDaAggiornare);
			strcpy(pacchettoApplicativo.idTransazione, idTransazione);
			sendPacchetto(&socketPerAggiornamenti, &pacchettoApplicativo);
			bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
			receivePacchetto(&socketPerAggiornamenti, &pacchettoApplicativo, sizeof(struct pacchetto));
			
			if(strcmp(pacchettoApplicativo.tipoOperazione, "aggiorna file, pronto a ricevere") == 0) {
			
			spedisciFile(&socketPerAggiornamenti, fileConAggiornamenti, &pacchettoApplicativo);
			printf("  %d: File spedito con successo al server ", getpid());
			stampaIpEporta(&indirizzoServer[i]);
			printf("\n");
			}
			else
				printf("  %d: C\'è stato un\'errore durante l\'invio dell\'aggiornamento al server\n", getpid());
			
			closeSocket(&socketPerAggiornamenti);
		}
	}
}


//Chiede al DNS tutti gli IP e restituisce un sockaddr_in in cui salva tutti gli indirizzi ESCLUSO quello dell'idNumericoServer che fa la richiesta
void chiediTuttiGliIpAlDNS(struct sockaddr_in *arrayDoveSalvareIndirizziDeiServer, char *indirizzoDNSinStringa, int portaDNS, int idNumericoServerCheFaLaRichiesta) {
		char **IPDaAssegnare, stringaIndirizzoIP[19];
		int socketPerRichiestaLista, i, portaDaAssegnare, idServer;
		struct pacchetto pacchettoApplicativo;
		struct sockaddr_in indirizzoDNS;
		
		IPDaAssegnare = malloc(NUMERODISERVERREPLICA*19*sizeof(char));
		for(i = 0; i < NUMERODISERVERREPLICA; i++) //ip:porta
			IPDaAssegnare[i] = malloc(19*sizeof(char));
		
		for(i=0;i<NUMERODISERVERREPLICA;i++){
			bzero(&arrayDoveSalvareIndirizziDeiServer[i], sizeof(struct sockaddr_in));
		}
		
		printf("   %d: Chiedo gli IP degli altri server al DNS\n", getpid());
		createSocketStream(&socketPerRichiestaLista);
		bzero(&indirizzoDNS,sizeof(struct sockaddr_in));
		assegnaIPaServaddr(indirizzoDNSinStringa,portaDNS,&indirizzoDNS);
		connectSocket(&socketPerRichiestaLista,&indirizzoDNS);
		
		bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo.tipoOperazione,"indirizzi server");
		sendPacchetto(&socketPerRichiestaLista,&pacchettoApplicativo);
		
		bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
		receivePacchetto(&socketPerRichiestaLista,&pacchettoApplicativo,sizeof(struct pacchetto));
		closeSocket(&socketPerRichiestaLista);
		
		printf("   %d: IP ricevuti!\n", getpid());
		
		//Prendo il pacchetto ricevuto e mi salvo i tre indirizzi IP in tre char di indirizzi e 3 array di porte. Sono costretto a fare un for separato solo per questo perchè altrimento la strtok non funziona.
		for(i=0;i<NUMERODISERVERREPLICA;i++){
			char *indirizzotok; //se non funziona sposta questa fuori
			if(i == 0)
				indirizzotok=strtok(pacchettoApplicativo.messaggio,"\n");
			else 
				indirizzotok=strtok(NULL,"\n");
			
// 			printf("   %d: Faccio la token di %s\n",getpid(), indirizzotok);
			strcpy(IPDaAssegnare[i], indirizzotok);
		}
		
		for(i=0;i<NUMERODISERVERREPLICA;i++){
			separaIpEportaDaStringa(IPDaAssegnare[i],stringaIndirizzoIP,&portaDaAssegnare,&idServer);
			portaDaAssegnare = portaDaAssegnare + 1000; //+ 1000 perchè devo contattare il server sulla porta di servizio e non quella normale
// 			printf("   %d: IP \'%s\', porta: %d id server: %d\n", getpid(), stringaIndirizzoIP, portaDaAssegnare, idServer);
			
			//se è diverso assegno l'ip alla servaddr. SE è uguale allora è il mio ip e non mi interessa contattare me stesso
			if(idServer != idNumericoServerCheFaLaRichiesta) {
				assegnaIPaServaddr(stringaIndirizzoIP,portaDaAssegnare,&arrayDoveSalvareIndirizziDeiServer[i]);
			}
		}
	
	
}