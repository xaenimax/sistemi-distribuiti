CC = gcc
CFLAGS = -Wall
SRC = client.c selettore.c funzioni_io.c algoritmi.c
OBJ = $(SRC:.c=.o)

all: $(OBJ) 
	${CC} client.o funzioni_io.o -o client
	${CC} selettore.o funzioni_io.o -o selettore

funzioni_io.o: funzioni_io.h

algoritmi.o: algoritmi.h

client.o: funzioni_io.h 

selettore.o: funzioni_io.h algoritmi.h


clean:
	rm -f *.o core 

cleanall:
	rm -f *.o core selettore client
