/*
*  C Implementation: provoThread
*
* Description: 
*
*
* Author: Vienna Codeluppi, Alessandro Pacca, Marina Dorelli <alessandro.pacca@gmail.com>, (C) 2009
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include <pthread.h>
#include "basic.h"

pthread_t tid;
void *prova(void *arg);



int main() {
/* default behavior*/
	int ret;
	int numero = 5;
	ret = pthread_create(&tid, NULL, prova, (void *)numero);

}

void *prova (void *arg) {
	
	int tempiDiPingDeiServer[numeroDiServerPresenti];
	
	for(int i = 0; i < numeroDiServerPresenti; i++)
		tempiDiPingDeiServer[numeroDiServerPresenti] = i;

		
}