all : ../funzioniGeneriche.c ../funzioniSocket.c clientNormale.c clientDiServizio.c clientDNS.c
#	gcc -o serverConFork serverConForkEselect.c function.c
# Attenzione, non compilo il file che contiene il server con Fork e select!
#	gcc -o serverReplica serverReplica.c funzioniGeneriche.c funzioniSocket.c funzioniServerReplica.c
	gcc -o clientNormale clientNormale.c ../funzioniGeneriche.c ../funzioniSocket.c 
	gcc -o clientDiServizio clientDiServizio.c ../funzioniGeneriche.c ../funzioniSocket.c 
#	gcc -o DNSserver DNSserver.c funzioni_dns.c funzioniGeneriche.c funzioniSocket.c 
	gcc -o clientDNS clientDNS.c ../funzioniGeneriche.c ../funzioniSocket.c 
	
#DNSserver : DNSserver.c funzioniGeneriche.c funzioniSocket.c funzioni_dns.c
#	gcc -o DNSserver DNSserver.c funzioniGeneriche.c funzioniSocket.c funzioni_dns.c
	
clientDNS : clientDNS.c ../funzioniGeneriche.c ../funzioniSocket.c 
	gcc -o clientDNS clientDNS.c ../funzioniGeneriche.c ../funzioniSocket.c 
	
#serverReplica : serverReplica.c funzioniGeneriche.c funzioniSocket.c funzioniServerReplica.c
#	gcc -o serverReplica serverReplica.c funzioniGeneriche.c funzioniSocket.c funzioniServerReplica.c
	
#serverConFork : serverConForkEselect.c funzioniGeneriche.c funzioniSocket.c 
#	gcc -o serverConFork serverConForkEselect.c funzioniGeneriche.c funzioniSocket.c 

clientNormale : clientNormale.c ../funzioniGeneriche.c ../funzioniSocket.c 
	gcc -o clientNormale clientNormale.c ../funzioniGeneriche.c ../funzioniSocket.c 
	
clientDiServizio : clientDiServizio.c ../funzioniGeneriche.c ../funzioniSocket.c 
	gcc -o clientDiServizio clientDiServizio.c ../funzioniGeneriche.c ../funzioniSocket.c 

