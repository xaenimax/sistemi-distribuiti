#include "../general.h"

//#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	//**********************parte relativa al dns
	char riferimento_servreplica[600];
	
	int socketCL, numeroDatiRicevuti, i, portaDelServer;
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	struct timeval tempoDiAttesa;
	char stringaInseritaDallutente[MAXLINE];
	char *cartellaDoveSalvareIfile="fileCondivisi/";
	char *indirizzoIpDelServer;
	
	indirizzoIpDelServer = malloc(16*sizeof(char));

	tempoDiAttesa.tv_sec = 30; //setto la costante per il tempo di attesa della receive a 30 secondi
	
	//La inizializzo io a 111 per farlo entrare nel for
	errno = 111;

	//errno = 111 sarebbe 'Errore nell'apertura della connessione: Connection refused'. Ovvero, il server è spento o l'ip non è raggiungibile
	//errno = 11 scaduto il timeout
	for(i = 1; errno == 111 && i <= NUMERODISERVERREPLICA; i++) {
		int idServer;
		errno = 0; //per evitare che mi chiuda il socket
		createSocketStream(&socketCL);
		setsockopt(socketCL, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tempoDiAttesa, sizeof(struct timeval));
		contattaDNS(riferimento_servreplica);
		separaIpEportaDaStringa(riferimento_servreplica, indirizzoIpDelServer, &portaDelServer, &idServer);
		assegnaIPaServaddr(indirizzoIpDelServer, portaDelServer, &servaddr);
		printf("Provo a connettermi al server %d con ip ", idServer);
		stampaIpEporta(&servaddr);
		printf("\n");
		connectSocket(&socketCL, &servaddr);
		//chiudo il socket solo se non riesco a connettermi
		if(errno == 111)
			closeSocket(&socketCL);
	}
	
	//Vuol dire che ho provato tutti i server ed erano tutti irraggiungibili
	if(i == 4 && errno == 111) {
		printf("Non risulta nessun server attivo! :(\n");
		exit(0);
	}

	while(1) {
		bzero(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		printf("Operazione da eseguire:\n");
		inserisciTesto(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);

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
		
		sendPacchetto(&socketCL, &pacchettoApplicativo);
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Dati inviati. Attendo la ricezione di dati dal server\n");
		
		receivePacchetto(&socketCL, &pacchettoApplicativo, sizeof(struct pacchetto));
		if(errno == 11) {
			//< 5 vuol dire che ho provato 5 volte a rispedire il pacchetto a distanza di 1, 2,3,4,5 secondi
			printf("Il server non risponde da 30 secondi. Chiudo la connessione\n", tempoDiAttesa.tv_sec);
			closeSocket(&socketCL);
			exit(0);
		}
		
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
			int status=1;
			while(status==1)
			{
				printf("[%s]Messaggio server: %s \n",pacchettoApplicativo.tipoOperazione,pacchettoApplicativo.messaggio);
				//se c'è una richiesta di scrittura allora si manda l'id di transazione, dopodichè il server chiede con un while infinito di inserire le modifiche
// 				una fatto commit da parte dell'utente si sottomettono le modifiche effettuate, altrimenti l'abort fa eliminare il file temporaneo
				char stringaImmessa[600],IDtransazione[11];

				strcpy(IDtransazione,pacchettoApplicativo.idTransazione);
				
				bzero(&pacchettoApplicativo,sizeof(pacchettoApplicativo));
				inserisciTesto(stringaImmessa,sizeof(stringaImmessa));
				strcpy(pacchettoApplicativo.messaggio,stringaImmessa);
			
				strcpy(pacchettoApplicativo.idTransazione,IDtransazione);
				
				strcpy(pacchettoApplicativo.tipoOperazione,"scrivi file");
				//simulato errore crash client
				//	sleep (40);
		
				//simulo fail bizantino
		//		strcpy(pacchettoApplicativo.idTransazione,"000000");
				
				sendPacchetto(&socketCL, &pacchettoApplicativo);				
				
				if(strcmp(pacchettoApplicativo.messaggio,"commit")==0)
				{
					printf("Modifiche in salvataggio, attendere...");
					bzero(&pacchettoApplicativo,sizeof(pacchettoApplicativo));
					//marina: altro check del numero di messaggio?
					receivePacchetto(&socketCL,&pacchettoApplicativo,sizeof(pacchettoApplicativo));
					
					//se il commit è andato a buon fine
					if(strcmp(pacchettoApplicativo.tipoOperazione, "commit eseguito") == 0) {
						printf("%s\n", pacchettoApplicativo.messaggio);
						status=0;
					}
					//c'è stato qualche problema con agrawala e il server me lo comunica.
					else
						printf("[%s] %s\n", pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
				}
				else if(strcmp(pacchettoApplicativo.messaggio,"abort")==0)
				{
					printf("Modifiche annullate\n");
					status=0;
				}
				else{
					bzero(&pacchettoApplicativo,sizeof(pacchettoApplicativo));
					receivePacchetto(&socketCL,&pacchettoApplicativo,sizeof(pacchettoApplicativo));
				}	
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
		if(strcmp(pacchettoApplicativo.tipoOperazione, "leggi file, trovato") < 0)
		{
			printf("[%s] %s\n", pacchettoApplicativo.tipoOperazione, pacchettoApplicativo.messaggio);
		}
		
// 		riceve il pacchetto dal server se ha risposto al tentativo di scrittura
// 		printf("Operazione ricevuta: %s\n", pacchettoApplicativo.tipoOperazione);
		
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
