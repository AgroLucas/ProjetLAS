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


void requestHandler(void* arg1);

void executeProgram(int programId, int clientSocket);
void compile(void* arg1, void* arg2);
void executeAndGetSdtout(void* arg1, void* arg2);

Programm createEmptyProgram(char* progName);
int getFreeIdNumber();
char* generateFreePath(char* progName);
Programm getProgram(int programId);
void modifyProgram(Programm program, int clientSocket);
void compileAndGetErrors(void* arg1, void* arg2);


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
	
	Programm program;
	if (request.firstInt == -1) 
		program = createEmptyProgram(request.progName);
	else 
		program = getProgram(request.firstInt);

	modifyProgram(program, *clientSocket);
}


void executeProgram(int programId, int clientSocket) {
	int progStatus = 1;
	int executionTime = 0;
	int statusCode = -1;

	char* stdout;

	Programm program = getProgram(programId);
	if (program.fichierSource == NULL)
		progStatus = -2;
	else {
		//TODO demander comment gérer le a.out
		char* executablePath;

		int childId = fork_and_run2(compile, program.fichierSource, executablePath);
		int status;
		swaitpid(childId, &status, 0);
		statusCode = WEXITSTATUS(status);
		if (statusCode != 0) {
			progStatus = -1;
			//TODO update hasError from shared memory
		} else {
			int pipefd[2];
			spipe(pipefd);
			childId = fork_and_run2(executeAndGetSdtout, executablePath, pipefd);
			sclose(pipefd[1]);
			getStringFromInput(&stdout, pipefd[0]);
			sclose(pipefd[0]);

			status;
			swaitpid(childId, &status, 0);
			int exitCode = WEXITSTATUS(status);

			if (exitCode != 0) {
				progStatus =  0;
				//TODO update hasError from shared memory
			}
		}	
	}

	ExecuteResponse executeResponse = {programId, progStatus, executionTime, statusCode};

	swrite(clientSocket, &executeResponse, sizeof(executeResponse));
	swrite(clientSocket, stdout, strlen(stdout)*sizeof(char));
	//TODO demander comment faire pour terminer le read autre que par un EOF
	//simulate CTRL + D
	int c = EOF;
	swrite(clientSocket, &c, sizeof(int));

	sclose(clientSocket);
}


void compile(void* arg1, void* arg2) {
	char* path = arg1;
	char* executablePath = arg2;
	sexecl("/bin/gcc", "gcc", "-o", executablePath, path, NULL);
}

//TODO ask how to get execution time and how to return it
void executeAndGetSdtout(void* arg1, void* arg2) {
	char* executablePath = arg1;
	int *pipefd = arg2;

	sclose(pipefd[0]);

	dup2(pipefd[1], 1);

	sexecl(executablePath, NULL);

	sclose(pipefd[1]);
}


Programm createEmptyProgram(char* progName) {
	Programm program;
	program.programmeID = getFreeIdNumber();
	program.fichierSource = generateFreePath(progName);
	program.hasError = false;
	program.nombreExcecutions = 0;
	program.tempsExcecution = 0;

	//TODO put in shared memory


	return program;
}

//TODO return nbr of progs existing+1
int getFreeIdNumber() {
	return 0;
}

//TODO demander comment gérer repertoire de code
char* generateFreePath(char* progName) {
	return NULL;
}

//TODO get from shared memory or NULL if does not exist
Programm getProgram(int programId) {
	Programm program;
	return program;
}


void modifyProgram(Programm program, int clientSocket) {
	overwriteFromInputIntoOutput(clientSocket, program.fichierSource);

	int pipefd[2];
	spipe(pipefd);
	int childId = fork_and_run2(compileAndGetErrors, program.fichierSource, pipefd);
	sclose(pipefd[1]);
	char* errors;
	getStringFromInput(&errors, pipefd[0]);
	sclose(pipefd[0]);

	int status;
	swaitpid(childId, &status, 0);
	int exitCode = WEXITSTATUS(status);

	if (exitCode != 0) {
		//TODO update hasError from shared memory
	}

	CompilationResponse compilationResponse = {program.programmeID, exitCode};

	//send Response to client
	swrite(clientSocket, &compilationResponse, sizeof(compilationResponse));
	swrite(clientSocket, errors, strlen(errors) * sizeof(char));
	
	

	free(errors);
	sclose(clientSocket);
}


void compileAndGetErrors(void* arg1, void* arg2) {
	char* path = arg1;
	int *pipefd = arg2;

	sclose(pipefd[0]);

	dup2(pipefd[1], 2);

	sexecl("/bin/gcc", "gcc", path, NULL);

	sclose(pipefd[1]);
}