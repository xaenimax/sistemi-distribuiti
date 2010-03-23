#include "general.h"

#define SERV_PORT   5193
#define SERVICE_PORT	6000
#define BACKLOG       10
#define MAXLINE     1024

void mainDelFiglio();
void mainDelFiglioDiServizio();
void acceptFiglioNormale();
void acceptFiglioDiServizio();
void interrompi();
void inviaListaFile(int *);

int pid, pidServizio, i;
int listensd, connsd, listensdDiServizio, connsdDiServizio, connessioneDiServizio;
struct sockaddr_in servaddr, servaddrDiServizio;
struct sockaddr_in ricevutoSuAddr;
const char directoryDeiFile[] = "/home/alessandro/workspace/Fork/";
	
main() {

	printf("%d: Avvio del server...\n", getpid());
	
	createSocketStream(&listensd);
	createSocketStream(&listensdDiServizio);

	inizializza_memset(&servaddr, SERV_PORT);
	inizializza_memset(&servaddrDiServizio, SERVICE_PORT);
	
	bindSocket(&listensd, &servaddr);
	bindSocket(&listensdDiServizio, &servaddrDiServizio);
	
	listenSocket(&listensd, BACKLOG);
	listenSocket(&listensdDiServizio, BACKLOG);
	
	pid = fork();
	
	acceptFiglioNormale();
	
	//ciclo di istruzioni del server padre. E' > 0 perché se PID < 0 ho avuto problemi nella creazione del figlio
	if(pid > 0) {
			//Gestisce l'interruzione con ctrl-c
		(void) signal(SIGINT, interrompi);
		
		//il padre crea un altro figlio per le richieste di servizio
		pid = fork();
		acceptFiglioDiServizio();
			
		printf("%d: Server avviato\n", getpid());
		
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
			acceptSocket(&connsd, &listensd);
					//se è stata accettata una connessione normale...
			if(connsd != 0) {
				printf(" %d: Creazione di un figlio in corso...\n", getpid());
				
				pid = fork();
			
				mainDelFiglio();
			
				int ritornoPid;
				
				//remember: non c'è bisogno di fare la close del socket nel figlio in quanto esso ripassa da qua e termina
				
				closeSocket(&connsd);
			}
		}
	}
}

void acceptFiglioDiServizio() {

	if(pid == 0) {
		
		printf(" %d: In attesa di una richiesta di servizio...\n", getpid());
		
		while(1) {
			acceptSocket(&connessioneDiServizio, &listensdDiServizio);
			
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
			time_t        ticks;
			char          buff[MAXLINE];
			
			printf("  %d: Presa in consegna richiesta normale.\n", getpid());
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			
			getpeername(connsd, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			
			printf("  %d: Ricevuta richiesta dall'indirizzo IP: %s:%d. Elaboro la richiesta...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));
				
			/* accetta una connessione con un client */
			ticks = time(NULL); /* legge l'orario usando la chiamata di sistema time */
			/* scrive in buff l'orario nel formato ottenuto da ctime; 
				snprintf impedisce l'overflow del buffer. */
			snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); /* ctime trasforma la data 
					e lora da binario in ASCII. \r\n per carriage return e line feed*/

			/* scrive sul socket di connessione il contenuto di buff */
			if (write(connsd, buff, strlen(buff)) != strlen(buff)) {
				printf("  %d: ", getpid());
				perror("errore in write del figlio\n"); 
				exit(-1);
			}
			
			printf("  %d: Richiesta elaborata!\n", getpid());
			
			exit(0);
		}
}

//questo main dovrà essere usato per gestire le richieste di servizio.
void mainDelFiglioDiServizio() {
	
		if(pid == 0) 
		{
			int dimensioneDatiRicevuti;
			struct pacchetto pacchettoRicevuto, pacchettoDaInviare;

			closeSocket(&listensdDiServizio);
			
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
				if(strcmp(pacchettoRicevuto.tipoOperazione, "Lista File") == 0) {
					inviaListaFile(&connessioneDiServizio);
					
					bzero(&pacchettoRicevuto, sizeof(pacchettoRicevuto));
					dimensioneDatiRicevuti = receivePacchetto(&connessioneDiServizio, &pacchettoRicevuto, sizeof(pacchettoRicevuto));
				}
				
				//richiesta di uscita
				else if (strcmp(pacchettoRicevuto.tipoOperazione, "Uscita") == 0) {
					bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
					
					printf("  %d: Il client chiude la connessione\n", getpid());
					
					strcpy(pacchettoDaInviare.tipoOperazione, "Arrivederci");
					strcpy(pacchettoDaInviare.messaggio, "Arrivederci");

 					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					
					dimensioneDatiRicevuti = 0; //faccio in modo di uscire dal ciclo di attesa di dati da ricevere
				}
				
				//se non riconosco nessuna delle richieste che mi è giunta chiudo la connessione con il client
				else {
					bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
					strcpy(pacchettoDaInviare.messaggio, "Operazione non riconosciuta.\r\n Operazioni permesse: Lista File, Uscita");
					
// 					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare);
					printf("  %d: Operazione \'%s\' non riconosciuta.\n", getpid(), pacchettoRicevuto.messaggio);
					
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


//Effettua la "dir" nella cartella del filesystem distribuito e la invia al client connesso al socket
void inviaListaFile(int *socketConnesso) {
	int numeroDiFileTrovati = 0;
	struct direct **fileTrovati;
	struct pacchetto pacchettoDaInviare;
	char stringaDiFileTrovati[MAXLINE];
	
	//svuoto il buffer di invio
	bzero(&pacchettoDaInviare, sizeof(pacchettoDaInviare));
	
	printf("  %d: Invio la lista file, come richiesto.\n", getpid());

	numeroDiFileTrovati = scandir(directoryDeiFile, &fileTrovati, NULL, alphasort);

		/* If no files found, make a non-selectable menu item */
// 	if 		(numeroDiFileTrovati <= 0) {
// 		printf("  %d: Nessun file trovato!\n", getpid());
// 	}
	
// 	printf("  %d: Numero di file trovati: %d\n", getpid(), numeroDiFileTrovati);
	
	for (i=1;i<numeroDiFileTrovati+1;++i) {
			sprintf(stringaDiFileTrovati, "%s\r\n", fileTrovati[i-1]->d_name);
			strcat(pacchettoDaInviare.messaggio, stringaDiFileTrovati);
	}
	
	sendPacchetto(&connessioneDiServizio, &pacchettoDaInviare, sizeof(pacchettoDaInviare), 0);
}
