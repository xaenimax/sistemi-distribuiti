#include "general.h"

#define MAX_QTY 5
#define MAX_STRING_LENGTH 18


//----------- allocazione di memoria per un vettore di 5 stringhe ------//
char** inizializza_lista() {
	char** lista_ind = calloc(MAX_QTY, sizeof(char*));
		int i = 0;
		for (i = 0; i < MAX_QTY; ++i)
		  lista_ind[i] = malloc(MAX_STRING_LENGTH);

		return lista_ind;
}

//----------------prende gli indirizzi da file e li memorizza nel vettore-------------------//
void prendi_indirizzi(char** lista_indirizzi) {

	
	  char c[30];  /* indirizzo, 30 era 17 */
	  FILE *file;
      //static char lista_indirizzi[5][17+1];   // Vettore di 5 stringhe da 17 caratteri
      int i = 0; //indice per scorrere la lista


	  file = fopen("LISTA_SERVER", "r");


	  if(file==NULL) {
	    printf("Error: can't open file.\n");

	    /* fclose(file); DON'T PASS A NULL POINTER TO fclose !! */
	    //return 1;
	  }
	  else {
// 	    printf("File LISTA_server correttamente aperto:\n\n");

	    while(fgets(c, 30, file)!=NULL) { //preleva indirizzi dal file di testo //30 era 17!!!
	    	stpcpy (lista_indirizzi[i], c);  //li memorizza in un vettore locale
	    	i++;

	    }

	    int a = 0;
// 	    for(a=0;a<5;a++) { printf("INDIRIZZO %d: %s", a, lista_indirizzi[a]);}




	    printf("%d: Ho letto gli indirizzi dal file senza problemi\n", getpid());
	    fclose(file);
	  }
}

//------------------------------avanzamento circolare server---------
int scegli_server(int server_scelto) {     //genera circolarmente numeri da 1 a 4
	if (server_scelto == 4) {server_scelto = 0;}
	else ++server_scelto;
	return server_scelto;
}



