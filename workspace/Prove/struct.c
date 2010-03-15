#include <stdio.h>
#include <stdlib.h>

	struct persona {
	char *name;
	int eta;
};

typedef struct persona umano;

main(void) {
	
	umano adele;
	adele.name = "ciao";
	adele.eta = 18;

	printf("%s, %d\n", adele.name, adele.eta);

	exit(0);

}