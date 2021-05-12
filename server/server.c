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
/* Used for gettimeofday */
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utils_v10.h"
#include "../const.h"


void requestHandler(void* arg1);

void executeProgram(int programId, int clientSocket);
long getCurrentMs();
void executeAndGetSdtout(void* arg1, void* arg2);

Programm getProgram(int programId);

Programm createEmptyProgram(char* progName);
int getFreeIdNumber();
char* generateFreePath(char* progName);

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
	
	Programm program = request.firstInt == -1 ? createEmptyProgram(request.progName) : getProgram(request.firstInt);

	modifyProgram(program, *clientSocket);
}


void executeProgram(int programId, int clientSocket) {
	int programState = 1;
	int executionTime = 0;
	int exitCode = 0;
	//TODO GET PATH
	char* executablePath;

	char* stdout;

	Programm program = getProgram(programId);
	if (program.fichierSource == NULL) //program does not exist
		programState = -2;
	else if (program.hasError)
		programState = -1;				//program does not compile
	else {
		//execute prog and get stdout in variable
		int pipefd[2];
		spipe(pipefd);

		int childId = fork_and_run2(executeAndGetSdtout, executablePath, pipefd);
		sclose(pipefd[1]);
		getStringFromInput(&stdout, pipefd[0]);
		sclose(pipefd[0]);

		int status;
		swaitpid(childId, &status, 0);

		exitCode = WEXITSTATUS(status);

		if (exitCode != 0)
			programState =  0;			//program does not end normally
	}	

	//send Response to client
	ExecuteResponse executeResponse = {programId, programState, executionTime, exitCode};
	swrite(clientSocket, &executeResponse, sizeof(executeResponse));
	swrite(clientSocket, stdout, strlen(stdout)*sizeof(char));
	sshutdown(clientSocket, SHUT_WR);
	
	free(stdout);
	sclose(clientSocket);
}


long getCurrentMs() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + tv.tv_usec;
}


//use getTimeOfDay before after then make -
//todo remove close to have only sexecl
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

//TODO
char* generateFreePath(char* progName) {
	return NULL;
}

//TODO get from shared memory or NULL if does not exist
Programm getProgram(int programId) {
	Programm program = {programId, "helloWorld.c", false, 0, 0};
	return program;
}


void modifyProgram(Programm program, int clientSocket) {
	//write into program file source
	overwriteFromInputIntoClosedOutput(clientSocket, program.fichierSource);
	//compile and get errors in variable
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

	//send Response to client
	CompilationResponse compilationResponse = {program.programmeID, exitCode};
	swrite(clientSocket, &compilationResponse, sizeof(compilationResponse));
	swrite(clientSocket, errors, strlen(errors) * sizeof(char));
	sshutdown(clientSocket, SHUT_WR);


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