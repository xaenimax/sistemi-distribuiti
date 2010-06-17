all : client.c 
	gcc -o client client.c ../funzioniGeneriche.c ../funzioniSocket.c ../funzioniDNS.c 

client : client.o ../funzioniDNS.o ../funzioniGeneriche.o ../funzioniSocket.o
	gcc client.o ../funzioniDNS.o ../funzioniGeneriche.o ../funzioniSocket.o -o client

funzioniDNS.o : ../funzioniDNS.c ../funzioniDNS.h
	gcc -c ../funzioniDNS.c

funzioniGeneriche.o : ../funzioniGeneriche.c ../funzioniGeneriche.h
	gcc -c ../funzioniGeneriche.c

funzioniSocket.o : ../funzioniSocket.c ../funzioniSocket.h
	gcc -c ../funzioniSocket.c
client.o : client.c 
	gcc -c client.c