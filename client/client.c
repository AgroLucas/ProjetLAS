#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../utils_v10.h"
#include "../const.h"

#define MAX_QUERY_ARGS 3
#define MAX_QUERY_ARG_LENGHT 128
#define QUERY_BUFF_SIZE 3*MAX_QUERY_ARG_LENGHT

/*
*PRE: 	Children process are running.
*POST: 	SIGUSR1 signal are send to both children, shuting them down.
*RET:	Boolean (true = successful shutdown)
*/
bool shutdownChildren();
void runReccurChild(int* pipefd);
void runClockChild(int* pipefd);

/**
*PRE:	query is an array of char* (length = MAX_QUERY_ARGS)
*POST:	A line is read on stdin. query is filled with char* (each entry in the query array has to be freed)
*/
void readQuery(char**);

void addProg(const char*, int, char*);
void replaceProg(const char*, int, int, char*);
void execProgOnce(const char*, int, int);
void execProgReccur(int);
void readThenWrite(int infd, int outfd);
//	=== main ===

/*
*Expected arguments : IP_address, port, delay(sec)
*/
int main(int argc, char const *argv[])
{
	if(argc != 4) {
		perror("Nombre d'arguments invalide.");
		exit(EXIT_FAILURE);
	}
	const char* addr = argv[1];
	int port = atoi(argv[2]);
	int delay = atoi(argv[3]);

	int pipefd[2];
	spipe(pipefd);
	sclose(pipefd[0]);

	fork_and_run1(runReccurChild, pipefd);
	fork_and_run1(runClockChild, pipefd);

	bool quit = false;
	while(!quit) {
		char* query[MAX_QUERY_ARGS];
		readQuery(query);
		char queryType = query[0][0];
		int progNum;
		char* filePath;
		switch(queryType) {
			case 'q':
				quit = true;
				break;
			case '+':
				filePath = query[1];
				addProg(addr, port, filePath);
				break;
			case '.':
				progNum = atoi(query[1]);
				filePath = query[2];
				replaceProg(addr, port, progNum, filePath);
				break;
			case '*':
				progNum = atoi(query[1]);
				execProgReccur(progNum);
				break;
			case '@':
				progNum = atoi(query[1]);
				execProgOnce(addr, port, progNum);
				break;
		}
		for(int i=0; i<MAX_QUERY_ARGS; i++) {
			free(query[i]);
		}
	}
	sclose(pipefd[1])
	exit(killChildren());
}

//	=== Business functions ===

bool killChildren() {
	printf("shutdownChildren\n");
	return EXIT_SUCCESS;
}

void readQuery(char** query) {
	char buffRd[QUERY_BUFF_SIZE];
	if (sread(STDIN_FILENO, buffRd, QUERY_BUFF_SIZE) == -1) {
		perror("ERROR : can not read the file");
		exit(EXIT_FAILURE);
	}
	char* token = strtok(buffRd, " \n");
	int i=0;
	for(int i=0; i<MAX_QUERY_ARGS; i++) {
		//malloc
		char* arg = (char*) malloc(MAX_QUERY_ARG_LENGHT * sizeof(char));
		if(token != NULL) {
			strcpy(arg, token);
			query[i] = arg;
			token = strtok(NULL, " \n");	
		}else {
			query[i] = arg;
		}
	}
}

//	=== Request functions ===

void addProg(const char* addr, int port, char* filePath) {
	printf("add prog '%s'\n", filePath);
	replaceProg(addr, port, -1, filePath);
}

void replaceProg(const char* addr, int port, int progNum, char* filePath) {
	int lenFilePath = strlen(filePath);
	Request req = {progNum, lenFilePath, *filePath};
	int sockfd = initSocketClient(addr, port);
	int filefd = sopen(filePath, O_RDONLY, 0744);
	swrite(sockfd, &req, sizeof(Request));
	readThenWrite(filefd, sockfd);

	sshutdown(sockfd, SHUT_WR);

	CompilationResponse cResponse;
	checkNeg(
		sread(sockfd, &cResponse, sizeof(CompilationResponse)), 
		"Error reading CompilationResponse");

	printf("Replace program no. %d\nExit code: %d\nEventual error msg: \n", 
		cResponse.n, cResponse.exitCode);
	readThenWrite(sockfd, STDOUT_FILENO);
	sclose(sockfd);
}

void execProgOnce(const char* addr, int port, int progNum) {
	Request req = {-2, progNum, ""};

	int sockfd = initSocketClient(addr, port);
	swrite(sockfd, &req, sizeof(Request));
	sshutdown(sockfd, SHUT_WR);

	//read
	printf("Exec. program no. %d", progNum);
	readThenWrite(sockfd, STDOUT_FILENO);
	sclose(sockfd);
}

void execProgReccur(int progNum) {
	printf("add prog num. %d to reccurent programs\n", progNum);
	//TODO
}

//	=== Child functions ===

void runReccurChild(int pipefd) {
	sclose(pipefd[1]);

	//todo while(!reqQuit)

	sclose(pipefd[0]);
}

void runClockChild(int* pipefd) {
	sclose(pipefd[0]);

	//todo while(!clockQuit)

	sclose(pipefd[1]);
}