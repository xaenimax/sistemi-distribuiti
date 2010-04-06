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

#include <sys/dir.h>

struct pacchetto {
	char tipoOperazione[200];
	char idTransazione[10];
	unsigned char messaggio[10000];
	int numeroMessaggio;
	char nomeFile[350];
	int timeStamp;	
};