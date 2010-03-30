#include "general.h"

//Serve ad inserire del testo da stdin. Salva il testo in buffer
void inserisciTesto(char *bufferDoveInserireIlTesto, int dimensioneDelBufferDiTesto) {
	
	fflush(stdout);
	
	if ( fgets(bufferDoveInserireIlTesto, dimensioneDelBufferDiTesto, stdin) != NULL ) {
		
			char *newline = strchr(bufferDoveInserireIlTesto, '\n'); /* search for newline character */
			
			if ( newline != NULL ) {
				*newline = '\0'; /* overwrite trailing newline */
				}
	}
}

//genera 
void generaIDtransazione(char *idTransazione) {

	int i, numeroRandom;
	i = 0;
	srand(time(NULL));

	while(i < 10) {
		
		//Genera numeri RANDOM da 48 a 122. (122-48=74. Genero numeri random da 0 a 74 e aggiungo 48)
		numeroRandom = 48 + (rand()/(int)(((unsigned)RAND_MAX + 1) / 74));

		//Se il numero random generato è compreso tra questi valori ho i caratteri 0-9,a-z,A-Z
		if((numeroRandom >= 48 && numeroRandom <= 57) || (numeroRandom >= 65 && numeroRandom <= 90) || (numeroRandom >= 97 && numeroRandom <= 122)) {
			idTransazione[i] = numeroRandom;
			i++;
		}
	}
	
	idTransazione[10] = '\0';
}