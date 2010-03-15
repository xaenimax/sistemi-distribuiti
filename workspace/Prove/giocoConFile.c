#include <stdio.h>
#include <stdlib.h>

	main(int argc, char *argv) {

	FILE *aproFile;
	
	char buffer[11], i;

	for(i = 0; i < 10; i++) {
		buffer[i] = i + '0'; //Sommando + '0' mi sposto dalla posizione 48 in poi dei caratteri che corrisponde ai numeri 
//		printf("buffer[%d]: %c.\n", i, buffer[i]); debug
	}
	
	buffer[10] = '\0';
	printf("\n");
	
	if((aproFile = fopen("prova.txt", "w")) == NULL) {
		printf("Errore nell'apertura del file");
		exit(0);
	} 
	else {
		printf("Fantastico, hai aperto il file prova.txt con successo. Ora procedo a scriverci dentro...\n\n");
	
		if((fputs(buffer, aproFile)) != -1)
			printf("Ho scritto: %s\n", buffer);
		}
	
	fclose(aproFile);
	
	for(i = 0; i < 10; i++) {
		buffer[i] = '0';
	}
	
	printf("\nOra provo a riaprire il file. Nel frattempo, sai che il buffer contiene: %s\n", buffer);

	if((aproFile = fopen("prova.txt", "r")) == NULL) {
		printf("Errore nell'apertura del file");
		exit(0);
	}	
	else {
			printf("Fantastico, hai aperto il file prova.txt con successo. Ora vediamo cosa c'e' dentro...\n");
			fgets(buffer, 11, aproFile);
			printf("Ho letto sta roba, ti risulta??: %s. Inoltre, ora sono nel file alla posizione: %li\n", buffer, ftell(aproFile));
		}

	fclose(aproFile);
	exit(0);

	}	