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

#include "servConst.h"
#include "../utils_v10.h"
#include "../const.h"

#define BASE_PROG_PATH "./server/repertoireProgs/"
#define MAX_STRING_SIZE_INT 4


void requestHandler(void* arg1);

bool createEmptyProgram(Programm** program, char* progName);
int getFreeIdNumber();

void programCpy(Programm* dest, Programm* src);

bool getProgram(Programm** program, int programId);

void setProgram(Programm* program, bool isNew);

bool getProgramPath(int programId, char**path, char* extension);

bool executionHandler(Programm* program, int programId, int clientSocket);
bool prepareExecuteResponse(Programm* program, ExecuteResponse* executeResponse, char** stdout);
void sendExecuteResponse(ExecuteResponse* executeResponse, char** stdout, int clientSocket);
long getCurrentMs();
void executeAndGetSdtout(void* arg1, void* arg2);

bool compilationHandler(Programm* program, int clientSocket);
bool prepareCompilationResponse(Programm* program, CompilationResponse* compilationResponse, char** errors, int clientSocket);
void sendCompilationResponse(CompilationResponse* compilationResponse, char** errors, int clientSocket);
void compileAndGetErrors(void* arg1, void* arg2, void* arg3);


int main(int argc, char **argv) {
	printf("%s\n", argv[0]);
	//TODO check path
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
	sread(*clientSocket, &request, sizeof(Request));
	Programm* program = NULL;
	if (request.firstInt == EXECUTION_VALUE) {
		getProgram(&program, request.secondInt);
		if (executionHandler(program, request.secondInt, *clientSocket))
			free(program);
	} else {
		bool flag = request.firstInt == ADD_VALUE 
		? createEmptyProgram(&program, request.progName) 
		: getProgram(&program, request.firstInt);
		if (flag && compilationHandler(program, *clientSocket))
			free(program);
	}

	sshutdown(*clientSocket, SHUT_WR);
	sclose(*clientSocket);
}


bool createEmptyProgram(Programm** program, char* progName) {
	if ((*program = (Programm*)malloc(sizeof(Programm))) == NULL) {
		perror("Allocation dynamique de program impossible");
		return false;
	}
	(*program)->programmeID = getFreeIdNumber();
	strcpy((*program)->fichierSource, progName);
	(*program)->hasError = false;
	(*program)->nombreExcecutions = 0;
	(*program)->tempsExcecution = 0;
	setProgram(*program, true);


	return true;
}


int getFreeIdNumber() {
   	int semID = sem_get(SEMA_KEY, NO_SEMAPHORE);
    int sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, 0);
    sem_down0(semID);

    SharedMemoryContent* sharedMemory = sshmat(sharedMemID);

  	sshmdt(sshmat(sharedMemID));
    sem_up0(semID);
	return sharedMemory->logicalSize;
}

//deep copy of struct Program
void programCpy(Programm* dest, Programm* src) {
	dest->programmeID = src->programmeID;
	strcpy(dest->fichierSource, src->fichierSource);
	dest->hasError = src->hasError;
	dest->nombreExcecutions = src->nombreExcecutions;
	dest->tempsExcecution = src->tempsExcecution;
}


bool getProgram(Programm** program, int programId) {
	if (programId < 0) return false;

	if ((*program = (Programm*)malloc(sizeof(Programm))) == NULL) {
		perror("Allocation dynamique de program impossible");
		return false;
	}

    int semID = sem_get(SEMA_KEY, NO_SEMAPHORE);
    int sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, 0);
    sem_down0(semID);

    SharedMemoryContent* content = sshmat(sharedMemID);
    if (programId >= content->logicalSize) {
        perror("id du programme invalide");
	    sem_up0(semID);
	  	sshmdt(sshmat(sharedMemID));
        return false;
    }

    **program = content->programTab[programId];

  	sshmdt(sshmat(sharedMemID));
    sem_up0(semID);
	return true;
}


