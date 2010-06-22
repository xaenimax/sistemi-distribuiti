#include "serverReplica.h"

void mainDelFiglio();
void mainDelFiglioDiServizio();
void acceptFiglioNormale();
void acceptFiglioDiServizio();
void interrompi();
void mainFiglioAgrawala();

main( int argc, char *argv[] ) {

	struct fileApertiDalServer *listaFileAperti;
	
	//Alloco memoria dinamica per l'arrey listaFileAperti
	listaFileAperti = malloc(15*sizeof(struct fileApertiDalServer));
	srand(time(NULL));
	chiaveMemCondivisa = 48 + (rand()/(int)(((unsigned)RAND_MAX + 1) / 74));
	
	idSegmentoMemCond = shmget(chiaveMemCondivisa, 15*sizeof(struct fileApertiDalServer), IPC_CREAT|0666); //creo la memoria condivisa. La chiave mi serve per identificare, se voglio, la mem condivisa.
	listaFileAperti = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);

	svuotaStrutturaListaFile(listaFileAperti);
	
	descrittoreLogFileServer = open(percorsoFileDiLog, O_WRONLY|O_CREAT|O_APPEND, 0666);
	
	leggiFileDiConfigurazione(&ID_numerico_server, &portaRichiesteNormali, &portaDNS, directoryDeiFile, stringaIndirizzoDNS);
	
	portaDiServizio = portaRichiesteNormali + 1000; //Dato che il server DNS manda solo le porte per le richieste normali, la porta di servizio sarà quella normale + 1000. In questo modo, quando agrawala andra' a chiedere le porte al DNS, aggiungera' mille per sapere quale sarà la porta di servizio
	sprintf(bufferPerLog, "%d : Avvio la sincronizzazione del file system..\n",getpid());
	writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
	
	int esitoSincronizzazione=sincronizzazioneFile(directoryDeiFile);
	if(esitoSincronizzazione<=0) {
		sprintf(bufferPerLog, "%d: sbagliato qualcosa, o sono l'unico superstite! Aiuto, non lasciatemi solo!\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
	}
	
	sprintf(bufferPerLog, "%d: Avvio del server numero (ID) %d. Porta richieste : %d; porta di servizio: %d\n", getpid(), ID_numerico_server, portaRichiesteNormali, portaDiServizio);
	writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
	
	createSocketStream(&listenNormale);
	createSocketStream(&listenDiServizio);
	
	inizializza_memset(&indirizzoNormale, portaRichiesteNormali);
	inizializza_memset(&indirizzoDiServizio, portaDiServizio);
	
	int reuse = 1;
	setsockopt(listenNormale, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	setsockopt(listenDiServizio, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	
	bindSocket(&listenNormale, &indirizzoNormale);
	bindSocket(&listenDiServizio, &indirizzoDiServizio);
	
	listenSocket(&listenNormale, BACKLOG);
	listenSocket(&listenDiServizio, BACKLOG);
	
	pid = fork();
	
	acceptFiglioNormale();
	
	//ciclo di istruzioni del server padre. E' > 0 perché se PID < 0 ho avuto problemi nella creazione del figlio
	if(pid > 0) {
			//Gestisce l'interruzione con ctrl-c
		(void) signal(SIGINT, interrompi);
		
		//il padre crea un altro figlio per le richieste di servizio
		pid = fork();
		acceptFiglioDiServizio();
		
		sprintf(bufferPerLog, "%d: Server avviato:\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
		
		// -1 sta per aspetto qualasiasi figlio che termina, 0 sta per nessuna opzione, rimango bloccato fino a che non muore qualche figlio.
		waitpid(-1, &pid, 0);
		
		sprintf(bufferPerLog, "%d: Il server è stato arrestato per qualche errore!\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
		exit(0);
	}
}

void acceptFiglioNormale() {
	if(pid == 0) {
		
		sprintf(bufferPerLog, " %d: In attesa di una richiesta normale...\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
	
		while(1) {
			acceptSocket(&connessioneNormale, &listenNormale);
			struct timeval tempoDiAttesa;
			tempoDiAttesa.tv_sec=60;
			setsockopt(connessioneNormale,SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tempoDiAttesa, sizeof(struct timeval));
			
					//se è stata accettata una connessione normale...
			if(connessioneNormale != 0) {
				sprintf(bufferPerLog, " %d: Creazione di un figlio in corso...\n", getpid());
				writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
				
				pid = fork();
			
				mainDelFiglio();
				
				//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
				
				closeSocket(&connessioneNormale);
			}
		}
	}
}

void acceptFiglioDiServizio() {

	if(pid == 0) {
		
		sprintf(bufferPerLog, " %d: In attesa di una richiesta di servizio...\n", getpid());
		writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
		
		pidFiglioAgrawala = fork();
		mainFiglioAgrawala();
		//se sono il padre faccio le accept per le richieste di servizio
		if(pidFiglioAgrawala != 0) {
			while(1) {
				acceptSocket(&connessioneDiServizio, &listenDiServizio);
			
					//se è stata accettata una connessione normale...
				if(connessioneDiServizio != 0) {
					sprintf(bufferPerLog, " %d: Creazione di un figlio di servizio in corso...\n", getpid());
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
				
					pid = fork();
			
					//printf("%d: Figlio creato\n", getpid());
				
					mainDelFiglioDiServizio();
				
					//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
				
					closeSocket(&connessioneDiServizio);
				}
			}
		}
	}
}

//Questo main dovrà essere usato per gestire il trasferimento di file
void mainDelFiglio() {
	 
		if(pid == 0) 
		{
			struct pacchetto pacchettoApplicativo;
			int numeroDatiRicevuti;
			sprintf(bufferPerLog, "  %d: Presa in consegna richiesta normale.\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			
			closeSocket(&listenNormale);
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			getpeername(connessioneNormale, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			sprintf(bufferPerLog, "  %d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
			
			numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));

			while(numeroDatiRicevuti > 0) {
// 				printf("  %d: Operazione ricevuta: %s\n", getpid(), pacchettoApplicativo.tipoOperazione);
				
				//se il client chiede il logout chiudo la connessione
				if(strcmp(pacchettoApplicativo.tipoOperazione, "uscita") == 0) {					
					sprintf(bufferPerLog, "  %d:[%s] Il client chiude la connessione\n", getpid(), pacchettoApplicativo.tipoOperazione);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					strcpy(pacchettoApplicativo.tipoOperazione, "Arrivederci");
					strcpy(pacchettoApplicativo.messaggio, "Arrivederci");
 					sendPacchetto(&connessioneNormale, &pacchettoApplicativo);
					
					numeroDatiRicevuti = 0; //faccio in modo di uscire dal ciclo di attesa di dati da ricevere
				}
				
				//richiesta lista file
				else if(strcmp(pacchettoApplicativo.tipoOperazione, "lista file") == 0) {
					inviaListaFile(&connessioneNormale, directoryDeiFile);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
				}
				
				else if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file") == 0) {
					char nomeFileDaLeggere[500];
					
					strcpy(nomeFileDaLeggere, directoryDeiFile);
					strcat(nomeFileDaLeggere, pacchettoApplicativo.nomeFile);
					
					FILE *fileDaLeggere = fopen(nomeFileDaLeggere, "rb");
					
					//se non trovo il file spedisco un messaggio e avverto il client
					if(fileDaLeggere == NULL) {
						sprintf(bufferPerLog, "  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.nomeFile);
						writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
						bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
						
						strcpy(pacchettoApplicativo.tipoOperazione, "leggi file");
						strcpy(pacchettoApplicativo.messaggio, "File non trovato\n");
						
						sendPacchetto(&connessioneNormale, &pacchettoApplicativo);
						
						bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
						numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
					}
					
					//Se trovo il file lo spedisco al client.
					else {
						sprintf(bufferPerLog, "  %d:[%s] File \'%s\' trovato!\n",getpid(), pacchettoApplicativo.tipoOperazione, nomeFileDaLeggere);
						writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
// 						int dimensioneDelFile, numeroDiByteLetti;
// 						char bufferFileLetto[sizeof(pacchettoApplicativo.messaggio)];
			
						spedisciFile(&connessioneNormale, fileDaLeggere, &pacchettoApplicativo);
 						
 						bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
 						numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));

					}
				}
				
				else if(strcmp(pacchettoApplicativo.tipoOperazione, "copia file") == 0) {
									
					char nomeFileDaScrivere[350];

					strcpy(nomeFileDaScrivere, pacchettoApplicativo.nomeFile);
					
					sprintf(bufferPerLog, "  %d:[%s] Mi preparo a inviare il file %s\n", getpid(), pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.nomeFile);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					strcpy(pacchettoApplicativo.tipoOperazione, "copia file, pronto a ricevere");
					strcpy(pacchettoApplicativo.nomeFile, nomeFileDaScrivere);
					
					//dico al client che sono pronto a ricevere
					sendPacchetto(&connessioneNormale, &pacchettoApplicativo);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
					
					char nomeFileDaScrivereConPercorso[sizeof(directoryDeiFile) + sizeof(pacchettoApplicativo.nomeFile)];
					strcpy(nomeFileDaScrivereConPercorso, directoryDeiFile);
					strcat(nomeFileDaScrivereConPercorso, pacchettoApplicativo.nomeFile);
				
					riceviFile(&connessioneNormale, nomeFileDaScrivereConPercorso, &pacchettoApplicativo);
											
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));				
				}
				
				else if(strcmp(pacchettoApplicativo.tipoOperazione, "scrivi file") == 0){
					char IDTransazione[10];
					generaIDtransazione(IDTransazione);
					int esitoScrittura=richiestaScritturaFile(IDTransazione,&pacchettoApplicativo,&connessioneNormale,idSegmentoMemCond, ID_numerico_server,directoryDeiFile);
					// se il risultato è >0 la funzione per la richiesta di modifiche ha lavorato correttamente
					if(esitoScrittura>0)
						numeroDatiRicevuti=receivePacchetto(&connessioneNormale,&pacchettoApplicativo,sizeof(pacchettoApplicativo));
					else
						numeroDatiRicevuti=0;
				}
				
				else {
					sprintf(bufferPerLog, "  %d:[%s] Operazione non riconosciuta\n",getpid(), pacchettoApplicativo.tipoOperazione,ntohs(ricevutoSuAddr.sin_port));
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					strcpy(pacchettoApplicativo.tipoOperazione, "Sconosciuta");
					strcpy(pacchettoApplicativo.messaggio, "Operazione non riconosciuta.\r\n Operazioni permesse: leggi file, lista file, copia file, scrivi file, uscita");
					
					sendPacchetto(&connessioneNormale, &pacchettoApplicativo);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
				}
			}
			
			sprintf(bufferPerLog, "  %d: Richiesta elaborata!\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			
			exit(0);
		}
}

//questo main dovrà essere usato per gestire le richieste di servizio.
void mainDelFiglioDiServizio() { //sta in attesa di richieste di altri server.

		if(pid == 0) 
		{
			//int esitoSincronizzazione=sincronizzazioneFile(stringaIndirizzoDNS);
			//if(esitoSincronizzazione<=0)
			//	che fa?
			int dimensioneDatiRicevuti;
// 			char pippo[10];
			struct pacchetto pacchettoRicevuto, pacchettoDaInviare;

			closeSocket(&listenDiServizio);
			
			sprintf(bufferPerLog, "  %d: Presa in consegna richiesta di servizio.\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			//se voglio sapere chi mi manda la richiesta..
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			getpeername(connessioneDiServizio, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			sprintf(bufferPerLog,"  %d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta di servizio...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);

			dimensioneDatiRicevuti = receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(pacchettoRicevuto));
		
			//finchè ho dati nella receive continuo a ricevere e a inviare dati con il socket
			while(dimensioneDatiRicevuti > 0) {
// 				printf("  %d: Ho ricevuto: %s.\n", getpid(), pacchettoRicevuto.tipoOperazione);
			
				//richiesta lista file
				if(strcmp(pacchettoRicevuto.tipoOperazione, "lista file") == 0) {
					inviaListaFile(&connessioneDiServizio, directoryDeiFile);
					
					bzero(&pacchettoRicevuto, sizeof(pacchettoRicevuto));
					dimensioneDatiRicevuti = receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(pacchettoRicevuto));
				}
				
				else if(strcmp(pacchettoRicevuto.tipoOperazione, "chiedo di fare commit") == 0) {
						struct fileApertiDalServer *listaFile;
						listaFile = malloc(15*sizeof(struct fileApertiDalServer));
						
						listaFile = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);
					
						sprintf(bufferPerLog, "  %d:[%s] Ricevuta richiesta di commit da parte del server %d\n", getpid(), pacchettoRicevuto.tipoOperazione, pacchettoRicevuto.timeStamp);
						writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
						
						int i;
						
						//controllo se sto usando il file
						for(i = 0; i < 10; i++) {
							//se sto usando il file controllo se io ho più priorità dell'altro aspetto fino a che non finisco di usarlo
							while(strcmp(listaFile[i].nomeFile, pacchettoRicevuto.nomeFile) == 0 && ID_numerico_server < pacchettoRicevuto.timeStamp) {
								sprintf(bufferPerLog, "  %d: Sto usando il file \'%s\', il server %d dovrà aspettare\n", getpid(), listaFile[i].nomeFile, pacchettoRicevuto.timeStamp);
								writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
								sleep(10);
							}
						}
						
						bzero(&pacchettoDaInviare, sizeof(struct pacchetto));
						strcpy(pacchettoDaInviare.tipoOperazione, "conferma per il commit");
						strcpy(pacchettoDaInviare.messaggio, "ok");
						
						sprintf(bufferPerLog, "  %d: Invio la conferma al server %d\n", getpid(), pacchettoRicevuto.timeStamp);
						writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
						
						sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);	
						
						dimensioneDatiRicevuti = 0;
				}
				
				else if(strcmp(pacchettoRicevuto.tipoOperazione, "aggiorna file") == 0) {
					char idTransazione[sizeof(pacchettoRicevuto.idTransazione)];
					char nomeFileDaAggiornare[sizeof(pacchettoRicevuto.nomeFile)]; //contiene il file che andrà aggiornato
					char nomeFileConAggiornamenti[sizeof(directoryDeiFile) + sizeof(pacchettoRicevuto.nomeFile)]; //contiene il nome del file temporaneo con gli aggiornamenti
					FILE *fileDaAggiornare, *fileConAggiornamenti;
					
					strcpy(idTransazione, pacchettoRicevuto.idTransazione);
					
					strcpy(nomeFileDaAggiornare, directoryDeiFile);
					strcat(nomeFileDaAggiornare, pacchettoRicevuto.nomeFile);
					
					strcpy(nomeFileConAggiornamenti, directoryDeiFile);
					strcat(nomeFileConAggiornamenti, idTransazione);
					strcat(nomeFileConAggiornamenti, ".marina");
					
					sprintf(bufferPerLog, "  %d: Sto per aggiornare il file: \'%s\'\n", getpid(), nomeFileDaAggiornare);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
					//avviso il client che sono pronto a ricevere e gli dico anche qual'è il nome del file che deve mandarmi. Serve per la funzione spedisci file che è richiamata nel client
					bzero(&pacchettoDaInviare, sizeof(struct pacchetto));
					strcpy(pacchettoDaInviare.tipoOperazione, "aggiorna file, pronto a ricevere");
					strcpy(pacchettoDaInviare.nomeFile, pacchettoRicevuto.nomeFile);
					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					
					//attendo la ricezione della dimensione del file che dovrò passare alla funzione riceviFile
					bzero(&pacchettoRicevuto, sizeof(struct pacchetto));
					receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(struct pacchetto));
					riceviFile(&connessioneDiServizio, nomeFileConAggiornamenti, &pacchettoRicevuto);
					
					//Ora che ho ricevuto il file con gli aggiornamenti, aggiorno il file originale
					fileDaAggiornare = fopen(nomeFileDaAggiornare, "a");
					fileConAggiornamenti = fopen(nomeFileConAggiornamenti, "rb");
					
					copiaFile(fileConAggiornamenti, fileDaAggiornare, NULL, NULL, 0, 0);
					
					fclose(fileDaAggiornare);
					fclose(fileConAggiornamenti);
					
					if(remove(nomeFileConAggiornamenti) < 0) {
						sprintf(bufferPerLog, "  %d: Errore durante la rimozione del file \'%s\'\n", getpid(), nomeFileConAggiornamenti);
						writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					}
					
					dimensioneDatiRicevuti = 0;
				}
				
				else if(strcmp(pacchettoRicevuto.tipoOperazione,"copia file") == 0) {
					//marina da modificare
					FILE *fileDaLeggere;
					char IDTransazione[11],tempNomeFile[50];
					
					char nomeFileDaLeggere[(sizeof(pacchettoRicevuto.nomeFile))+(sizeof(directoryDeiFile))];
					strcpy(tempNomeFile,pacchettoRicevuto.nomeFile);
					strcpy(nomeFileDaLeggere,directoryDeiFile);
					strcat(nomeFileDaLeggere,pacchettoRicevuto.nomeFile);
					strcpy(IDTransazione,pacchettoRicevuto.idTransazione);
					
	//				strcat(nomeFileDaLeggere,directoryDeiFile);
					sprintf(bufferPerLog, "  %d: Mi preparo a inviare il file \'%s\'\n", getpid(), nomeFileDaLeggere);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
										
					fileDaLeggere = fopen(nomeFileDaLeggere, "rb");
					
					bzero(&pacchettoRicevuto, sizeof(struct pacchetto));
					//se non trovo il file spedisco un messaggio e avverto il client
					if(fileDaLeggere == NULL) {
						printf("  %d: File \'%s\'non trovato\n", getpid(), pacchettoRicevuto.nomeFile);
						strcpy(pacchettoRicevuto.tipoOperazione, "copia file, non trovato");
						sendPacchetto(&connessioneDiServizio,&pacchettoRicevuto);
						dimensioneDatiRicevuti = 0;
					}
					else{
						strcpy(pacchettoRicevuto.tipoOperazione,"copia file, pronto");
						strcpy(pacchettoRicevuto.idTransazione,IDTransazione);
						strcpy(pacchettoRicevuto.nomeFile,tempNomeFile);
						
						sendPacchetto(&connessioneDiServizio,&pacchettoRicevuto);
						bzero(&pacchettoRicevuto,sizeof(struct pacchetto));
						receivePacchetto(&connessioneDiServizio,&pacchettoRicevuto,sizeof(struct pacchetto));
						if(strcmp(pacchettoRicevuto.tipoOperazione, "copia file, pronto a ricevere") == 0) {
							spedisciFile(&connessioneDiServizio, fileDaLeggere, &pacchettoRicevuto);	
							bzero(&pacchettoRicevuto, sizeof(struct pacchetto));
						}
						dimensioneDatiRicevuti = receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(struct pacchetto));
					}
					/*
					bzero(&pacchettoRicevuto,sizeof(struct pacchetto));
					receivePacchetto(&connessioneDiServizio,pacchettoRicevuto,sizeof(struct pacchetto));
					if(strcmp(pacchettoRicevuto.tipoOperazione, "copia file, pronto a ricevere") == 0) {
					
						bzero(&pacchettoRicevuto, sizeof(struct pacchetto));
					}*/
				}
				
				//richiesta di uscita
				else if (strcmp(pacchettoRicevuto.tipoOperazione, "uscita") == 0) {
					bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
					
					sprintf(bufferPerLog, "  %d:[%s] Il client chiude la connessione\n",getpid(), pacchettoRicevuto.tipoOperazione);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
					strcpy(pacchettoDaInviare.tipoOperazione, "Arrivederci");
					strcpy(pacchettoDaInviare.messaggio, "Arrivederci");

 					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					
					dimensioneDatiRicevuti = 0; //faccio in modo di uscire dal ciclo di attesa di dati da ricevere
				}
				
				//se non riconosco nessuna delle richieste che mi è giunta chiudo la connessione con il client
				else {
					bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
					strcpy(pacchettoDaInviare.messaggio, "Operazione non riconosciuta.\r\n Operazioni permesse: lista file, Uscita");
					
// 					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					sprintf(bufferPerLog, "  %d:[%s] Operazione non riconosciuta.\n", getpid(), pacchettoRicevuto.tipoOperazione);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					
					bzero(&pacchettoRicevuto, sizeof(pacchettoRicevuto));
					dimensioneDatiRicevuti = receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(pacchettoRicevuto));
				}
			}
			
			sprintf(bufferPerLog, "  %d: Richiesta elaborata!\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			
			exit(0);
		}
}

