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

void writeFileWithLock(int descrittoreFile, char *contenutoDaScrivere, int stampaAvideo, int aggiungiData);

void stampaIpEportaConLog(struct sockaddr_in *indirizzoIP);

void inviaListaFile(int *socketConnesso, char *directoryDeiFile) {
	int numeroDiFileTrovati = 0, descrittoreLogFileServer;
	int i;
	struct direct **fileTrovati;
	struct pacchetto pacchettoDaInviare;
	char stringaDiFileTrovati[1024];
	char stringaDaStampare[500];
	
	//svuoto il buffer di invio
	bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	sprintf(stringaDaStampare, "  %d: Invio la lista file, come richiesto.\n", getpid());
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);

	numeroDiFileTrovati = scandir(directoryDeiFile, &fileTrovati, NULL, alphasort);
		/* If no files found, make a non-selectable menu item5AA091C9A091ABCF */
	if 		(numeroDiFileTrovati <= 0) {
		strcpy(pacchettoDaInviare.tipoOperazione, "lista file");
		strcpy(pacchettoDaInviare.messaggio, "Nessun file trovato!");
		sprintf(stringaDaStampare, "  %d: Nessun file trovato!\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	}
	else {
		for (i=1;i<numeroDiFileTrovati+1;++i) {
				sprintf(stringaDiFileTrovati, "%s\r\n", fileTrovati[i-1]->d_name);
				strcat(pacchettoDaInviare.messaggio, stringaDiFileTrovati);
		}
	}
	
	sendPacchetto(socketConnesso, &pacchettoDaInviare, sizeof(pacchettoDaInviare), 0);
	close(descrittoreLogFileServer);
}

int richiestaScritturaFile(char *IDgenerato, struct pacchetto *pacchettoApplicativo,int *socketConnesso, int idSegmentoMemCond, int idServer, char *directoryDeiFile) {
	
	int descrittoreLogFileServer;
	char stringaDaStampare[500];
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	sprintf(stringaDaStampare, "  %d [%s]Creazione dei percorsi file \n",getpid(),pacchettoApplicativo->tipoOperazione);
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		
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
	
	sprintf(stringaDaStampare, "  %d: [%s] Apro il primo file temporaneo %s\n", getpid(),pacchettoApplicativo->tipoOperazione,percorsoDestinazione);
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	
	//Creo il file richiesto dall'utente
	fileDiScritturaMomentanea=fopen(percorsoDestinazione,"a");
	// apro i file con relativi controlli di errore
	if ((fileDiScritturaMomentanea<0)){
		sprintf(stringaDaStampare, "  %d [%s] Errore nell'apertura del file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,percorsoDestinazione);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione,"chiudi connessione");
		strcpy(pacchettoApplicativo->messaggio,"Errore nella creazione del file temporaneo");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
		perror("Errore nella creazione del file temporaneo");
		close(descrittoreLogFileServer);
		return 0;
	}
	
	sprintf(stringaDaStampare, "  %d [%s] Apro il secondo file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,percorsoOrigine);
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	fileOriginaleDaCopiare = fopen(percorsoOrigine,"a");		
	
	if(fileOriginaleDaCopiare<0){
		sprintf(stringaDaStampare, "  %d [%s] Errore nell'apertura del file da copiare %s\n",getpid(),pacchettoApplicativo->tipoOperazione,pacchettoApplicativo->nomeFile);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione,"chiudi connessione");
		strcpy(pacchettoApplicativo->messaggio,"Errore nell'apertura del file originale");
		sendPacchetto(socketConnesso,pacchettoApplicativo);
		perror("Errore nell'apertura del file originale");
		close(descrittoreLogFileServer);
		return(0);
	}
	
	if(fileOriginaleDaCopiare==NULL){
		sprintf(stringaDaStampare, "  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo->tipoOperazione, pacchettoApplicativo->nomeFile);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		bzero(pacchettoApplicativo, sizeof(struct pacchetto));
		strcpy(pacchettoApplicativo->tipoOperazione, "scrivi file");
		strcpy(pacchettoApplicativo->messaggio, "File non trovato\n");
		sendPacchetto(socketConnesso, pacchettoApplicativo);
		close(descrittoreLogFileServer);
		return 0;
	}
	
	//Lo chiudo perché è il file originale che dovrò scrivere. Faccio queste operazioni solo per crearlo, la funzione copia file si occuperà di aggiornarne il contenuto in modalità con lock
	if(fclose(fileOriginaleDaCopiare)<0){
		sprintf(stringaDaStampare, "%d [%s]Errore nella chiusura del file da copiare\n",getpid(),pacchettoApplicativo->tipoOperazione);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		perror("");
// 		close(descrittoreLogFileServer);		
	}
	
					
	//Se trovo il file lo spedisco al client.
	else {
		sprintf(stringaDaStampare, "  %d:[%s] File \'%s\' trovato!\n",getpid(), pacchettoApplicativo->tipoOperazione, nomeFileDaSostituire);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
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
		sprintf(stringaDaStampare, "  %d [%s] Invio messaggio al client\n",getpid(),pacchettoApplicativo->tipoOperazione);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		
		sendPacchetto(socketConnesso,pacchettoApplicativo);
		sprintf(stringaDaStampare, "  %d [%s] In attesa di ricezione dal client..\n",getpid(),pacchettoApplicativo->tipoOperazione);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		bzero(pacchettoApplicativo,sizeof(struct pacchetto));
		receivePacchetto(socketConnesso, pacchettoApplicativo,sizeof(struct pacchetto));
		if(errno==11){
			sprintf(stringaDaStampare, "  %d: [%s] Il client non risponde da 30 secondi. Operazione annullata\n",getpid(),pacchettoApplicativo->tipoOperazione);
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	//		closeSocket(socketConnesso); per ora lo lascio commentato e vedo che fa
			remove(percorsoDestinazione);
			free(nomeDaSostituire);
			free(nomeFileDaSostituire);
			free(buffer);
			free(stringaImmessa);
			free(percorsoDestinazione);
			free(nomeFileTemporaneo);
			free(percorsoOrigine);
			//operazione terminata con errore
			close(descrittoreLogFileServer);
			return 0;
		}
		if((strcmp(pacchettoApplicativo->idTransazione,IDgenerato)==0)&&(strcmp(pacchettoApplicativo->tipoOperazione,"scrivi file")==0)){
			
			sprintf(stringaDaStampare, "  %d [%s] Messaggio ricevuto: %s\n",getpid(),pacchettoApplicativo->tipoOperazione,pacchettoApplicativo->messaggio);
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);

			if(strcmp(pacchettoApplicativo->messaggio,"commit")==0) {
				// 		richiama il metodo con l'algoritmo di agrawala
				struct fileApertiDalServer *listaFile;
				char percorsoFileFifo[50], contenutoFileFifo[100];
				int descrittoreFileFifo;
				listaFile = malloc(15*sizeof(struct fileApertiDalServer));
				
				svuotaStrutturaListaFile(listaFile);

				listaFile = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);
				
				sprintf(stringaDaStampare, "  %d:[%s] Commit in esecuzione! File: \'%s\'\n", getpid(), pacchettoApplicativo->tipoOperazione, nomeFileDaSostituire);
				writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
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
				
				sprintf(stringaDaStampare, "  %d: Agrawala ha detto si'. :D Avviso il client che il commit è andato a buon fine\n", getpid());
				writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
				
				strcpy(nomeFileTemporaneo,IDgenerato);
				strcat(nomeFileTemporaneo,".marina");

				//Lo chiudo perchè dato che ci stavo scrivendo, il puntatore al file è alla fine e non copierebbe niente nel file originale
				fclose(fileDiScritturaMomentanea);
				fileDiScritturaMomentanea=fopen(percorsoDestinazione, "r");
// 				copiaFile(fileDiScritturaMomentanea, fileOriginaleDaCopiare, NULL, NULL, 0);
				copiaFile(fileDiScritturaMomentanea, NULL, NULL, percorsoOrigine, 0, 1);
				
				spedisciAggiornamentiAiServer(fileDiScritturaMomentanea, nomeDaSostituire,IDgenerato,idServer);
				
				if(copiaFile > 0)
					remove(percorsoDestinazione);
				
				//spedisco la conferma al client
				bzero(pacchettoApplicativo, sizeof(struct pacchetto));
				strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
				strcpy(pacchettoApplicativo->tipoOperazione, "commit eseguito");
				strcpy(pacchettoApplicativo->messaggio, "Commit eseguito con successo!");
				sendPacchetto(socketConnesso, pacchettoApplicativo);
				sprintf(stringaDaStampare, "  %d: Spedita la conferma al client.\n", getpid());
				writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
				
				//queste righe mi servono per uscire dal ciclo while
				bzero(pacchettoApplicativo, sizeof(struct pacchetto));
				strcpy(pacchettoApplicativo->idTransazione,IDgenerato);
				strcpy(pacchettoApplicativo->tipoOperazione,"scrivi file, pronto");
				strcpy(pacchettoApplicativo->messaggio,"commit");
				//queste due righe mi servono per uscire dal ciclo while
				sprintf(stringaDaStampare, "  %d [%s] Operazione di scrittura terminata con successo\n",getpid(),pacchettoApplicativo->tipoOperazione);
				writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
			}
		
			else if(strcmp(pacchettoApplicativo->messaggio,"abort")==0)
			{
				if(remove(percorsoDestinazione)<0){
					sprintf(stringaDaStampare, "  %d [%s]Errore nella cancellazione del file momentaneo abortito in scrittura",getpid(),pacchettoApplicativo->tipoOperazione);
					writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
					perror("");
					return(0);
				}
				sprintf(stringaDaStampare, "  %d [%s]Operazione annullata\n",getpid(),pacchettoApplicativo->tipoOperazione);
				writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
						
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
			sprintf(stringaDaStampare, "  %d [%s]Errore dei dati in ricezione\n",getpid(),pacchettoApplicativo->tipoOperazione);
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		}
	}
	sprintf(stringaDaStampare, "  %d: chiudo i file in uso\n",getpid());
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	
	if(fclose(fileDiScritturaMomentanea)<0){
		sprintf(stringaDaStampare, "%d [%s]Errore nella chiusura del file temporaneo\n",getpid(),pacchettoApplicativo->tipoOperazione);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		perror("");
		close(descrittoreLogFileServer);
	}
// 	if(fclose(fileOriginaleDaCopiare)<0){
// 		sprintf(stringaDaStampare, "%d [%s]Errore nella chiusura del file da copiare\n",getpid(),pacchettoApplicativo->tipoOperazione);
// 		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
// 		perror("");
// 		close(descrittoreLogFileServer);		
// 	}
	
	//ci vuole il free di tutte le malloc
	free(nomeDaSostituire);
	free(nomeFileDaSostituire);
	free(buffer);
	free(stringaImmessa);
	free(percorsoDestinazione);
	free(nomeFileTemporaneo);
	free(percorsoOrigine);
	close(descrittoreLogFileServer);
	//operazione completata correttamente
	return (1);
}

//Spedisce il file temporaneo con gli aggiornamenti agli altri server
int spedisciAggiornamentiAiServer(FILE* fileConAggiornamenti, char* nomeFileDaAggiornare, char* idTransazione, int idServer) {
	
	int socketPerAggiornamenti, i, portaDNS, idServerPerFileConfNonUsato, portaServerNonUsato, descrittoreLogFileServer;
	char stringaIndirizzoDNS[20], percorsoFileNonUsato[100], stringaDaStampare[500]; //Le variabili con il tag nonusato sono state create in quanto la funzione leggiconfigurazione accetta in ingresso anche queste variabili che devono essere settate a un qualche valore. Se non le passo, ho un comportamento anomalo del server
	struct sockaddr_in indirizzoServer[NUMERODISERVERREPLICA];
	struct pacchetto pacchettoApplicativo;
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	//mi porto all'inizio del file
	fseek(fileConAggiornamenti, 0L, SEEK_SET);
	
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
			sprintf(stringaDaStampare, "  %d: Non riesco a mandare l'aggiornamento al server",getpid());
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
			stampaIpEportaConLog(&indirizzoServer[i]);
			sprintf(stringaDaStampare, "\n");
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 0);
			close(descrittoreLogFileServer);
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
			sprintf(stringaDaStampare, "  %d: File spedito con successo al server ", getpid());
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
			stampaIpEportaConLog(&indirizzoServer[i]);
			sprintf(stringaDaStampare, "\n");
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 0);
			}
			else {
				sprintf(stringaDaStampare, "  %d: C\'è stato un\'errore durante l\'invio dell\'aggiornamento al server\n", getpid());
				writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
			}
			
			closeSocket(&socketPerAggiornamenti);
			close(descrittoreLogFileServer);
		}
	}
}

