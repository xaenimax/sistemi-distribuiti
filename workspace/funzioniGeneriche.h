
//Serve ad inserire del testo da stdin. Salva il testo in buffer
void inserisciTesto(char *bufferDoveInserireIlTesto, int dimensioneDelBufferDiTesto);

void generaIDtransazione(char *idTransazione);

void spedisciFile(int *socketConnesso, FILE *fileDaLeggere, struct pacchetto *pacchettoApplicativo);

void riceviFile(int *socketConnesso, char *nomeFileDaScrivereConPercorso, struct pacchetto *pacchettoApplicativo);

void svuotaStrutturaListaFile(struct fileApertiDalServer *listaFile);

void stampaIpEporta(struct sockaddr_in *indirizzoIP);

// void spedisciFile(int *socketConnesso, FILE *fileDaLeggere, struct pacchetto *pacchettoApplicativo);

// void riceviFile(int *socketConnesso, char *nomeFileDaScrivereConPercorso, struct pacchetto *pacchettoApplicativo) ;

int copiaFile(FILE *fileOriginaleDaCopiare, FILE *fileDiDestinazione, char *percorsoFileOriginale, char *percorsoFileDiDestinazione, int apriFile,  int scritturaConLock);

void writeFileWithLock(int descrittoreFile, char *contenutoDaScrivere, int stampaAvideo, int aggiungiData);