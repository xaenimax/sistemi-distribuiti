tutto : serverConForkEselect.c function.c clientNormale.c
	gcc -o serverConFork serverConForkEselect.c function.c
	gcc -o clientNormale clientNormale.c function.c
	gcc -o clientDiServizio clientDiServizio.c function.c

serverConFork : serverConForkEselect.c function.c
	gcc -o serverConFork serverConForkEselect.c function.c

clientNormale : clientNormale.c
	gcc -o clientNormale clientNormale.c function.c
	
clientDiServizio : clientDiServizio.c
	gcc -o clientDiServizio clientDiServizio.c function.c