//Chiede al DNS tutti gli IP e restituisce un sockaddr_in in cui salva tutti gli indirizzi ESCLUSO quello dell'idNumericoServer che fa la richiesta
void chiediTuttiGliIpAlDNS(struct sockaddr_in *arrayDoveSalvareIndirizziDeiServer, char *indirizzoDNSinStringa, int portaDNS, int idNumericoServerCheFaLaRichiesta) {
	char **IPDaAssegnare, stringaIndirizzoIP[19], stringaDaStampare[500];
	int socketPerRichiestaLista, i, portaDaAssegnare, idServer, descrittoreLogFileServer;
	struct pacchetto pacchettoApplicativo;
	struct sockaddr_in indirizzoDNS;
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	IPDaAssegnare = malloc(NUMERODISERVERREPLICA*19*sizeof(char));
	for(i = 0; i < NUMERODISERVERREPLICA; i++) //ip:porta
		IPDaAssegnare[i] = malloc(19*sizeof(char));
	
	for(i=0;i<NUMERODISERVERREPLICA;i++){
		bzero(&arrayDoveSalvareIndirizziDeiServer[i], sizeof(struct sockaddr_in));
	}
	
	sprintf(stringaDaStampare, "   %d: Chiedo gli IP degli altri server al DNS %s:%d\n", getpid(), indirizzoDNSinStringa, portaDNS);
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
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
	
	sprintf(stringaDaStampare, "   %d: IP ricevuti!\n", getpid());
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	
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
	
	close(descrittoreLogFileServer);
}


