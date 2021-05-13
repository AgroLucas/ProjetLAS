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

#define BASE_PROG_PATH "./server/progs/"
#define MAX_STRING_SIZE_INT 4


void requestHandler(void* arg1);

void executeProgram(int programId, int clientSocket);
void prepareExecuteResponse(ExecuteResponse* executeResponse, char** stdout);
void sendExecuteResponse(ExecuteResponse* executeResponse, char** stdout, int clientSocket);
long getCurrentMs();
void executeAndGetSdtout(void* arg1, void* arg2);

void getProgram(Programm* program, int programId);

void createEmptyProgram(Programm* program, char* progName);
int getFreeIdNumber();

void modifyProgram(Programm* program, int clientSocket);
void prepareCompilationResponse(CompilationResponse* compilationResponse, Programm* program, char** errors, int clientSocket);
void sendCompilationResponse(CompilationResponse* compilationResponse, char** errors, int clientSocket);
void compileAndGetErrors(void* arg1, void* arg2, void* arg3);

void getProgPath(int programId, char**path, char* extension);


int main(int argc, char **argv) {
	if (argc != 2) {
		perror("Usage : ./server/server [port]\n");
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

	if (request.firstInt == EXECUTION_VALUE) return executeProgram(request.secondInt, *clientSocket);
	
	Programm* program;
	request.firstInt == ADD_VALUE ? createEmptyProgram(program, request.progName) : getProgram(program, request.firstInt);

	modifyProgram(program, *clientSocket);
}


//--- EXECUTION ---

void executeProgram(int programId, int clientSocket) {
	ExecuteResponse executeResponse = {programId, GOOD_EXECUTION, 0, 0};
	char* stdout;

	prepareExecuteResponse(&executeResponse, &stdout);
	sendExecuteResponse(&executeResponse, &stdout, clientSocket);
	
	free(stdout);
	sclose(clientSocket);
}


void prepareExecuteResponse(ExecuteResponse* executeResponse, char** stdout) {
	Programm* program;
	getProgram(program, executeResponse->n);
	if (program == NULL)
		executeResponse->programState = NOT_EXIST;
	else if (program->hasError)
		executeResponse->programState = NOT_COMPILE;
	else {											//execute prog and get stdout in variable
		char* executablePath;
		getProgPath(executeResponse->n, &executablePath, "out");

		int pipefd[2];
		spipe(pipefd);
		int status; 

		long beginTime = getCurrentMs();
		int childId = fork_and_run2(executeAndGetSdtout, executablePath, pipefd);
		sclose(pipefd[1]);
		swaitpid(childId, &status, 0);
		executeResponse->executionTime = getCurrentMs() - beginTime;

		getStringFromInput(stdout, pipefd[0]);
		sclose(pipefd[0]);
		executeResponse->exitCode = WEXITSTATUS(status);
		free(executablePath);
		
		if (executeResponse->exitCode != 0)
			executeResponse->programState = WRONG_EXECUTION;
		//todo incrémenter 
	}
}


long getCurrentMs() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + tv.tv_usec;
}



void executeAndGetSdtout(void* arg1, void* arg2) {
	char* executablePath = arg1;
	int *pipefd = arg2;

	sclose(pipefd[0]);
	dup2(pipefd[1], 1);
	sexecl(executablePath, NULL);
	sclose(pipefd[1]);
}


void sendExecuteResponse(ExecuteResponse* executeResponse, char** stdout, int clientSocket) {
	swrite(clientSocket, executeResponse, sizeof(&executeResponse));
	swrite(clientSocket, *stdout, strlen(*stdout)*sizeof(char));
	sshutdown(clientSocket, SHUT_WR);
}


void createEmptyProgram(Programm* program, char* progName) {
	if ((program = (Programm*)malloc(sizeof(Programm))) == NULL) {
		perror("Allocation dynamique de program impossible");
		exit(1);
	}
	program->programmeID = getFreeIdNumber();
	program->fichierSource = progName;
	program->hasError = false;
	program->nombreExcecutions = 0;
	program->tempsExcecution = 0;

	//TODO put in shared memory
}


//TODO
int getFreeIdNumber() {
	//return number of progs in shared memory
	return 0;
}


void modifyProgram(Programm* program, int clientSocket) {
	CompilationResponse compilationResponse = {program->programmeID, 0};
	char* errors;

	prepareCompilationResponse(&compilationResponse, program, &errors, clientSocket);
	sendCompilationResponse(&compilationResponse, &errors, clientSocket);

	free(errors);
	sclose(clientSocket);
}


void prepareCompilationResponse(CompilationResponse* compilationResponse, Programm* program, char** errors, int clientSocket) {
	char* inputPath;
	getProgPath(program->programmeID, &inputPath, "c");
	char* outputPath;
	getProgPath(program->programmeID, &outputPath, "out");
	//write into program file source
	int fd = sopen(inputPath, O_WRONLY | O_CREAT | O_TRUNC, 0744);
 	readThenWrite(clientSocket, fd);
	close(fd);
	//compile and get errors in variable
	int pipefd[2];
	spipe(pipefd);
	int childId = fork_and_run3(compileAndGetErrors, inputPath, outputPath, pipefd);
	sclose(pipefd[1]);
	getStringFromInput(errors, pipefd[0]);
	sclose(pipefd[0]);

	int status;
	swaitpid(childId, &status, 0);
	int exitCode = WEXITSTATUS(status);

	free(inputPath);
	free(outputPath);

	if (exitCode != 0) {
		//TODO update hasError from shared memory
	}
}


void compileAndGetErrors(void* arg1, void* arg2, void* arg3) {
	char* inputPath = arg1;
	char* outputPath = arg2;
	int *pipefd = arg3;

	sclose(pipefd[0]);

	dup2(pipefd[1], 2);

	sexecl("/bin/gcc", "gcc", "-o",  outputPath, inputPath, NULL);

	sclose(pipefd[1]);
}


void sendCompilationResponse(CompilationResponse* compilationResponse, char** errors, int clientSocket) {
	swrite(clientSocket, compilationResponse, sizeof(compilationResponse));
	swrite(clientSocket, *errors, strlen(*errors) * sizeof(char));
	sshutdown(clientSocket, SHUT_WR);
}





//TODO get from shared memory or NULL if does not exist
void getProgram(Programm* program, int programId) {
	if ((program = (Programm*)malloc(sizeof(Programm))) == NULL) {
		printf("Allocation dynamique de program impossible");
		return NULL;
	}
	program->programmeID = programId;
	program->fichierSource = "helloWorld.c";
	program->hasError = false;
	program->nombreExcecutions = 0;
	program->tempsExcecution = 0;
}


void getProgPath(int programId, char** path, char* extension) {
	int size = strlen(BASE_PROG_PATH) + MAX_STRING_SIZE_INT + strlen(extension);
	if ((*path = (char*)malloc(size*sizeof(char))) == NULL) {
		printf("Allocation dynamique de path impossible");
		return NULL;
	}
	sprintf(*path, "%s%d.%s", BASE_PROG_PATH, programId, extension);
}