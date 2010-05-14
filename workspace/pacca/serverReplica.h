#define CHIAVEMEMCONDIVISA	23
#define NUMERODISERVERREPLICA	4

int pid, pidServizio, i, ID_numerico_server, pidFiglioAgrawala, portaDiServizio;
int listenNormale, connessioneNormale, listenDiServizio, connessioneDiServizio;
struct sockaddr_in indirizzoNormale, indirizzoDiServizio, ricevutoSuAddr;
const char directoryDeiFile[] = "fileCondivisi/";
int idSegmentoMemCond;