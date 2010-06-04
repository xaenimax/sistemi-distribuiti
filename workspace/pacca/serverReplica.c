#include "serverReplica.h"

void mainDelFiglio();
void mainDelFiglioDiServizio();
void acceptFiglioNormale();
void acceptFiglioDiServizio();
void interrompi();
void mainFiglioAgrawala();

main( int argc, char *argv[] ) {
        
	struct fileApertiDalServer *listaFileAperti;
	
	listaFileAperti = malloc(15*sizeof(struct fileApertiDalServer));
	srand(time(NULL));
	chiaveMemCondivisa = 48 + (rand()/(int)(((unsigned)RAND_MAX + 1) / 74));
	
	
	idSegmentoMemCond = shmget(chiaveMemCondivisa, 15*sizeof(struct fileApertiDalServer), IPC_CREAT|0666); //creo la memoria condivisa. La chiave mi serve per identificare, se voglio, la mem condivisa.
	listaFileAperti = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);

	svuotaStrutturaListaFile(listaFileAperti);
	
	if ( argc < 2 ) //andiamo a prendere l'id numerico da riga di comando
  {
	printf( "\n Utilizzo: %s ID numerico \n", argv[0] ); //errore
	exit(1);
  }
  else 
  { 
		ID_numerico_server = atoi(argv[1]);
	}

	if(argc < 3) {
// 		printf("Porta di servizio non specificata. Verra' usata la porta di servizio %d.\n", SERVICE_PORT);
		portaDiServizio = SERVICE_PORT;
	}
	else
		portaDiServizio = atoi(argv[2]);
	
		if(argc < 4) {
// 		printf("Porta normale non specificata. Verra' usata la porta %d.\n", SERVICE_PORT);
		portaRichiesteNormali = NORMAL_PORT;
	}
	else
		portaRichiesteNormali = atoi(argv[3]);
	
	
	printf("%d: Avvio del server numero (ID) %d. Porta richieste : %d; porta di servizio: %d\n", getpid(), ID_numerico_server, portaRichiesteNormali, portaDiServizio);
	
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
		
		printf("%d: Server avviato:\n", getpid());
		
		// -1 sta per aspetto qualasiasi figlio che termina, 0 sta per nessuna opzione, rimango bloccato fino a che non muore qualche figlio.
		waitpid(-1, &pid, 0);
		
		printf("%d: Il server è stato arrestato per qualche errore!\n", getpid());
		exit(0);
	}
}

