#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void accessoMutuaEsclusione(int idSegMemCond);

sem_t mutex;

void main(){
  pid_t pid;
  sem_init(&mutex, 0, 1);

  int i, idSegmentoMemCond, chiave = 30;
	int variabileCondivisa = 10, *ptrToVarCondivisa;

	idSegmentoMemCond = shmget(chiave, 5, IPC_CREAT|0666); //creo la memoria condivisa. La chiave mi serve per identificare, se voglio, la mem condivisa.
	ptrToVarCondivisa = (int*)shmat(idSegmentoMemCond, 0 , SHM_W); //scrivo il valore corrente della mem condivisa. Da finire.
	
	ptrToVarCondivisa = &variabileCondivisa;
	
  for(i =0;i<10;i++){
		pid=fork();
    if(pid==0){
      printf("[%d] Sono nato! :D %d\n",getpid(), *ptrToVarCondivisa);
      accessoMutuaEsclusione(idSegmentoMemCond);
      exit(0);
    }
  }
	
  if(pid!=0) {
    printf("[%d] Sono il padre \n",getpid());
		system("sleep 2");
		printf("[%d] Avete finito, ora var vale %d. \n",getpid(), variabileCondivisa);
		exit(0);
	}
}

void accessoMutuaEsclusione(int idSegMemCond){
  
	int *indirizzoMemCond;
	
	printf("[%d] Sono in attesa... \n",getpid());
  sem_wait(&mutex); 
	indirizzoMemCond = (int*)shmat(idSegMemCond, 0 , SHM_RND); //leggo il valore corrente della mem condivisa e lo salvo dentro la variabile condivisa.
  printf("[%d] Wow, ora tocca a me, incremento %d. \n",getpid(), *indirizzoMemCond);
	*indirizzoMemCond++;
  //operazione di scrittura
	
  sem_post(&mutex);
	printf("[%d] Fatto! Potete entrare! Var:%d\n",getpid(), *indirizzoMemCond);

}