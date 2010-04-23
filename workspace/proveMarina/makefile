all : mutex.c
#	gcc -o serverConFork serverConForkEselect.c function.c
# Attenzione, non compilo il file che contiene il server con Fork e select!
	gcc -o mutex mutex.c -lpthread
	
mutex : mutex.c
#	gcc -o serverConFork serverConForkEselect.c function.c
# Attenzione, non compilo il file che contiene il server con Fork e select!
	gcc -o mutex mutex.c -lpthread
utility : utility.o ../funzioniGeneriche.o ../funzioniSocket.o
	gcc utility.o ../funzioniGeneriche.o ../funzioniSocket.o -o utility
	cp fileorigine.txt filediriferimento.txt

funzioniSocket.o : funzioniSocket.c
	gcc -c ../funzioniSocket.c

funzioniGeneriche.o : ../funzioniGeneriche.c ../funzioniGeneriche.h
	gcc -c ../funzioniGeneriche.c

utility.o : utility.c 
	gcc -c utility.c