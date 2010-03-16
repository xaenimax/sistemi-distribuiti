tutto : serverConForkEbasta.c function.c clientNormale.c clientDiServizio.c
#	gcc -o serverConFork serverConForkEselect.c function.c
# Attenzione, non compilo il file che contiene il server con Fork e select!
	gcc -o serverConForkEbasta serverConForkEbasta.c function.c
	gcc -o clientNormale clientNormale.c function.c
	gcc -o clientDiServizio clientDiServizio.c function.c

serverConForkEbasta : serverConForkEbasta.c function.c
	gcc -o serverConForkEbasta serverConForkEbasta.c function.c
	
serverConFork : serverConForkEselect.c function.c
	gcc -o serverConFork serverConForkEselect.c function.c

clientNormale : clientNormale.c
	gcc -o clientNormale clientNormale.c function.c
	
clientDiServizio : clientDiServizio.c
	gcc -o clientDiServizio clientDiServizio.c function.c

