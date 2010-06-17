all : serverReplica.c DNSserver.c

	gcc -o serverReplica serverReplica.c ../funzioniGeneriche.c ../funzioniSocket.c ../funzioniServerReplicaPacca.c ../funzioniDNSserver.c ../funzioniDNS.c
	gcc -o DNSserver DNSserver.c ../funzioniDNSserver.c ../funzioniGeneriche.c ../funzioniSocket.c
	cp ./fileCondivisi/marina2.txt ./fileCondivisi/marina.txt
	cp serverReplica server2
	cp serverReplica server3
	
serverReplica : serverReplica.o ../funzioniGeneriche.o ../funzioniSocket.o ../funzioniServerReplicaPacca.o
	gcc serverReplica.o ../funzioniGeneriche.o ../funzioniSocket.o ../funzioniServerReplicaPacca.o -o serverReplica

DNSserver : DNSserver.o ../funzioniDNSserver.o ../funzioniGeneriche.o ../funzioniSocket.o 
	gcc DNSserver.o ../funzioniDNSserver.o ../funzioniGeneriche.o ../funzioniSocket.o -o DNSserver


funzioniGeneriche.o : ../funzioniGeneriche.c ../funzioniGeneriche.h
	gcc -c ../funzioniGeneriche.c

funzioniSocket.o : ../funzioniSocket.c ../funzioniSocket.h
	gcc -c ../funzioniSocket.c

funzioniServerReplicaPacca.o : ../funzioniServerReplicaPacca.c ../funzioniServerReplica.h
	gcc -c ../funzioniServerReplica.c

funzioniDNSserver : ../funzioniDNSserver.c ../funzioniDNSserver
	gcc -c ../funzioniDNSserver.c


serverReplica.o : serverReplica.c 
	gcc -c serverReplica.c

DNSserver.o : DNSserver.c 
	gcc -c DNSserver.c
