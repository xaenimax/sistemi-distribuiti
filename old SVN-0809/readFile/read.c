#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

	FILE *fileDaLeggere;
	
	if((fileDaLeggere = fopen("prova.txt", "r")) == NULL) {
		printf("Errore nell'apertura del file.\n");
		exit(-1);
	}
		
	
	
	fclose(fileDaLeggere);
	
	exit(0);
}