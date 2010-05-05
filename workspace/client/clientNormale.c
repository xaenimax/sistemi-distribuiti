#include "../general.h"

//#define SERV_PORT		5193
#define MAXLINE		1024

main() {

	//**********************parte relativa al dns
	char* riferimento_servreplica = NULL;
	char* indirizzo_servreplica = NULL;
	char* porta_servreplica = 0;


	riferimento_servreplica = (char*)contattaDNS();
    printf(" prova (direttamente dal client) %s \n", riferimento_servreplica);

    //separo indirizzo e porta (spostarlo in una funzione esterna!)
    char *p = strtok (riferimento_servreplica,":");

    if(p != NULL)  { indirizzo_servreplica = p;
    				 p = strtok(NULL,"\n");
    				 if(p!=NULL) porta_servreplica = p;
				    }
    //prove! da rimnuovere
	printf("\n prova indirizzo (direttamente dal client) %s \n", indirizzo_servreplica);
	printf("\n prova porta (direttamente dal client) %s \n", porta_servreplica);
	int porta_servreplicaINT = atoi(porta_servreplica);
	printf("\n prova porta int (direttamente dal client) %d \n", porta_servreplicaINT);


   //*****************



	int socketCL, numeroDatiRicevuti, i, numeroMessaggioInviato;
	//const char IP_ADDRESS[] = "127.0.0.1";  //togli, ora è preso dinamicamente
	struct sockaddr_in servaddr;
	struct pacchetto pacchettoApplicativo;
	char stringaInseritaDallutente[MAXLINE];
		
	numeroMessaggioInviato = 1;
	
	createSocketStream(&socketCL);
	
	memset((void*)&servaddr, 0, sizeof(servaddr)); //azzera il contenuto di servaddr
	
	servaddr.sin_family = AF_INET;
	//servaddr.sin_port = htons(SERV_PORT);
	servaddr.sin_port = htons(porta_servreplicaINT);  //ora è presa dinamicamente
	
	//inetPton(&servaddr, IP_ADDRESS);
	inetPton(&servaddr, indirizzo_servreplica); //ora è preso dinamicamente

	connectSocket(&socketCL, &servaddr);
	printf("%d %s",servaddr.sin_port,(char*)inet_ntoa(servaddr.sin_addr));

	while(1) {
		
		bzero(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		
		printf("Operazione da eseguire:\n");
		inserisciTesto(stringaInseritaDallutente, sizeof(stringaInseritaDallutente));
		printf("%s\n", stringaInseritaDallutente);
		
		strcpy(pacchettoApplicativo.tipoOperazione, stringaInseritaDallutente);
		pacchettoApplicativo.numeroMessaggio = numeroMessaggioInviato;
		
		printf("Invio i dati...\n");
		
		if(sendPacchetto(&socketCL, &pacchettoApplicativo) > 0)
			numeroMessaggioInviato++;
		bzero(&pacchettoApplicativo, sizeof(pacchettoApplicativo));
		receivePacchetto(&socketCL, &pacchettoApplicativo, sizeof(pacchettoApplicativo));
		printf("%s",pacchettoApplicativo.messaggio);
		printf("id generato= %s lunghezza %d" ,pacchettoApplicativo.idTransazione, (int)strlen(pacchettoApplicativo.idTransazione));
		
		
// 		riceve il pacchetto dal server se ha risposto al tentativo di scrittura
// ****************************Marina	
	if(strlen(pacchettoApplicativo.idTransazione)==10)
	{
			printf("id transazione generato \n");
			while((strcmp(pacchettoApplicativo.messaggio,"commit\n")!=0)&&(strcmp(pacchettoApplicativo.messaggio,"abort\n")!=0))
			{
				printf("%s",pacchettoApplicativo.messaggio);
				//se c'è una richiesta di scrittura allora si manda l'id di transazione, dopodichè il server chiede con un while infinito di inserire le modifiche
// 				una fatto commit da parte dell'utente si sottomettono le modifiche effettuate, altrimenti l'abort fa eliminare il file temporaneo
				char stringaImmessa[100];
				inserisciTesto(stringaImmessa,sizeof(stringaImmessa));
				strcat(stringaImmessa,"\n");
				strcat(stringaImmessa,pacchettoApplicativo.messaggio);
				sendPacchetto(&socketCL, &pacchettoApplicativo);
				
				receivePacchetto(&socketCL,&pacchettoApplicativo,sizeof(pacchettoApplicativo));
				
			}
	}
// 		*******************************Marina
		
		
		
		
		
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
