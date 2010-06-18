#include "general.h"
// #include "funzioniServerReplica.h"
#include "funzioniGeneriche.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//Effettua la "dir" nella cartella del filesystem distribuito e la invia al client connesso al socket

void chiediTuttiGliIpAlDNS(struct sockaddr_in *arrayDoveSalvareIndirizziDeiServer, char *indirizzoDNSinStringa, int portaDNS, int idNumericoServerCheFaLaRichiesta);
void leggiFileDiConfigurazione(int *idServer, int *portaServer, int *portaDNS, char *percorsoFileCondivisi, char* indirizzoDNS);

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

int richiestaScritturaFile(char *IDgenerato, struct pacchetto *pacchettoApplicativo,int *socketConnesso, int idSegmentoMemCond, int idServer, char *directoryDeiFile){
	
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
	strcpy(percorsoOrigine,directoryDeiFile);
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
		return 0;
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
		return(0);
	}
	
	if(fileOriginaleDaCopiare==NULL){
		printf("  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo->tipoOperazione, pacchettoApplicativo->nomeFile);
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione, "scrivi file");
		strcpy(pacchettoApplicativo->messaggio, "File non trovato\n");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
		printf("File non trovato\n");
		return 0;
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
		printf("  %d [%s] In attesa di ricezione dal client..\n",getpid(),pacchettoApplicativo->tipoOperazione);
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		receivePacchetto(socketConnesso, pacchettoApplicativo,sizeof(struct pacchetto));
		if(errno==11){
			printf("  %d: [%s] Il client non risponde da 30 secondi. Operazione annullata\n",getpid(),pacchettoApplicativo->tipoOperazione);
	//		closeSocket(socketConnesso); per ora lo lascio commentato e vedo che fa
			remove(nomeFileDaSostituire);
			free(nomeDaSostituire);
			free(nomeFileDaSostituire);
			free(buffer);
			free(stringaImmessa);
			free(percorsoDestinazione);
			free(nomeFileTemporaneo);
			free(percorsoOrigine);
			//operazione terminata con errore
			return 0;
		}
		if((strcmp(pacchettoApplicativo->idTransazione,IDgenerato)==0)&&(strcmp(pacchettoApplicativo->tipoOperazione,"scrivi file")==0)){
			
			printf("  %d [%s] Messaggio ricevuto: %s\n",getpid(),pacchettoApplicativo->tipoOperazione,pacchettoApplicativo->messaggio);

			if(strcmp(pacchettoApplicativo->messaggio,"commit")==0) {
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
					//Per evitare di sovraccaricare la cpu
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
					return(0);
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
		else{
			//aggiungere un contatore in modo che al 3° tentativo chiuda la connessione
			bzero(pacchettoApplicativo,sizeof(struct pacchetto));
			strcpy(pacchettoApplicativo->messaggio,"Inserire nuovamente i dati");
			strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
			strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
			printf("  %d [%s]Errore dei dati in ricezione\n",getpid(),pacchettoApplicativo->tipoOperazione);
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
	free(nomeDaSostituire);
	free(nomeFileDaSostituire);
	free(buffer);
	free(stringaImmessa);
	free(percorsoDestinazione);
	free(nomeFileTemporaneo);
	free(percorsoOrigine);
	//operazione completata correttamente
	return (1);
}

//Spedisce il file temporaneo con gli aggiornamenti agli altri server
int spedisciAggiornamentiAiServer(FILE* fileConAggiornamenti, char* nomeFileDaAggiornare, char* idTransazione, int idServer) {
	
	int socketPerAggiornamenti, i, portaDNS, idServerPerFileConfNonUsato, portaServerNonUsato;
	char stringaIndirizzoDNS[20], percorsoFileNonUsato[100]; //Le variabili con il tag nonusato sono state create in quanto la funzione leggiconfigurazione accetta in ingresso anche queste variabili che devono essere settate a un qualche valore. Se non le passo, ho un comportamento anomalo del server
	struct sockaddr_in indirizzoServer[NUMERODISERVERREPLICA];
	struct pacchetto pacchettoApplicativo;
	//mi porto all'inizio del file
	fseek(fileConAggiornamenti, 0L, SEEK_SET);
	printf("IDSERVER : %d", idServer);
	
	leggiFileDiConfigurazione(&idServerPerFileConfNonUsato, &portaServerNonUsato, &portaDNS, percorsoFileNonUsato, stringaIndirizzoDNS);
	
	chiediTuttiGliIpAlDNS(indirizzoServer, stringaIndirizzoDNS, portaDNS, idServer);
	
	for(i = 0; i < NUMERODISERVERREPLICA; i++) {
		
		//Se è = 0 vuol dire che in questa posizione ci dovrebbe essere il mio indirizzo. Non devo contattare me stesso per fare agrawala
		if(indirizzoServer[i].sin_port==0) {
			//Se sono l'ultimo server non devo spedire l'aggiornamento a nessuno
			if(i != NUMERODISERVERREPLICA-1)
				i++;
			else
				break;
		}
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
	
	printf("   %d: Chiedo gli IP degli altri server al DNS %s:%d\n", getpid(), indirizzoDNSinStringa, portaDNS);
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


//Legge il file di configurazione e salva i parametri dentro le variabili passate
void leggiFileDiConfigurazione(int *idServer, int *portaServer, int *portaDNS, char *percorsoFileCondivisi, char* indirizzoDNS) {

	FILE *fileDiConfigurazione;
	int numeroDiParametriDaLeggere = 5, i;
	char *contenutoFileTok, contenutoFilediConfigurazione[500], rigaFileDiconfigurazione[numeroDiParametriDaLeggere][100]; //contenutoFilediConfigurazione contiene tutto il file, numeroDiParametriDaLeggere sono le righe del file di configurazione da leggere
	
	fileDiConfigurazione = fopen(percorsoFileDiConfigurazione, "r");
	
	if(fileDiConfigurazione == NULL) {
		printf("%d: Il file di configurazione \'%s\' non esiste. Impossibile avviare il server\n", getpid(), percorsoFileDiConfigurazione);
		exit(-1);
	}
	
	fread(contenutoFilediConfigurazione, 1, sizeof(contenutoFilediConfigurazione), fileDiConfigurazione);
	
	contenutoFileTok = strtok(contenutoFilediConfigurazione, "\n");
	strcpy(rigaFileDiconfigurazione[0], contenutoFileTok);
	
	//Salvo ogni riga del file dentro un array
	for(i = 1;contenutoFileTok != NULL && i < numeroDiParametriDaLeggere; i++) {
		contenutoFileTok = strtok(NULL, "\n");
		strcpy(rigaFileDiconfigurazione[i],contenutoFileTok);
// 		printf("Tok: %s\n", contenutoFileTok);
	}
	
	//Metto i parametri di ogni riga del file di configurazione dentro le varie variabili
	for(i = 0; i < numeroDiParametriDaLeggere; i++) {
		//Legge la stringa, non mi serve
		contenutoFileTok = strtok(rigaFileDiconfigurazione[i], ":");

		//legge il valore, me serve :D
		contenutoFileTok = strtok(NULL, ":");

		switch(i) {
			
			case 0:
				*idServer = atoi(contenutoFileTok);
				break;
				
			case 1:
				*portaServer = atoi(contenutoFileTok);
				break;
				
			case 2:
				strcpy(percorsoFileCondivisi, contenutoFileTok);
				break;
				
			case 3:
				strcpy(indirizzoDNS, contenutoFileTok);
				break;
				
			case 4:
				*portaDNS = atoi(contenutoFileTok);
				break;
		}
	}
	
// 	printf("%d, %d, %s, %s %d %d", idServer, porta, percorsoFileCondivisi, indirizzoDNS, portaDNS);
}


int sincronizzazioneFile(char *directoryDeiFile){
	char riferimento_server[600],indirizzoIpDelServer[100];
	int connessioneSincr,i,portaDelServer;
	struct timeval tempoDiAttesa;
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	
	tempoDiAttesa.tv_sec=30;
	errno=111;
	//contatta il primo Server disponibile
	for(i = 1; errno == 111 && i <= NUMERODISERVERREPLICA; i++) {
		int idServer;
		errno = 0; //per evitare che mi chiuda il socket
		createSocketStream(&connessioneSincr);
		setsockopt(connessioneSincr, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tempoDiAttesa, sizeof(struct timeval));
		contattaDNS(riferimento_server);
		separaIpEportaDaStringa(riferimento_server, indirizzoIpDelServer, &portaDelServer, &idServer);
		assegnaIPaServaddr(indirizzoIpDelServer, portaDelServer, &servaddr);
		printf("Provo a connettermi al server %d con ip ", idServer);
		stampaIpEporta(&servaddr);
		printf("\n");
		connectSocket(&connessioneSincr, &servaddr);
		//chiudo il socket solo se non riesco a connettermi
		if(errno == 111)
			closeSocket(&connessioneSincr);
	}
	if(i == 4 && errno == 111) {
		printf("Non risulta nessun server attivo! :(\n");
		//exit(0);
		return (-1);
	}
	bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
	//chiede la lista dei file da copiare
	strcpy(pacchettoApplicativo.tipoOperazione,"lista file");
	sendPacchetto(&connessioneSincr,&pacchettoApplicativo);
	bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
	receivePacchetto(&connessioneSincr,&pacchettoApplicativo,sizeof(struct pacchetto));
	printf("La lista file ricevuta %s",pacchettoApplicativo.messaggio);
	
	char *nomeFile=strtok(pacchettoApplicativo.messaggio,"\n");
	generaIDtransazione(pacchettoApplicativo.idTransazione);
	
	bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
	strcpy(pacchettoApplicativo.tipoOperazione, "copia file, pronto a ricevere");
	strcpy(pacchettoApplicativo.nomeFile, nomeFile);
	
	//dico al client che sono pronto a ricevere
	sendPacchetto(&connessioneSincr, &pacchettoApplicativo);
	
	bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
	receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
	
	char nomeFileDaScrivereConPercorso[sizeof(directoryDeiFile) + sizeof(pacchettoApplicativo.nomeFile)];
	strcpy(nomeFileDaScrivereConPercorso, directoryDeiFile);
	strcat(nomeFileDaScrivereConPercorso, pacchettoApplicativo.nomeFile);

	riceviFile(&connessioneSincr, nomeFileDaScrivereConPercorso, &pacchettoApplicativo);
							
	bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
	receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(pacchettoApplicativo));		

	while(nomeFile!=NULL){
		nomeFile=strtok(pacchettoApplicativo.messaggio,NULL);
		strcpy(nomeFile, pacchettoApplicativo.nomeFile);
		generaIDtransazione(pacchettoApplicativo.idTransazione);
		
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		strcpy(pacchettoApplicativo.tipoOperazione, "copia file, pronto a ricevere");
		strcpy(pacchettoApplicativo.nomeFile, nomeFile);
		
		//dico al client che sono pronto a ricevere
		sendPacchetto(&connessioneSincr, &pacchettoApplicativo);
		
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		char nomeFileDaScrivereConPercorso[sizeof(directoryDeiFile) + sizeof(pacchettoApplicativo.nomeFile)];
		strcpy(nomeFileDaScrivereConPercorso, directoryDeiFile);
		strcat(nomeFileDaScrivereConPercorso, pacchettoApplicativo.nomeFile);
	
		riceviFile(&connessioneSincr, nomeFileDaScrivereConPercorso, &pacchettoApplicativo);
								
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(pacchettoApplicativo));				

		
		
		
	}
	//controlla le operazioni di copia file deve essere contrario rispetto a client server
	
	
	
	return 1;
}