void setProgram(Programm* program, bool isNew) {
	int semID = sem_get(SEMA_KEY, NO_SEMAPHORE);
    int sharedMemID = sshmget(SHAREDMEM_KEY, SHAREDMEMSIZE, 0);
    sem_down0(semID);

    SharedMemoryContent* content = sshmat(sharedMemID);
    if (isNew) 
    	content->logicalSize++;
    programCpy(&(content->programTab[program->programmeID]), program);
    
  	sshmdt(sshmat(sharedMemID));
    sem_up0(semID);
}


bool getProgramPath(int programId, char** path, char* extension) {
	int size = strlen(BASE_PROG_PATH) + MAX_STRING_SIZE_INT + strlen(extension);
	if ((*path = (char*)malloc(size*sizeof(char))) == NULL) {
		perror("Allocation dynamique de path impossible");
		return false;
	}
	sprintf(*path, "%s%d.%s", BASE_PROG_PATH, programId, extension);
	return true;
}


//--- EXECUTION ---

bool executionHandler(Programm* program, int programId, int clientSocket) {
	ExecuteResponse executeResponse = {programId, GOOD_EXECUTION, 0, 0};
	char* stdout = NULL;

	if (!prepareExecuteResponse(program, &executeResponse, &stdout)) return false;

	if (executeResponse.programState == GOOD_EXECUTION || executeResponse.programState == WRONG_EXECUTION) {
		setProgram(program, false);
		sendExecuteResponse(&executeResponse, &stdout, clientSocket);
		free(stdout);
	} else
		sendExecuteResponse(&executeResponse, &stdout, clientSocket);
	
	return true;
}


bool prepareExecuteResponse(Programm* program, ExecuteResponse* executeResponse, char** stdout) {
	if (strlen(program->fichierSource) == 0)
		executeResponse->programState = NOT_EXIST;
	else if (program->hasError)
		executeResponse->programState = NOT_COMPILE;
	else {
		char* executablePath;
		if (!getProgramPath(executeResponse->n, &executablePath, "out")) return false;

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

		program->nombreExcecutions++;
		program->tempsExcecution += executeResponse->executionTime;
	}
	return true;
}


long getCurrentMs() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000) + tv.tv_usec;
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
	swrite(clientSocket, executeResponse, sizeof(ExecuteResponse));
	swrite(clientSocket, *stdout, strlen(*stdout)*sizeof(char));
	sshutdown(clientSocket, SHUT_WR);
}


//--- COMPILATION ---

bool compilationHandler(Programm* program, int clientSocket) {
	CompilationResponse compilationResponse = {program->programmeID, 0};
	char* errors = NULL;
	prepareCompilationResponse(program, &compilationResponse, &errors, clientSocket);
	setProgram(program, false);
	sendCompilationResponse(&compilationResponse, &errors, clientSocket);

	free(errors);
	return true;
}


bool prepareCompilationResponse(Programm* program, CompilationResponse* compilationResponse, char** errors, int clientSocket) {
	char* inputPath;
	if (!getProgramPath(program->programmeID, &inputPath, "c")) return false;
	char* outputPath;
	if (!getProgramPath(program->programmeID, &outputPath, "out")) return false;
	//write into program file source
	int fd = sopen(inputPath, O_WRONLY | O_CREAT | O_TRUNC, 0744);
 	readThenWrite(clientSocket, fd);
	sclose(fd);
	//compile and get errors in variable
	int pipefd[2];
	spipe(pipefd);
	int childId = fork_and_run3(compileAndGetErrors, inputPath, outputPath, pipefd);
	sclose(pipefd[1]);
	getStringFromInput(errors, pipefd[0]);
	sclose(pipefd[0]);

	int status;
	swaitpid(childId, &status, 0);
	compilationResponse->exitCode = WEXITSTATUS(status);

	free(inputPath);
	free(outputPath);

	program->hasError = (compilationResponse->exitCode == 0) ? false : true;

	return true;
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
	swrite(clientSocket, compilationResponse, sizeof(CompilationResponse));
	swrite(clientSocket, *errors, strlen(*errors) * sizeof(char));
	sshutdown(clientSocket, SHUT_WR);
}