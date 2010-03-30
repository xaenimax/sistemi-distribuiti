#include "../general.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#define SERV_PORT_DNS		7000
#define MAXLINE		1024


main() {

	int socketCl, n;
	char recvline[MAXLINE], bufferDiInvio[MAXLINE];
	const char IP_ADDRESS[] = "127.0.0.1";	
	struct sockaddr_in servaddr;

	//utili per il dns
	char* riferimento_replica = NULL;
	char* indirizzo_sreplica = NULL;
	char* porta_sreplica = 0;
	int porta_replica;

	struct pacchetto richiesta;
	struct pacchetto risposta;
	int byte_ricevuti = 0;



		createSocketStream(&socketCl);
		
		memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr

		bzero(&richiesta, sizeof(richiesta)); //azzero il contenuto dei pacchetti
		bzero(&risposta, sizeof(risposta));
		
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT_DNS);
		
		if(inet_pton(AF_INET, IP_ADDRESS, &servaddr.sin_addr) <= 0) {
			perror("Errore nella conversione dell'indirizzo");
			exit(-1);
		}

		connectSocket(&socketCl, &servaddr);
		
		int lunghezzaAddr = sizeof(servaddr);
			
		getsockname(socketCl, (struct sockaddr *) &servaddr, &lunghezzaAddr); //se voglio sapere chi mi manda la richiesta..

		richiesta.numeroMessaggio = 1;  //incremento il contatore di messaggi scambiati

		printf("%d: Avviato client su Indirizzo: %s Porta: %d.\n", getpid(), (char*)inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
		
		strcpy(richiesta.tipoOperazione, "DNS");  //setto il tipo di operazione

		//strcpy(bufferDiInvio, "Richiesto indirizzo di un server replica");

		printf("\nInvio i richiesta al server:\n");

				if(sendPacchetto(&socketCl, &richiesta) > 0) //invio pacchetto e incremento contatore msg
					(richiesta.numeroMessaggio)++;



		/* scrive sul socket di connessione il contenuto di buff */
		//if (send(socketCl, bufferDiInvio, strlen(bufferDiInvio), 0) < 0) {  //invio la richiesta
		//	printf("%d: ", getpid());
		//	perror("errore in write del figlio\n");
		//	exit(-1);
		//}

 		printf("Dati inviati. Attendo la ricezione di dati dal server\n");

		//bzero(recvline, MAXLINE); //svuota array

 		byte_ricevuti = receivePacchetto(&socketCl, &risposta, sizeof(risposta), 0);

 		// FARE //##############################àbzero(&richiesta, sizeof(richiesta)); //riazzero il pacchetto per utilizzi successivi

 		printf("Ricevuto Indirizzo Server Replica: %s", risposta.messaggio);
		
		//while((n = recv(socketCl, recvline, MAXLINE, 0)) > 0) {

        //			printf("Dati ricevuti: %s\n", recvline);

			riferimento_replica = risposta.messaggio; //memorizzo l'ind. ricevuto in una variabile locale

			printf("prova %s", riferimento_replica);

			//separo indirizzo e porta;
			char *p = strtok (riferimento_replica,":");

			if(p != NULL)  { indirizzo_sreplica = p;
								p = strtok(NULL,"\n");
								if(p!=NULL) porta_sreplica = p;
			}

			printf("\n prova indirizzo  %s \n", indirizzo_sreplica);
			printf("\n prova porta %s \n", porta_sreplica);
			porta_replica = atoi(porta_sreplica);
			printf("\n prova porta int %d \n", porta_replica);

			if(n < 0)
				perror("Errore nella read");

			close(socketCl);
		//} //end while recv

		//adesso il client apre un nuovo socket per connettersi al server replica ricevuto dal dns









	exit(0);  //exit del main
} //end main