void acceptFiglioNormale() {
	if(pid == 0) {
		
		printf(" %d: In attesa di una richiesta normale...\n", getpid());
	
		while(1) {
			acceptSocket(&connessioneNormale, &listenNormale);
					//se è stata accettata una connessione normale...
			if(connessioneNormale != 0) {
				printf(" %d: Creazione di un figlio in corso...\n", getpid());
				
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
		
		printf(" %d: In attesa di una richiesta di servizio...\n", getpid());
		
		pidFiglioAgrawala = fork();
		mainFiglioAgrawala();
		//se sono il padre faccio le accept per le richieste di servizio
		if(pidFiglioAgrawala != 0) {
			while(1) {
				acceptSocket(&connessioneDiServizio, &listenDiServizio);
			
					//se è stata accettata una connessione normale...
				if(connessioneDiServizio != 0) {
					printf(" %d: Creazione di un figlio di servizio in corso...\n", getpid());
				
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
			printf("  %d: Presa in consegna richiesta normale.\n", getpid());
			
			closeSocket(&listenNormale);
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			getpeername(connessioneNormale, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			printf("  %d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));
			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
			
			numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));

			while(numeroDatiRicevuti > 0) {
// 				printf("  %d: Operazione ricevuta: %s\n", getpid(), pacchettoApplicativo.tipoOperazione);
				
				//se il client chiede il logout chiudo la connessione
				if(strcmp(pacchettoApplicativo.tipoOperazione, "uscita") == 0) {					
					printf("  %d:[%s] Il client chiude la connessione\n", getpid(), pacchettoApplicativo.tipoOperazione);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					strcpy(pacchettoApplicativo.tipoOperazione, "Arrivederci");
					strcpy(pacchettoApplicativo.messaggio, "Arrivederci");
					sleep(5);
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
						printf("  %d:[%s] File \'%s\'non trovato\n", getpid(), pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.nomeFile);
						bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
						
						strcpy(pacchettoApplicativo.tipoOperazione, "leggi file");
						strcpy(pacchettoApplicativo.messaggio, "File non trovato\n");
						
						sendPacchetto(&connessioneNormale, &pacchettoApplicativo);
						
						bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
						numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
					}
					
					//Se trovo il file lo spedisco al client.
					else {
						printf("  %d:[%s] File \'%s\' trovato!\n",getpid(), pacchettoApplicativo.tipoOperazione, nomeFileDaLeggere);
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
					generaIDtransazione(pacchettoApplicativo.idTransazione);
					
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
					richiestaScritturaFile(IDTransazione,&pacchettoApplicativo,&connessioneNormale,idSegmentoMemCond);
					numeroDatiRicevuti=receivePacchetto(&connessioneNormale,&pacchettoApplicativo,sizeof(pacchettoApplicativo));
					
				}
				
				else {
					printf("  %d:[%s] Operazione non riconosciuta\n",getpid(), pacchettoApplicativo.tipoOperazione,ntohs(ricevutoSuAddr.sin_port));
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					strcpy(pacchettoApplicativo.tipoOperazione, "Sconosciuta");
					strcpy(pacchettoApplicativo.messaggio, "Operazione non riconosciuta.\r\n Operazioni permesse: leggi file, lista file, copia file, scrivi file, uscita");
					
					sendPacchetto(&connessioneNormale, &pacchettoApplicativo);
					
					bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
					numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
				}
			}
			
			printf("  %d: Richiesta elaborata!\n", getpid());
			
			exit(0);
		}
}

//questo main dovrà essere usato per gestire le richieste di servizio.
void mainDelFiglioDiServizio() { //sta in attesa di richieste di altri server.
	
		if(pid == 0) 
		{
			int dimensioneDatiRicevuti;
// 			char pippo[10];
			struct pacchetto pacchettoRicevuto, pacchettoDaInviare;

			closeSocket(&listenDiServizio);
			
			printf("  %d: Presa in consegna richiesta di servizio.\n", getpid());
					
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			//se voglio sapere chi mi manda la richiesta..
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			getpeername(connessioneDiServizio, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			printf("  %d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta di servizio...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));

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
					
						printf("  %d:[%s] Ricevuta richiesta di commit da parte del server %d\n", getpid(), pacchettoRicevuto.tipoOperazione, pacchettoRicevuto.timeStamp);
						
						int i;
						
						//controllo se sto usando il file
						for(i = 0; i < 10; i++) {
							//se sto usando il file controllo se io ho più priorità dell'altro aspetto fino a che non finisco di usarlo
							while(strcmp(listaFile[i].nomeFile, pacchettoRicevuto.nomeFile) == 0 && ID_numerico_server < pacchettoRicevuto.timeStamp) {
								printf("  %d: Sto usando il file \'%s\', il server %d dovrà aspettare\n", getpid(), listaFile[i].nomeFile, pacchettoRicevuto.timeStamp);
								sleep(10);
							}
						}
						
						bzero(&pacchettoDaInviare, sizeof(struct pacchetto));
						strcpy(pacchettoDaInviare.tipoOperazione, "conferma per il commit");
						strcpy(pacchettoDaInviare.messaggio, "ok");
						
						printf("  %d: Invio la conferma al server %d\n", getpid(), pacchettoRicevuto.timeStamp);
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
					
					printf("  %d: Sto per aggiornare il file: \'%s\'\n", getpid(), nomeFileDaAggiornare);
					
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
					
					copiaFile(fileConAggiornamenti, fileDaAggiornare, NULL, NULL, 0);
					
					fclose(fileDaAggiornare);
					fclose(fileConAggiornamenti);
					
					if(remove(nomeFileConAggiornamenti) < 0)
						printf("  %d: Errore durante la rimozione del file \'%s\'\n", getpid(), nomeFileConAggiornamenti);
					
					dimensioneDatiRicevuti = 0;
				}
				
				//richiesta di uscita
				else if (strcmp(pacchettoRicevuto.tipoOperazione, "uscita") == 0) {
					bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
					
					printf("  %d:[%s] Il client chiude la connessione\n",getpid(), pacchettoRicevuto.tipoOperazione);
					
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
					printf("  %d:[%s] Operazione non riconosciuta.\n", getpid(), pacchettoRicevuto.tipoOperazione);
					
					bzero(&pacchettoRicevuto, sizeof(pacchettoRicevuto));
					dimensioneDatiRicevuti = receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(pacchettoRicevuto));
				}
			}
			
			printf("  %d: Richiesta elaborata!\n", getpid());
			
			exit(0);
		}
}

void mainFiglioAgrawala() {
	if(pidFiglioAgrawala == 0) {
		
		struct fileApertiDalServer *listaFile;
		listaFile = malloc(15*sizeof(struct fileApertiDalServer));
		int i, socketPerRichiestaConferme[4], confermeRicevute = 0, descrittoreFileFifo;
		struct sockaddr_in indirizzoServer[4];
		struct pacchetto pacchettoApplicativo;
		char percorsoFileFifo[50];

		svuotaStrutturaListaFile(listaFile);
		listaFile = (struct fileApertiDalServer*)shmat(idSegmentoMemCond, 0 , 0);
		
		while(1) {
			
			printf("   %d: In attesa di richieste per agrawala..\n", getpid());
			
			//rimango bloccato fino a che non viene riempito l'array che contiene i file di cui bisogna fare il commit
			for(i = 0; strlen(listaFile[i].nomeFile) == 0; i++) {
				if(i == 9) {
					sleep(3); //Controllo ogni secondo se qualcuno vuole fare il commit. Evita che la cpu vada al 100%
					i = -1; // i è -1 perché appena si rifà il ciclo viene incrementato da i++
				}
			}
			
			printf("   %d: Trovata richiesta di commit. File: \'%s\', i: %d\n", getpid(), listaFile[i].nomeFile, i);
			printf("   %d: Chiedo agli altri server la conferma per poter procedere..\n", getpid());
			
			bzero(&indirizzoServer[0], sizeof(struct sockaddr_in));
			bzero(&indirizzoServer[1], sizeof(struct sockaddr_in));
// 			bzero(&indirizzoServer[2], sizeof(struct sockaddr_in));
// 			bzero(&indirizzoServer[3], sizeof(struct sockaddr_in));

			//Questi ip dovranno essere presi dal DNS
			assegnaIPaServaddr("127.0.0.1", 5001, &indirizzoServer[0]);
			assegnaIPaServaddr("127.0.0.1", 5002, &indirizzoServer[1]);
// 			assegnaIPaServaddr("127.0.0.1", 5003, &indirizzoServer[2]);
			
			int iDelWhile = 0;

			while(confermeRicevute < NUMERODISERVERREPLICA-1) {

				createSocketStream(&socketPerRichiestaConferme[1]);
				printf("   %d: Sto per connettermi all'ip: ", getpid());
				stampaIpEporta(&indirizzoServer[iDelWhile]);
				printf("\n");
				connectSocket(&socketPerRichiestaConferme[1], &indirizzoServer[iDelWhile]);
				
				bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
				strcpy(pacchettoApplicativo.nomeFile, listaFile[i].nomeFile);
				strcpy(pacchettoApplicativo.tipoOperazione, "chiedo di fare commit");
				pacchettoApplicativo.timeStamp = ID_numerico_server;
				
				printf("   %d: Invio richiesta di commit al server %d\n",getpid(), pacchettoApplicativo.timeStamp);
				sendPacchetto(&socketPerRichiestaConferme[1], &pacchettoApplicativo);
				
				bzero(&pacchettoApplicativo, sizeof(struct pacchetto));
				receivePacchetto(&socketPerRichiestaConferme[1], &pacchettoApplicativo, sizeof(struct pacchetto));
				
// 				printf("   %d, DEBUG: [%s]: Ricevuto: %s\n",getpid(), pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
				
				//se il server mi da la conferma che posso fare il commit mi segno la sua conferma
				if(strcmp(pacchettoApplicativo.tipoOperazione, "conferma per il commit") == 0 && strcmp(pacchettoApplicativo.messaggio, "ok") == 0) {
					printf("   %d:[%s] Ricevuta conferma dal server %d\n", getpid(), pacchettoApplicativo.tipoOperazione, iDelWhile);
					confermeRicevute++;
				}
				
				closeSocket(&socketPerRichiestaConferme[1]);
				iDelWhile++;
			}
			
			printf("   %d: Ora posso fare il commit, ho ricevuto tutte le conferme!\n", getpid());
			
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
	printf("%d: Il server è stato terminato da console\n", getpid());
	exit(0);
}
