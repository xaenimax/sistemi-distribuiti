all : clientNormale.c clientDiServizio.c

	gcc -o clientNormale clientNormale.c ../funzioniGeneriche.c ../funzioniSocket.c ../funzioniDNS.c 
	gcc -o clientDiServizio clientDiServizio.c ../funzioniGeneriche.c ../funzioniSocket.c
	gcc -o clientNormalePacca clientNormalePacca.c ../funzioniGeneriche.c ../funzioniSocket.c ../funzioniDNS.c 

clientNormale : clientNormale.o ../funzioniDNS.o ../funzioniGeneriche.o ../funzioniSocket.o
	gcc clientNormale.o ../funzioniDNS.o ../funzioniGeneriche.o ../funzioniSocket.o -o clientNormale
	
clientDiServizio : clientDiServizio.o ../funzioniGeneriche.o ../funzioniSocket.o 
	gcc -o clientDiServizio.o ../funzioniGeneriche.o ../funzioniSocket.o -o clientDiServizio 

clientNormalePacca : clientNormalePacca.o ../funzioniDNS.o ../funzioniGeneriche.o ../funzioniSocket.o
	gcc clientNormalePacca.o ../funzioniDNS.o ../funzioniGeneriche.o ../funzioniSocket.o -o clientNormalePacca

funzioniDNS.o : ../funzioniDNS.c ../funzioniDNS.h
	gcc -c ../funzioniDNS.c

funzioniGeneriche.o : ../funzioniGeneriche.c ../funzioniGeneriche.h
	gcc -c ../funzioniGeneriche.c

funzioniSocket.o : ../funzioniSocket.c ../funzioniSocket.h
	gcc -c ../funzioniSocket.c


clientNormale.o : clientNormale.c 
	gcc -c clientNormale.c

clientDiServizio.o : clientDiServizio.c 
	gcc -c clientDiServizio.c


