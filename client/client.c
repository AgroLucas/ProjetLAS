#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../utils_v10.h"
#include "../const.h"
#include "clientConst.h"

volatile sig_atomic_t terminate_recur = 0;
volatile sig_atomic_t terminate_clock = 0;

volatile sig_atomic_t clock_kill_receipt = 0;
volatile sig_atomic_t reccur_kill_receipt = 0;


/*
*PRE: 	Children process are running.
*POST: 	SIGUSR1 signal are send to both children, shuting them down.
*RET:	Boolean (true = successful shutdown)
*/
bool killChildren(int, int);
void clockSigchldHandler(int sig);
void reccurSigchldHandler(int sig);
void runReccurChild(void* arg1, void* arg2, void* arg3); //
void runClockChild(void* arg1, void* arg2); //

/**
*PRE:	query is an array of char* (length = MAX_QUERY_ARGS)
*POST:	A line is read on stdin. query is filled with char* (each entry in the query array has to be freed)
*/
void readQuery(char**);

void addProg(char*, int, char*);
void replaceProg(char*, int, int, char*);
void execProgOnce(char*, int, int);
void execProgReccur(int progNum, int* pipefd);

void reccurSigusr1Handler(int sig);
void clockSigusr1Handler(int sig);

//	=== main ===

/*
*Expected arguments : IP_address, port, delay(sec)
*/
int main(int argc, char *argv[])
{
	if(argc != 4) {
		perror("Nombre d'arguments invalide.");
		exit(EXIT_FAILURE);
	}
 	char* addr = argv[1];
	int port = atoi(argv[2]);
	int delay = atoi(argv[3]);

	int pipefd[2];
	spipe(pipefd);

	sigset_t set;
	ssigemptyset(&set);
	ssigaddset(&set, SIGUSR1);
	ssigprocmask(SIG_BLOCK, &set, NULL);

	int reccurPid = fork_and_run3(runReccurChild, pipefd, addr, &port);
	int clockPid = fork_and_run2(runClockChild, pipefd, &delay);
	sclose(pipefd[0]);

	bool quit = false;
	while(!quit) {
		char* query[MAX_QUERY_ARGS];
		printf("Entrez une commande: \n");
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
				execProgReccur(progNum, pipefd);
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
	sclose(pipefd[1]);
	exit(killChildren(clockPid, reccurPid));
}

//	=== Parent Business functions ===

bool killChildren(int clockPid, int reccurPid) {
	ssigaction(SIGCHLD, clockSigchldHandler);
	skill(clockPid, SIGUSR1);
	while(clock_kill_receipt == 0) {
		// wait
	}
	printf("(Parent) clock kill receipt\n");

	ssigaction(SIGCHLD, reccurSigchldHandler);
	skill(reccurPid, SIGUSR1);
	while(reccur_kill_receipt == 0) {
		// wait
	}
	printf("(Parent) reccur kill receipt\n");
	return EXIT_SUCCESS;
}

void clockSigchldHandler(int sig) {
	clock_kill_receipt = 1;
}

void reccurSigchldHandler(int sig) {
	reccur_kill_receipt = 1;
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

void addProg(char* addr, int port, char* filePath) {
	printf("add prog '%s'\n", filePath);
	replaceProg(addr, port, -1, filePath);
}

void replaceProg(char* addr, int port, int progNum, char* filePath) {
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

	if(progNum == -1){
		printf("Add Program :\n");
	} else {
		printf("Replace Program :\n");
	}

	printf("Program no. %d\nExit code: %d\nEventual error msg: \n",
		cResponse.n, cResponse.exitCode);
	
	readThenWrite(sockfd, STDOUT_FILENO);
	sclose(sockfd);
}

void execProgOnce(char* addr, int port, int progNum) {
	Request req = {-2, progNum, ""};

	int sockfd = initSocketClient(addr, port);
	swrite(sockfd, &req, sizeof(Request));
	sshutdown(sockfd, SHUT_WR);

	//read
	ExecuteResponse eResponse;
	checkNeg(
		sread(sockfd, &eResponse, sizeof(ExecuteResponse)),
		"Error reading ExecuteResponse");
	//TODO switch on eResponse.programState
	printf("Exec. program no. %d.\nTemps d'exécution: %d\nCode de sortie: %d", 
		progNum, eResponse.executionTime, eResponse.exitCode);
	readThenWrite(sockfd, STDOUT_FILENO);
	sclose(sockfd);
}

void execProgReccur(int progNum, int* pipefd) {
	printf("add prog num. %d to reccurent programs\n", progNum);
	Message msg = {ADD_RECCUR, progNum};
	swrite(pipefd[1], &msg, sizeof(Message));
}

//	=== Child functions ===
// reccurent execution child
void runReccurChild(void* arg1, void* arg2, void* arg3) {
	int* pipefd = arg1;
	char* addr = arg2;
	int* port = arg3;

	ssigaction(SIGUSR1, reccurSigusr1Handler);

	sigset_t set;
	ssigemptyset(&set);
	ssigaddset(&set, SIGUSR1);
	ssigprocmask(SIG_UNBLOCK, &set, NULL);

	int* execTable = (int*) malloc(RECCUR_TABLE_START_SIZE * sizeof(int));
	int lSize = 0;
	int pSize = RECCUR_TABLE_START_SIZE;
	
	sclose(pipefd[1]);
	pid_t pidParent = getppid();

	while(terminate_recur == 0) {
		Message msg;
		sread(pipefd[0], &msg, sizeof(Message));
		if(msg.messageType == CLOCK_TICK) {
			for(int i=0; i<lSize; i++) {
				execProgOnce(addr, *port, execTable[i]);
			}
		} else {
			while(lSize >= pSize) {
				pSize *= 2;
				if ((execTable = (int*)realloc(execTable, pSize*sizeof(int))) == NULL) {
				    perror("Allocation dynamique de execTable impossible");
				}
			}
			execTable[lSize] = msg.progNum;
			lSize++;
		}
	}
	printf("reccur killed\n");
	sclose(pipefd[0]);
	free(execTable);
	skill(pidParent, SIGCHLD);
}

void reccurSigusr1Handler(int sig) {
	terminate_recur = 1;
}

// clock child
void runClockChild(void* arg1, void* arg2) {
	printf("clock created\n");
	int* pipefd = arg1;
	int* delay = arg2;

	ssigaction(SIGUSR1, clockSigusr1Handler); 
	
	sigset_t set;
	ssigemptyset(&set);
	ssigaddset(&set, SIGUSR1);
	ssigprocmask(SIG_UNBLOCK, &set, NULL); 

	sclose(pipefd[0]);
	pid_t pidParent = getppid();
	while(terminate_clock == 0) {
		sleep(*delay);
		Message msg = {CLOCK_TICK, 0};
		swrite(pipefd[1], &msg, sizeof(Message));
	}
	printf("clock killed\n");
	sclose(pipefd[1]);
	skill(pidParent, SIGCHLD);
}

void clockSigusr1Handler(int sig) {
	terminate_clock = 1;
}