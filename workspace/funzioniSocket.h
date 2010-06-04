

void inizializza_memset(struct sockaddr_in* servaddr, int porta);

void createSocketStream(int *socketDaCreare);

void bindSocket(int *socketSuCuiFareBind, struct sockaddr_in* indirizzoDaAssegnareAlSocket);

void listenSocket(int *socketSucuiFareLaListen, int dimBacklog);

int connectSocket(int *socket, struct sockaddr_in *indirizzoSuCuiEffettuareLaConnect);

void acceptSocket(int *socketDiConnessione, int *socketDiListen);

int receiveData(int *socketConnesso, char *bufferDiRicezione, int dimensioneMassimaDelBuffer);

int sendData (int *socketConnesso, char *buff);

int sendPacchetto(int *socketConnesso, struct pacchetto *pacchettoDaInviare);

int receivePacchetto(int *socketConnesso, struct pacchetto *pacchettoDaInviare, int dimensioneMassimaDelBuffer);

void closeSocket(int *socketDaChiudere);

void inetPton(struct sockaddr_in *indirizzo, char *indirizzoInStringa);

