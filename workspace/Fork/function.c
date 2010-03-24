#include "general.h"

//inizializza la struct sockaddr_in
void inizializza_memset(struct sockaddr_in* servaddr, int porta) {
	
	memset((void *)servaddr, 0, sizeof(*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = htonl(INADDR_ANY); /* il server accetta 
				connessioni su una qualunque delle sue intefacce di rete */
	servaddr->sin_port = htons(porta); /* numero di porta del server */
}

//crea un socket orientato allo stream
void createSocketStream(int *socketDaCreare) {
	if ((*socketDaCreare = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) { /* crea il socket */
		printf("%d: ", getpid());
		perror("errore in socket\n");
		exit(1);
	}
}

//effettua la bind sul socket e assegna l'indirizzo passato
void bindSocket(int *socketSuCuiFareBind, struct sockaddr_in* indirizzoDaAssegnareAlSocket) {
	/* assegna l'indirizzo al socket normale */
	if ((bind(*socketSuCuiFareBind, (struct sockaddr *) indirizzoDaAssegnareAlSocket, sizeof(*indirizzoDaAssegnareAlSocket))) < 0) { 
		printf("%d: ", getpid());
		perror("errore in bind");
		exit(-1);
	}
}

//effettua la listen su un socket
void listenSocket(int *socketSucuiFareLaListen, int dimBacklog) {
	//listen normale
	if (listen(*socketSucuiFareLaListen, dimBacklog) < 0 ) {
		printf("%d: ", getpid());
		perror("errore in listen");
		exit(-1);
	}
}

//effettua la connect sul socket
void connectSocket(int *socket, struct sockaddr_in *indirizzoSuCuiEffettuareLaConnect) {
		if(connect(*socket, (struct sockaddr *) indirizzoSuCuiEffettuareLaConnect, sizeof(*indirizzoSuCuiEffettuareLaConnect)) < 0) {
			perror("Errore nell'apertura della connessione");
			exit(-1);
		}
}

void acceptSocket(int *socketDiConnessione, int *socketDiListen) {
	if ((*socketDiConnessione = accept(*socketDiListen, (struct sockaddr *)NULL, NULL)) < 0) {
		printf("%d: ", getpid());
		perror("errore in accept");
		exit(-1);
	}
}

int receiveData(int *socketConnesso, char *bufferDiRicezione, int dimensioneMassimaDelBuffer) {
	int numeroDatiRicevuti = 0;
	numeroDatiRicevuti = recv(*socketConnesso, (char*)bufferDiRicezione, dimensioneMassimaDelBuffer, 0);
	if(numeroDatiRicevuti < 0) {
		printf("%d: ", getpid());
		perror("errore in accept");
		exit(-1);
	}
	
	return numeroDatiRicevuti;
}

int sendData (int *socketConnesso, char *buff) {
	
	if (send(*socketConnesso, (char*)buff, strlen((char*)buff), 0) != strlen((char*)buff)) {
		printf("  %d: ", getpid());
		perror("errore in write del figlio\n"); 
		exit(-1);
	}
	
	return 1;
}

//effettua la send del pacchetto applicativo
int sendPacchetto(int *socketConnesso, struct pacchetto *pacchettoDaInviare) {
	
	if (send(*socketConnesso, (void*)pacchettoDaInviare, sizeof(struct pacchetto), 0) != sizeof(struct pacchetto)) {
		printf("  %d: ", getpid());
		perror("errore in write del figlio\n"); 
		exit(-1);
	}
	
	return 1;
}

//effettua la receive del pacchetto applicativo
int receivePacchetto(int *socketConnesso, struct pacchetto *pacchettoDaInviare, int dimensioneMassimaDelBuffer) {
	int numeroDatiRicevuti = 0;
	numeroDatiRicevuti = recv(*socketConnesso, (void*)pacchettoDaInviare, dimensioneMassimaDelBuffer, 0);
	if(numeroDatiRicevuti < 0) {
		printf("%d: ", getpid());
		perror("errore in accept");
		exit(-1);
	}
	
	return numeroDatiRicevuti;
}

//effettua la close sul socket
void closeSocket(int *socketDaChiudere) {
	if (close(*socketDaChiudere) == -1) {  /* chiude la connessione */
		printf("%d: ", getpid());
		perror("errore in close");
		exit(-1);
	}
}

//Serve ad inserire del testo da stdin. Salva il testo in buffer
void inserisciTesto(char *bufferDoveInserireIlTesto) {
	fflush(stdout);
	if ( fgets(bufferDoveInserireIlTesto, sizeof(*bufferDoveInserireIlTesto), stdin) != NULL ) {
		
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