#include <stdlib.h>
#include <stdio.h>

main() {

	char *pippo;
	
	printf("Inserisci un testo del cazzo: ");
	
	pippo = malloc(sizeof(pippo) * 15);
	
	scanf("%s", pippo, 3);
	
	printf("\nHai scritto: %s. L'indirizzo di pippo è: %p. Ora libero la memoria usata da pippo", pippo, &pippo);
	
	free(pippo);
	
	printf("\nMemoria liberata :D Ora pippo vale: %s. L'indirizzo di pippo è: %p\n", pippo, &pippo);

	exit (0);

}