//Legge il file di configurazione e salva i parametri dentro le variabili passate
void leggiFileDiConfigurazione(int *idServer, int *portaServer, int *portaDNS, char *percorsoFileCondivisi, char* indirizzoDNS) {

	FILE *fileDiConfigurazione;
	int numeroDiParametriDaLeggere = 5, i, descrittoreLogFileServer;
	char *contenutoFileTok, contenutoFilediConfigurazione[500], rigaFileDiconfigurazione[numeroDiParametriDaLeggere][100], stringaDaStampare[500]; //contenutoFilediConfigurazione contiene tutto il file, numeroDiParametriDaLeggere sono le righe del file di configurazione da leggere
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	fileDiConfigurazione = fopen(percorsoFileDiConfigurazione, "r");	
	
	if(fileDiConfigurazione == NULL) {
		sprintf(stringaDaStampare, "%d: Il file di configurazione \'%s\' non esiste. Impossibile avviare il server\n", getpid(), percorsoFileDiConfigurazione);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		close(descrittoreLogFileServer);
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

//sincronizza i file del server rispetto agli altri al primo avvio della macchina.
int sincronizzazioneFile(char *directoryDeiFile){
	char riferimento_server[600],indirizzoIpDelServer[100],stringaDaStampare[500],IDTransazione[20],listaFile[500];
	char *nomeFile;
	int connessioneSincr,i,portaDelServer,descrittoreLogFileServer;
	struct timeval tempoDiAttesa;
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
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
		portaDelServer=portaDelServer+1000;
		assegnaIPaServaddr(indirizzoIpDelServer, portaDelServer, &servaddr);
		sprintf(stringaDaStampare, "  %d: Provo a connettermi al server %d con ip ",getpid(), idServer);
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		stampaIpEportaConLog(&servaddr);
		sprintf(stringaDaStampare, "\n");
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 0);
		connectSocket(&connessioneSincr, &servaddr);
		//chiudo il socket solo se non riesco a connettermi
		if(errno == 111)
			closeSocket(&connessioneSincr);
	}
	if(i == 4 && errno == 111) {
		sprintf(stringaDaStampare, "  %d: Non risulta nessun server attivo! :(\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
		close(descrittoreLogFileServer);
		//exit(0);
		return (-1);
	}
	bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
	//chiede la lista dei file da copiare
	strcpy(pacchettoApplicativo.tipoOperazione,"lista file");
	printf("  %d:[%s] Invio la richiesta di lista file\n",getpid(), pacchettoApplicativo.tipoOperazione);
	sendPacchetto(&connessioneSincr,&pacchettoApplicativo);
	bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
	receivePacchetto(&connessioneSincr,&pacchettoApplicativo,sizeof(struct pacchetto));
	//salvo la lista file in un char da tokenizzare
	strcpy(listaFile,pacchettoApplicativo.messaggio);
	sprintf(stringaDaStampare, "  %d: Ho ricevuto la lista file.\n %s\n",getpid(), pacchettoApplicativo.messaggio);
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	

	generaIDtransazione(IDTransazione);
	
	nomeFile=strtok(listaFile,"\r\n");
	while(nomeFile!=NULL){
		if((strcmp(nomeFile,".")==0)||(strcmp(nomeFile,"..")==0)||(strcmp(nomeFile,".svn")==0))
			nomeFile=strtok(NULL,"\r\n");
		else{
// 			printf("\'%s\' nomefile \n",nomeFile);
			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
			strcpy(pacchettoApplicativo.idTransazione,IDTransazione);
			strcpy(pacchettoApplicativo.nomeFile,nomeFile);
			sprintf(stringaDaStampare,"  %d: Comincio a richiedere la copia aggiornata del file \'%s\'. IDtransazione: %s\n", getpid(), pacchettoApplicativo.nomeFile,pacchettoApplicativo.idTransazione);
			writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
			strcpy(pacchettoApplicativo.tipoOperazione, "copia file");
			sendPacchetto(&connessioneSincr, &pacchettoApplicativo);
			//ricevo la dimensione del file
			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
			
			receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
			
			if(strcmp(pacchettoApplicativo.tipoOperazione,"copia file, pronto")==0){
				printf("  %d: Sono pronto a ricevere il file.\n", getpid());
				char nomeFileDaScrivereConPercorso[sizeof(directoryDeiFile) + sizeof(pacchettoApplicativo.nomeFile)];
				strcpy(nomeFileDaScrivereConPercorso, directoryDeiFile);
				strcat(nomeFileDaScrivereConPercorso, nomeFile);
				
				bzero(&pacchettoApplicativo,sizeof(struct pacchetto));
				strcpy(pacchettoApplicativo.tipoOperazione,"copia file, pronto a ricevere");
				strcpy(pacchettoApplicativo.idTransazione,IDTransazione);
				strcpy(pacchettoApplicativo.nomeFile,nomeFile);
				sendPacchetto(&connessioneSincr,&pacchettoApplicativo);
				
				bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
				receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(struct pacchetto));
				
				riceviFile(&connessioneSincr, nomeFileDaScrivereConPercorso, &pacchettoApplicativo);
				printf("  %d: Ho finito la ricezione del file\n", getpid());
			}
		
		}
		nomeFile=strtok(NULL,"\r\n");
	}
	//controlla le operazioni di copia file deve essere contrario rispetto a client server
	
	sprintf(stringaDaStampare,"  %d: Ho terminato la ricezione dei file aggiornati, chiudo la connessione\n", getpid());
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 1);
	bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
	strcpy(pacchettoApplicativo.tipoOperazione, "uscita");
	sendPacchetto(&connessioneSincr, &pacchettoApplicativo);
	
	bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
	receivePacchetto(&connessioneSincr, &pacchettoApplicativo, sizeof(struct pacchetto));
	
	if(strcmp(pacchettoApplicativo.tipoOperazione, "Arrivederci") == 0)
		printf("  %d: Connessione chiusa correttamente\n", getpid());
	
	closeSocket(&connessioneSincr);
	close(descrittoreLogFileServer);
	
	return 1;
}

void stampaIpEportaConLog(struct sockaddr_in *indirizzoIP) {

	int descrittoreLogFileServer;
	char stringaDaStampare[500];
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	sprintf(stringaDaStampare, "%s:%d", inet_ntoa(indirizzoIP->sin_addr), ntohs(indirizzoIP->sin_port));
	writeFileWithLock(descrittoreLogFileServer, stringaDaStampare, 1, 0);
}