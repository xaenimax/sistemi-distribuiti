all : serverReplica.c DNSserver.c

	gcc -o serverReplica serverReplica.c ../funzioniGeneriche.c ../funzioniSocket.c ../funzioniServerReplica.c
	gcc -o DNSserver DNSserver.c ../funzioniDNSserver.c ../funzioniGeneriche.c ../funzioniSocket.c
	cp ./fileCondivisi/marina2.txt ./fileCondivisi/marina.txt

	
serverReplica : serverReplica.o ../funzioniGeneriche.o ../funzioniSocket.o ../funzioniServerReplica.o
	gcc serverReplica.o ../funzioniGeneriche.o ../funzioniSocket.o ../funzioniServerReplica.o -o serverReplica

DNSserver : DNSserver.o ../funzioniDNSserver.o ../funzioniGeneriche.o ../funzioniSocket.o 
	gcc DNSserver.o ../funzioniDNSserver.o ../funzioniGeneriche.o ../funzioniSocket.o -o DNSserver


funzioniGeneriche.o : ../funzioniGeneriche.c ../funzioniGeneriche.h
	gcc -c ../funzioniGeneriche.c

funzioniSocket.o : ../funzioniSocket.c ../funzioniSocket.h
	gcc -c ../funzioniSocket.c

funzioniServerReplica.o : ../funzioniServerReplica.c ../funzioniServerReplica.h
	gcc -c ../funzioniServerReplica.c

funzioniDNSserver : ../funzioniDNSserver.c ../funzioniDNSserver
	gcc -c ../funzioniDNSserver.c


serverReplica.o : serverReplica.c 
	gcc -c serverReplica.c

DNSserver.o : DNSserver.c 
	gcc -c DNSserver.c
