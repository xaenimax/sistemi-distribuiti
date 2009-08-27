CC = gcc
CFLAGS = -Wall
SRC = echo_client_sel.c echo_server_sel.c funzioni_io.c
OBJ = $(SRC:.c=.o)

all: $(OBJ) 
	${CC} echo_client_sel.o funzioni_io.o -o echo_client_sel
	${CC} echo_server_sel.o funzioni_io.o -o echo_server_sel

funzioni_io.o: funzioni_io.h

echo_client_sel.o: funzioni_io.h 

echo_server_sel.o: funzioni_io.h


clean:
	rm -f *.o core 

cleanall:
	rm -f *.o core echo_server_sel echo_client_sel
