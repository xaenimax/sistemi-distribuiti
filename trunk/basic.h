#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define SERV_PORT	5193
#define BACKLOG		10
#define MAXLINE		1024

//so che le variabili globali non sono "shiccose" e sono sconsigliate ma penso che queste servano in + punti del codice. In caso contrario le toglierò AP
extern int numeroServerDaContattare;
//questo parametro sarà di configurazione e dovrà essere impostato per specificare il numero di server che forniscono i servizi
extern int numeroDiServerPresenti;

