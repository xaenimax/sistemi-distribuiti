#include "../general.h"
// #include "funzioniServerReplica.h"
// #include "../funzioniGeneriche.h"
#include "../funzioniGeneriche.h"

#define NORMAL_PORT   5000
#define SERVICE_PORT	6000 //porta normale + 1000
#define BACKLOG       10
#define MAXLINE     1024

int pid, pidServizio, i, ID_numerico_server, pidFiglioAgrawala, portaDiServizio, portaRichiesteNormali, chiaveMemCondivisa;
int listenNormale, connessioneNormale, listenDiServizio, connessioneDiServizio;
struct sockaddr_in indirizzoNormale, indirizzoDiServizio, ricevutoSuAddr,indirizzoAgrawala;
const char directoryDeiFile[] = "fileCondivisi/";
int idSegmentoMemCond;