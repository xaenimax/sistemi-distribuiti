#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

void accessoMemCondivisa(int idSegMemCond);

struct stringhe {
char nomeFile[100];
};

sem_t mutex;

void main(){
  pid_t pid;

  int i, idSegmentoMemCond, chiave = 30;
	struct stringhe *arrayDiStruct;

	arrayDiStruct = malloc(10*sizeof(struct stringhe));
	
	idSegmentoMemCond = shmget(chiave, 5, IPC_CREAT|0666); //creo la memoria condivisa. La chiave mi serve per identificare, se voglio, la mem condivisa.
	arrayDiStruct = (struct stringhe*)shmat(idSegmentoMemCond, 0 , 0); //scrivo il valore corrente della mem condivisa. Da finire.
	
	for(i = 0; i < 10; i++)
		strcpy(arrayDiStruct[i].nomeFile, "ciao");
	
	pid=fork();
   if(pid==0){
		 printf("[%d]", getpid());
		 for(i = 0; i < 10; i++)
			printf("%s - ",arrayDiStruct[i].nomeFile);
		 printf("\n");
     accessoMemCondivisa(idSegmentoMemCond);
     exit(0);
  }
	
  if(pid!=0) {
    printf("[%d] Sono il padre \n",getpid());
		system("sleep 2");
		printf("[%d]", getpid());
		for(i = 0; i < 10; i++)
			printf("%s - ", arrayDiStruct[i].nomeFile);
		
		printf("\n");
		exit(0);
	}
}

void accessoMemCondivisa(int idSegMemCond){
  
	int i;
	char intToString[10];
	struct stringhe *arrayDiStringhe;
	arrayDiStringhe = malloc(10*sizeof(struct stringhe));
	
	arrayDiStringhe = (struct stringhe*)shmat(idSegMemCond, 0 , 0); //leggo il valore corrente della mem condivisa e lo salvo dentro la variabile condivisa.
	
	printf("[%d]", getpid());
	
	for(i = 0; i < 10; i++) {
		printf("%s - ",arrayDiStringhe[i].nomeFile);
		sprintf(intToString, "%d", i);
		strcat(arrayDiStringhe[i].nomeFile, intToString);
	}
		
	printf("\n");
}