void mainFiglioAgrawala() {
	if(pidFiglioAgrawala == 0) {
		
		struct fileApertiDalServer *listaFile;
		listaFile = malloc(15*sizeof(struct fileApertiDalServer));
		int i, socketPerRichiestaConferme, confermeRicevute = 0, descrittoreFileFifo, portaDaAssegnare, idServer;
		struct sockaddr_in indirizzoServer[4];
		struct pacchetto pacchettoApplicativo;
		char percorsoFileFifo[50], stringaIndirizzoIP[16];

		svuotaStrutturaListaFile(listaFile);
		listaFile = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);

		chiediTuttiGliIpAlDNS(&indirizzoServer, stringaIndirizzoDNS, portaDNS, ID_numerico_server);

		while(1) {
			
			sprintf(bufferPerLog, "   %d: In attesa di richieste per agrawala..\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			//rimango bloccato fino a che non viene riempito l'array che contiene i file di cui bisogna fare il commit
			for(i = 0; strlen(listaFile[i].nomeFile) == 0; i++) {
				if(i == 9) {
					sleep(3); //Controllo ogni secondo se qualcuno vuole fare il commit. Evita che la cpu vada al 100%
					i = -1; // i è -1 perché appena si rifà il ciclo viene incrementato da i++
				}
			}
			
			sprintf(bufferPerLog, "   %d: Trovata richiesta di commit. File: \'%s\', i: %d\n", getpid(), listaFile[i].nomeFile, i);
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			sprintf(bufferPerLog, "   %d: Chiedo agli altri server la conferma per poter procedere..\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			
			int iDelWhile = 0;
			
			while(confermeRicevute < NUMERODISERVERREPLICA-1) {

				//Vuol dire che l'indirizzo che dovrebbe stare in questa posizione è il mio. Non devo contattare me stesso quindi vado avanti. E' uguale a 0 perchè precedentemente evito di settare questo indirizzo se l'id del server con questo ip è uguale al mio
				if(indirizzoServer[iDelWhile].sin_port == 0) {
 					iDelWhile++;
				}
				
				createSocketStream(&socketPerRichiestaConferme);
				sprintf(bufferPerLog, "   %d: Sto per connettermi all'ip: ", getpid());
				writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
				stampaIpEportaConLog(&indirizzoServer[iDelWhile]);
				sprintf(bufferPerLog, "\n");
				writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 0);
				connectSocket(&socketPerRichiestaConferme, &indirizzoServer[iDelWhile]);
				
				//Vuol dire che il server non è attivo
				if(errno == 111) {
					sprintf(bufferPerLog, "   %d: Il server con IP ", getpid());
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					stampaIpEportaConLog(&indirizzoServer[iDelWhile]);
					sprintf(bufferPerLog, " sembra non essere attivo.\n");
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 0);
					
					closeSocket(&socketPerRichiestaConferme);
					confermeRicevute++;
				}
				
				else {
					bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
					strcpy(pacchettoApplicativo.nomeFile, listaFile[i].nomeFile);
					strcpy(pacchettoApplicativo.tipoOperazione, "chiedo di fare commit");
					pacchettoApplicativo.timeStamp = ID_numerico_server;
					
					sprintf(bufferPerLog, "   %d: Invio richiesta di commit al server %d\n",getpid(), pacchettoApplicativo.timeStamp);
					writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
					sendPacchetto(&socketPerRichiestaConferme, &pacchettoApplicativo);
					
					bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
					receivePacchetto(&socketPerRichiestaConferme, &pacchettoApplicativo, sizeof(struct pacchetto));
					
	// 				printf("   %d, DEBUG: [%s]: Ricevuto: %s\n",getpid(), pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
					
					//se il server mi da la conferma che posso fare il commit mi segno la sua conferma
					if(strcmp(pacchettoApplicativo.tipoOperazione, "conferma per il commit") == 0 && strcmp(pacchettoApplicativo.messaggio, "ok") == 0) {
						sprintf(bufferPerLog, "   %d:[%s] Ricevuta conferma dal server %d\n", getpid(), pacchettoApplicativo.tipoOperazione, iDelWhile);
						writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
						confermeRicevute++;
					}
					
					closeSocket(&socketPerRichiestaConferme);
					iDelWhile++;
				}
			}
			
			sprintf(bufferPerLog,"   %d: Ora posso fare il commit, ho ricevuto tutte le conferme!\n", getpid());
			writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
			
			//-------- Scrivo nella Pipe per informare l'altro processo che agrawala è ok e può scrivere il file -----
			strcpy(percorsoFileFifo, "/tmp/");
			strcat(percorsoFileFifo, listaFile[i].idTransazione);
			
			mkfifo(percorsoFileFifo, 0666);
			
			descrittoreFileFifo = open(percorsoFileFifo, O_WRONLY);
			if(descrittoreFileFifo < 0)
				perror("   %d: Errore nella creazione della memoria dinamica\n");
			
			write(descrittoreFileFifo, "Ok", sizeof("ok"));
			close(descrittoreFileFifo);
			//--------------------------------------------------------------------
			
			strcpy(listaFile[i].nomeFile, "");
		}
	}
}

void interrompi() {
	sprintf(bufferPerLog, "%d: Il server è stato terminato da console\n", getpid());
	writeFileWithLock(descrittoreLogFileServer, bufferPerLog, 1, 1);
	exit(0);
}
