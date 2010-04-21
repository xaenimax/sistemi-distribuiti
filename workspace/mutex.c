#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

void accessoMutuaEsclusione();

sem_t mutex;

void accessoMutuaEsclusione(){
  
  char pippo[5];
  sem_wait(&mutex);
  
  //operazione di scrittura
  fflush(stdout);
  printf("[%d]Accesso zona mutua esclusione\n",getpid() );
  int contatore=567+37575;
  
  sleep (1);
  fgets(pippo,5,stdin);
  printf("%s\n",pippo);
  fflush(stdout);
 
  sem_post(&mutex);

}

void main(){
  pid_t pid;
  sem_init(&mutex, 0, 1);

  int i;

  for(i =0;i<10;i++){
    if((pid=fork())==0){
      printf("[%d] sono il figlio \n",getpid());
      accessoMutuaEsclusione();
      exit(0);
    }
  }
  if(pid!=0)
     printf("[%d] sono il padre \n",getpid());
 

}