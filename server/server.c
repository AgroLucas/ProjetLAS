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

void executeProgram(int programId, int clientSocket);

void modifyProgram(char* path, int programId, int clientSocket);
char* getPathName(int programId);
void writeIntoFile(int clientSocket, int fd);
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
	int* clientSocket = arg1;

	Request request;
	sread(*clientSocket, &request, sizeof(request));

	if (request.firstInt == -2)	return executeProgram(request.secondInt, *clientSocket);
	
	int programId = request.firstInt;
	if (programId == -1) programId = getFreeIdNumber();

	modifyProgram(request.name, programId, *clientSocket);
}

//TODO return nbr of progs existing+1
int getFreeIdNumber() {
	return 0;
}


//should return Response and output
void executeProgram(int programId, int clientSocket) {

	//get program path

	char stdout[BUFFER_SIZE];

	ExecuteResponse executeResponse;
	executeResponse.n = programId;
	//TODO
	executeResponse.programState = 0;
	executeResponse.executionTime = 0;
	executeResponse.exitCode = 0;
	swrite(clientSocket, &executeResponse, sizeof(executeResponse));
	swrite(clientSocket, stdout, strlen(stdout)*sizeof(char));
}


void modifyProgram(char* progName, int programId, int clientSocket) {

	char* path = getPathName(programId);
	int fd = sopen(path, O_WRONLY | O_CREAT | O_TRUNC, 0744);

	writeIntoFile(clientSocket, fd);	

	//compile prog
	int pipefd[2];
	spipe(pipefd);
	int childId = fork_and_run2(compileHandler, progName, pipefd);
	sclose(pipefd[1]);

	//read compile errors
	char* errors;
	getStringFromInput(&errors, pipefd[0]);

	sclose(pipefd[0]);

	int status;
	swaitpid(childId, &status, 0);

	
	ModificationResponse modificationResponse;
	modificationResponse.n = programId;
	modificationResponse.exitCode = WEXITSTATUS(status);

	//send Response to client
	swrite(clientSocket, &modificationResponse, sizeof(modificationResponse));
	swrite(clientSocket, errors, strlen(errors)*sizeof(char));
	//demander
	//simulate CTRL + D
	int c = EOF;
	swrite(clientSocket, &c, sizeof(int));

	free(errors);
	sclose(clientSocket);
}


//TODO
char* getPathName(int programId) {
	return NULL;
}


void writeIntoFile(int clientSocket, int fd) {
	int sizeRead;
	char c;
	do {
		sizeRead = sread(clientSocket, &c, sizeof(char));
		if (c != EOF)
			swrite(fd, &c, sizeof(char));
	} while (c != EOF);
}


void compileHandler(void* arg1, void* arg2) {
	char* progName = arg1;
	int *pipefd = arg2;
	sclose(pipefd[0]);

	dup2(pipefd[1], 2);

	sexecl("/bin/gcc", "gcc", progName, NULL);

	sclose(pipefd[1]);
}