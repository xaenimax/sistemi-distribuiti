all : serverReplica.c function.c funzioni_dns.c clientNormale.c clientDiServizio.c DNSserver.c clientDNS.c
#	gcc -o serverConFork serverConForkEselect.c function.c
# Attenzione, non compilo il file che contiene il server con Fork e select!
	gcc -o serverReplica serverReplica.c function.c
	gcc -o clientNormale clientNormale.c function.c
	gcc -o clientDiServizio clientDiServizio.c function.c
	gcc -o DNSserver DNSserver.c function.c funzioni_dns.c
	gcc -o clientDNS clientDNS.c function.c
	
DNSserver : DNSserver.c function.c funzioni_dns.c
	gcc -o DNSserver DNSserver.c function.c funzioni_dns.c
	
clientDNS : clientDNS.c function.c
	gcc -o clientDNS clientDNS.c function.c
	
serverReplica : serverReplica.c function.c
	gcc -o serverConForkEbasta serverConForkEbasta.c function.c
	
serverConFork : serverConForkEselect.c function.c
	gcc -o serverConFork serverConForkEselect.c function.c

clientNormale : clientNormale.c
	gcc -o clientNormale clientNormale.c function.c
	
clientDiServizio : clientDiServizio.c
	gcc -o clientDiServizio clientDiServizio.c function.c

