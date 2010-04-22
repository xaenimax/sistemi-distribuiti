#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

void accessoMutuaEsclusione(int*);

sem_t mutex;
int variabileCondivisa = 10;

void main(){
  pid_t pid;
  sem_init(&mutex, 0, 1);

  int i;

  for(i =0;i<10;i++){
		pid=fork();
    if(pid==0){
      printf("[%d] Sono nato! :D \n",getpid());
      accessoMutuaEsclusione(&variabileCondivisa);
      exit(0);
    }
  }
	
  if(pid!=0) {
    printf("[%d] Sono il padre \n",getpid());
		sleep(5);
		printf("[%d] Avete finito, ora var vale %d. \n",getpid(), variabileCondivisa);
		exit(0);
	}
}

void accessoMutuaEsclusione(int *variabileCon){
  
	printf("[%d] Sono in attesa... \n",getpid());
  sem_wait(&mutex);
  printf("[%d] Wow, ora tocca a me, incremento %d di 1. \n",getpid(), *variabileCon);
	*variabileCon = *variabileCon + 1;
  //operazione di scrittura
	
  sem_post(&mutex);
	printf("[%d] Fatto! Potete entrare! Var:%d\n",getpid(), *variabileCon);

}