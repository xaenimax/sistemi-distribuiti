#include "../general.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 

int main() {

	int fd, pid;
	char buffer[100], buffer2[100], *percorsoDelFileFIFO = "/tmp/fifo.pacca";
	
	pid = fork();
	
	if(pid > 0) {
		mkfifo(percorsoDelFileFIFO, 0666);
		fd = open(percorsoDelFileFIFO, O_WRONLY);
		sleep(2);
		printf("Scrivo\n");
		write(fd, "ciao", sizeof("ciao"));
		close(fd);
		sleep(2);
	}
	if(pid == 0) {
	while(strcmp(buffer, "ciao") != 0) {
		fd = open(percorsoDelFileFIFO, O_RDONLY);
		read(fd, buffer, sizeof(buffer));
		close(fd);
		printf("  Tento di leggere: %s\n", buffer);
		sleep(2);
	}

	printf("  Ho letto: %s\n", buffer);
	}

	if(pid > 0) {
		remove(percorsoDelFileFIFO);
	}
	
	exit(0);
}