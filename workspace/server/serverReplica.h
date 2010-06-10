#define NUMERODISERVERREPLICA	3
#include "../general.h"
// #include "funzioniServerReplica.h"
// #include "../funzioniGeneriche.h"
#include "../funzioniGeneriche.h"

#define NORMAL_PORT   5193
#define SERVICE_PORT	6000
#define BACKLOG       10
#define MAXLINE     1024
#define PORTADNS  	7000

int pid, pidServizio, i, ID_numerico_server, pidFiglioAgrawala, portaDiServizio, portaRichiesteNormali, chiaveMemCondivisa;
int listenNormale, connessioneNormale, listenDiServizio, connessioneDiServizio;
struct sockaddr_in indirizzoNormale, indirizzoDiServizio, ricevutoSuAddr,indirizzoAgrawala;
const char directoryDeiFile[] = "fileCondivisi/";
int idSegmentoMemCond;
char stringaIndirizzoDNS[]="127.0.0.1";