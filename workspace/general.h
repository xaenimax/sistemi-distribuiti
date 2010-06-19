#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <errno.h>

#define percorsoFileDiConfigurazione	"configurazioneServer.cfg"
#define NUMERODISERVERREPLICA	3
#define percorsoFileDiLog	"serverReplica.log"

struct pacchetto {
	char tipoOperazione[200];
	char idTransazione[11];
	unsigned char messaggio[600];
	char nomeFile[350];
	int timeStamp;	
};

struct fileApertiDalServer {
	char nomeFile[100];
	char idTransazione[11];
};

