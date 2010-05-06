#include "../general.h"
// #include "funzioniServerReplica.h"
// #include "../funzioniGeneriche.h"

#define SERV_PORT   5193
#define SERVICE_PORT	6000
#define BACKLOG       10
#define MAXLINE     1024
#include "../funzioniGeneriche.h"

void mainDelFiglio();
void mainDelFiglioDiServizio();
void acceptFiglioNormale();
void acceptFiglioDiServizio();
void interrompi();


int pid, pidServizio, i, ID_numerico_server;
int listenNormale, connessioneNormale, listenDiServizio, connessioneDiServizio;
struct sockaddr_in indirizzoNormale, indirizzoDiServizio, ricevutoSuAddr;
const char directoryDeiFile[] = "fileCondivisi/";
char fileDiCuiSiIntendeFareIlCommit[10][100];

main( int argc, char *argv[] ) {
        
	if ( argc != 2 ) //andiamo a prendere l'id numerico da riga di comando
  {
	printf( "\n Utilizzo: %s ID numerico \n", argv[0] ); //errore
	exit(1);
  }
  else 
  { 
		ID_numerico_server = atoi(argv[1]);
	}

	printf("%d: Avvio del server numero (ID) %d \n", getpid(), ID_numerico_server);
	
	createSocketStream(&listenNormale);
	createSocketStream(&listenDiServizio);

	inizializza_memset(&indirizzoNormale, SERV_PORT);
	inizializza_memset(&indirizzoDiServizio, SERVICE_PORT);
	
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
		
		while(1) {
			acceptSocket(&connessioneDiServizio, &listenDiServizio);
			
					//se è stata accettata una connessione normale...
			if(connessioneDiServizio != 0) {
				printf(" %d: Creazione di un figlio di servizio in corso...\n", getpid());
				
				pid = fork();
			
		// 				printf("%d: Figlio creato\n", getpid());
				
				mainDelFiglioDiServizio();
				
				//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
				
				closeSocket(&connessioneDiServizio);
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
					numeroDatiRicevuti = receivePacchetto(&connessioneNormale, &pacchettoApplicativo, sizeof(pacchettoApplicativo), 0);
					
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
					richiestaScritturaFile(IDTransazione,pacchettoApplicativo.nomeFile,&pacchettoApplicativo,&connessioneNormale);
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

void interrompi() {
	printf("%d: Il server è stato terminato da console\n", getpid());
	exit(0);
}
