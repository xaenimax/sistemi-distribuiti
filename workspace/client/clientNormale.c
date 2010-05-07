#include "../general.h"

//#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	//**********************parte relativa al dns
	char* riferimento_servreplica = NULL;
	char* indirizzo_servreplica = NULL;
	char* porta_servreplica = 0;
	
	int socketCL, numeroDatiRicevuti, i, numeroMessaggioInviato;
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char stringaInseritaDallutente[MAXLINE];
	char *cartellaDoveSalvareIfile="fileCondivisi/";
	char indirizzoIpDelServer[15];

	riferimento_servreplica = (char*)contattaDNS();
    //separo indirizzo e porta (spostarlo in una funzione esterna!)
    char *p = strtok (riferimento_servreplica,":");

    if(p != NULL)  { indirizzo_servreplica = p;
    				 p = strtok(NULL,"\n");
    				 if(p!=NULL) porta_servreplica = p;
				    }

	int porta_servreplicaINT = atoi(porta_servreplica);

   //*****************

		
	numeroMessaggioInviato = 1;
	
	createSocketStream(&socketCL);
	
	memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
	
	servaddr.sin_family = AF_INET;

	servaddr.sin_port = htons(porta_servreplicaINT);  //ora è presa dinamicamente
	
	inetPton(&servaddr, indirizzo_servreplica); //ora è preso dinamicamente

	connectSocket(&socketCL, &servaddr);
	printf("%d %s",ntohs(servaddr.sin_port),(char*)inet_ntoa(servaddr.sin_addr));

	while(1) {
		
		bzero(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Operazione da eseguire:\n");
		
		inserisciTesto(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);
		pacchettoApplicativo.numeroMessaggio = numeroMessaggioInviato;
		
		if((strncmp("leggi file", stringaInseritaDallutente, 11) == 0)||(strncmp("scrivi file",stringaInseritaDallutente,11)==0)) {
			printf("Inserire il nome del file che si intende leggere:\n");
			bzero(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
			strcpy(pacchettoApplicativo.nomeFile, stringaInseritaDallutente);
		}
		
// 		commentata perchè serve a copiare un file dal client al server e non ci serve ma potrebbe servire
// 		else if(strncmp("copia file", stringaInseritaDallutente, 11) == 0) {
// 		
// 			printf("Inserire il nome del file che si intende scrivere:\n");
// 			bzero(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
// 			inserisciTesto(&stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
// 			strcpy(pacchettoApplicativo.nomeFile, stringaInseritaDallutente);
// 			
// 			char nomeFileDaLeggere[500];
// 			
// 			strcpy(nomeFileDaLeggere, cartellaDoveSalvareIfile);
// 			strcat(nomeFileDaLeggere, pacchettoApplicativo.nomeFile);
// 			
// 			fileDaLeggere = fopen(nomeFileDaLeggere, "rb");
// 			
// 			//se non trovo il file spedisco un messaggio e avverto il client
// 			if(fileDaLeggere == NULL) {
// 				printf("  %d: File \'%s\'non trovato\n", getpid(), pacchettoApplicativo.nomeFile);
// 				bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
// 				strcpy(pacchettoApplicativo.tipoOperazione, "non inviare");
// 			}
// 		}
		
		printf("Invio i dati...\n");
		
		if(sendPacchetto(&socketCL, &pacchettoApplicativo) > 0)
			numeroMessaggioInviato++;
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Dati inviati. Attendo la ricezione di dati dal server\n");
		
		receivePacchetto(&socketCL, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		//se il server ha trovato il file me lo comunica e comincio a scriverlo
		if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file, trovato") == 0) {
			
			char nomeFileDaScrivereConPercorso[sizeof(cartellaDoveSalvareIfile) + sizeof(pacchettoApplicativo.nomeFile)];
			strcpy(nomeFileDaScrivereConPercorso, cartellaDoveSalvareIfile);
			strcat(nomeFileDaScrivereConPercorso, pacchettoApplicativo.nomeFile);
			
			riceviFile(&socketCL, nomeFileDaScrivereConPercorso, &pacchettoApplicativo);
		}
		
		// ****************************Marina	
		else if(strcmp(pacchettoApplicativo.tipoOperazione,"scrivi file, pronto")==0)
		{
			printf("ID transazione generato %s\n",pacchettoApplicativo.idTransazione);
			while((strcmp(pacchettoApplicativo.messaggio,"commit")!=0)&&(strcmp(pacchettoApplicativo.messaggio,"abort")!=0))
			{
				printf("messaggio server: %s\n",pacchettoApplicativo.messaggio);
				//se c'è una richiesta di scrittura allora si manda l'id di transazione, dopodichè il server chiede con un while infinito di inserire le modifiche
// 				una fatto commit da parte dell'utente si sottomettono le modifiche effettuate, altrimenti l'abort fa eliminare il file temporaneo
				char stringaImmessa[100],IDtransazione[10];
				
				
				strcpy(IDtransazione,pacchettoApplicativo.idTransazione);
				
				bzero(&pacchettoApplicativo,sizeof(pacchettoApplicativo));
				inserisciTesto(stringaImmessa,sizeof(stringaImmessa));
				strcpy(pacchettoApplicativo.messaggio,stringaImmessa);
				printf("ho scritto %s\n",pacchettoApplicativo.messaggio);
				strcpy(pacchettoApplicativo.idTransazione,IDtransazione);
				strcpy(pacchettoApplicativo.tipoOperazione,"scrivi file");
				
				sendPacchetto(&socketCL, &pacchettoApplicativo);
				
				bzero(&pacchettoApplicativo,sizeof(pacchettoApplicativo));
				
				receivePacchetto(&socketCL,&pacchettoApplicativo,sizeof(pacchettoApplicativo));
				
				
			}
		}
// 		*******************************Marina
// 		commentato perchè non ci serve la copia del file sul server
// 		if(strcmp(pacchettoApplicativo.tipoOperazione, "copia file, pronto a ricevere") == 0) {
// 			
// 			spedisciFile(&socketCl, fileDaLeggere, &pacchettoApplicativo);	
//  			bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
// 	}
		
		//se il server fa una qualunque altra operazione che non sia collegata al file trovato, stampo il messaggio che altrimenti sarebbe incomprensibile
		if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file, trovato") < 0) {
			printf("[%s] %s\n", pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
		}
		
// 		riceve il pacchetto dal server se ha risposto al tentativo di scrittura
		printf("Operazione ricevuta: %s\n", pacchettoApplicativo.tipoOperazione);
		
		if(strcmp(pacchettoApplicativo.tipoOperazione, "Arrivederci") == 0) {
			printf("Chiudo la connessione\n");
			closeSocket(&socketCL);
			break;
		}
		
		else if(strcmp(pacchettoApplicativo.tipoOperazione, "Sconosciuta") == 0) {
			printf("%s\n", pacchettoApplicativo.messaggio);
		}
	}
	exit(0);
}
