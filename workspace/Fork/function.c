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

//effettua la close sul socket
void closeSocket(int *socketDaChiudere) {
	if (close(*socketDaChiudere) == -1) {  /* chiude la connessione */
		printf("%d: ", getpid());
		perror("errore in close");
		exit(-1);
	}
}