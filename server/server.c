/* Used by server and client */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
/* struct sockaddr_in */
#include <netinet/in.h>
#include <arpa/inet.h>
/* Used for read write open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utils_v10.h"
#include "../const.h"

#define CFLAGS "-std=c11 -pedantic -Wall -Wvla -Werror -D_POSIX_C_SOURCE"


void requestHandler(void* arg1);
int getFreeIdNumber();
void executeProgram(int programId);
void fillProgram(char* path, int programId, int newSockFd);
void compileHandler(void* arg1, void* arg2);


int main(int argc, char **argv) {

	if (argc != 2) {
		printf("Usage : %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int port = atoi(argv[1]);
	int sockFd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

	int newSockFd;
	while(true) {
		newSockFd = saccept(sockFd);
		fork_and_run1(requestHandler, &newSockFd);
	}
}


void requestHandler(void* arg1) {
	int* newSockFd = arg1;

	Request request;
	sread(*newSockFd, &request, sizeof(request));
	if (request.firstInt == -2)	{
		executeProgram(request.secondInt);
		return;
	}
	int programId = request.firstInt;
	if (programId == -1)
		programId = getFreeIdNumber();

	fillProgram(request.source, programId, *newSockFd);
}

//TODO return nbr of progs existing+1
int getFreeIdNumber() {
	return 0;
}


//should return Response and output
void executeProgram(int programId) {

}


void fillProgram(char* path, int programId, int newSockFd) {
	//open and fill file with content
	int fd = sopen(path, O_WRONLY | O_CREAT | O_TRUNC, 0744);

	int sizeRead;
	char content[BUFFER_SIZE];
	do {
		sizeRead = sread(newSockFd, content, BUFFER_SIZE);
		swrite(fd, content, sizeRead*sizeof(char));
	} while (sizeRead != 1);

	int pipefd[2];
	spipe(pipefd);

	//compile prog
	int childId = fork_and_run2(compileHandler, path, pipefd);
	sclose(pipefd[1]);

	//read compile errors
	sizeRead;
	char errors[BUFFER_SIZE];
	do {
		sizeRead = sread(pipefd[0], errors, BUFFER_SIZE);
	} while (sizeRead != 1);
	sclose(pipefd[0]);

	int status;
	swaitpid(childId, &status, 0);

	//send Response to client
	ModificationResponse modificationResponse;
	modificationResponse.n = programId;
	modificationResponse.exitCode = WEXITSTATUS(status);

	swrite(newSockFd, &modificationResponse, sizeof(modificationResponse));
	swrite(newSockFd, errors, strlen(errors)*sizeof(char));
}


void compileHandler(void* arg1, void* arg2) {
	char* path = arg1;
	int *pipefd = arg2;
	sclose(pipefd[0]);

	dup2(2, pipefd[1]);
	sexecl("/bin/gcc", "gcc", CFLAGS, path, "-o", NULL);
	swrite(pipefd[1], " ", sizeof(char));

	sclose(pipefd[1]);
}