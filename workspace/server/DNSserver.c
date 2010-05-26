#include "../general.h"


#define SERVICE_PORT	7000
#define BACKLOG       10
#define MAXLINE     1024



void fornisciDNS();
void acceptClientDNS();
void interrompi();

int pid, pidServizio, i;
int connsd, listensdDiServizio, connsdDiServizio, connessioneNormale;
struct sockaddr_in servaddrDiServizio;
struct sockaddr_in ricevutoSuAddr;

char** lista_server;
int server_scelto = 0; //ogni volta che un client si connette viene incrementato per selezionare sempre un server diverso
				   //con strategia ad anello
int byte_ricvt = 0;

struct pacchetto richiesta;
struct pacchetto risposta;

	
main() {
  
	printf("%d: Avvio del server...\n", getpid());
	
	createSocketStream(&listensdDiServizio);

	inizializza_memset(&servaddrDiServizio, SERVICE_PORT);
	
	bindSocket(&listensdDiServizio, &servaddrDiServizio);
	
	listenSocket(&listensdDiServizio, BACKLOG);
	
	inizializza_lista_pacca(lista_server);

	prendi_indirizzi(lista_server);    //prelievo e memorizzazione indirizzi da file LISTA_SERVER

	//printf("prova funzioni ok %s", lista_server[4]);

	//--------------------------------------------------------




	pid = fork();  //creo un server figlio
	
	acceptClientDNS();  //che accetta richieste DNS
	

	if(pid > 0) {  //il padre è >0 (se è 0 è il figlio)

		(void) signal(SIGINT, interrompi); //Gestisce l'interruzione con ctrl-c
		
			
		printf("  %d: Server avviato\n\n", getpid());
		
		// -1 sta per aspetto qualasiasi figlio che termina, 0 sta per nessuna opzione, rimango bloccato fino a che non muore qualche figlio.
		waitpid(-1, &pid, 0);
		
		printf("%d: Il server è stato arrestato per qualche errore!\n", getpid());
		exit(0);
	}

}   //-------------------------end main----------------


void acceptClientDNS() {

	if(pid == 0) {
		
		printf("  %d: In attesa di una richiesta di servizio...\n", getpid());
		
		while(1) {
			acceptSocket(&connessioneNormale, &listensdDiServizio);
			
					//se è stata accettata una connessione normale...
			if(connessioneNormale != 0) {
				printf("  %d: Creazione di un figlio di servizio in corso...\n", getpid());
				server_scelto = scegli_server(server_scelto);
				pid = fork();
			
		// 				printf("%d: Figlio creato\n", getpid());
				
				fornisciDNS();

				closeSocket(&connessioneNormale);
			}
		}
	}
}

void fornisciDNS() {

		if(pid == 0) 
		{
			int n;
			char buff[MAXLINE];
			char recvline[MAXLINE];

			closeSocket(&listensdDiServizio);
			
//############snprintf(buff, sizeof(buff), lista_server[3]);  //bufferizzo l'ip del server replica scelto, per poterlo inviare
			

			//printf("  %d: Presa in consegna richiesta di servizio.\n", getpid());
			
			memset((void *)&ricevutoSuAddr, 0, sizeof(ricevutoSuAddr));
			
			int lunghezzaAddr = sizeof(ricevutoSuAddr);
			
			//se voglio sapere chi mi manda la richiesta..
			getpeername(connessioneNormale, (struct sockaddr *) &ricevutoSuAddr, &lunghezzaAddr);
			
			printf("  %d: Ricevuta richiesta dall'indirizzo IP: %s : %d. Elaboro la richiesta di servizio...\n", getpid(), (char*)inet_ntoa(ricevutoSuAddr.sin_addr), ntohs(ricevutoSuAddr.sin_port));

			//while((n = recv(connessioneNormale, recvline, MAXLINE, 0)) > 0) {
				//printf("  %d: Messaggio ricevuto: %s.\n", getpid(), recvline, n);
			
				//if(strcmp(recvline, "Lista File") == 0) {
				//if(recvline) {


//					strcpy(buff, lista_server[server_scelto]);

//					sendData(&connessioneNormale, &buff);
	//			}
				
				//se non riconosco nessuna delle richieste che mi è giunta chiudo la connessione con il client
		//		else
	//				closeSocket(&connessioneNormale);
	//		}  //end while

			byte_ricvt = receivePacchetto(&connessioneNormale, &richiesta, sizeof(richiesta));

			if(strcmp(richiesta.tipoOperazione, "DNS") == 0) {
								strcpy(risposta.messaggio, lista_server[server_scelto]);

								sendPacchetto(&connessioneNormale, &risposta);

								bzero(&richiesta, sizeof(richiesta));
							}
			else printf("\n * * * errore - richiesta sconosciuta * * * \n ");


			
			printf("  %d: Richiesta elaborata!\n\n", getpid());
			
			exit(0);
		}
}
//--------------------------------------------------
void interrompi() {
	printf("%d: Il server è stato terminato da console\n", getpid());
	exit(0);
}
