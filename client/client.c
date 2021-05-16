#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
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
*PRE: 	child process are running.
*POST: 	SIGUSR1 signal are send to both children, all children process are terminated.
*/
void killChildren(int, int);

//signal handlers (parent)
void clockSIGCHLDHandler(int sig);
void reccurSIGCHLDHandler(int sig);

void runReccurChild(void* arg1, void* arg2, void* arg3); //
void runClockChild(void* arg1, void* arg2); //

/**
*PRE:	query is an array of char* (length = MAX_QUERY_ARGS)
*POST:	A line is read on stdin. query is filled with char* (each entry in the query array has to be freed)
*/
void readQuery(char**);

bool isValidQuery(char**);

bool verifyText(char*);
bool verifyInt(char*);
bool verifyChar(char*); 

// server request methods
void addProg(char*, int, char*);
void replaceProg(char*, int, int, char*);
void execProgOnce(char*, int, int);
void execProgReccur(int progNum, int* pipefd);

//signal handlers (children)
void reccurSIGUSR1Handler(int sig);
void clockSIGUSR1Handler(int sig);

//	=== main ===

/*
*Arguments : IP_address, port, delay(sec)
*/
int main(int argc, char *argv[]) {
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
		const char* msg = "> ";
		nwrite(STDOUT_FILENO, msg, strlen(msg));
		readQuery(query);
		if(isValidQuery(query)){
			char queryType = query[0][0];
			int progNum;
			char* filePath;
			switch(queryType) {
				case 'q':
					quit = true;
					break;
				case '+':
					filePath = query[1];
					printf("filepath: %s", filePath);
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
		}else {
			printf("Commande invalide!\n");
		}
		for(int i=0; i<MAX_QUERY_ARGS; i++) {
			free(query[i]);
		}
	}
	sclose(pipefd[1]);
	killChildren(clockPid, reccurPid);
	exit(EXIT_SUCCESS);
}

//	=== Parent Business functions ===

void killChildren(int clockPid, int reccurPid) {
	ssigaction(SIGCHLD, clockSIGCHLDHandler);
	skill(clockPid, SIGUSR1);
	while(clock_kill_receipt == 0) {
		// wait
	}

	ssigaction(SIGCHLD, reccurSIGCHLDHandler);
	skill(reccurPid, SIGUSR1);
	while(reccur_kill_receipt == 0) {
		// wait
	}
}

void clockSIGCHLDHandler(int sig) {
	clock_kill_receipt = 1;
}

void reccurSIGCHLDHandler(int sig) {
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

bool isValidQuery(char** query) {
	if(!verifyChar(query[0]))
		return false;
	switch(query[0][0]) {
		case 'q':
		break;
		case '+':
			if(!verifyText(query[1])) 
				return false;
		break;
		case '.':
			if(!verifyInt(query[1])) 
				return false;
			if(!verifyText(query[2])) 
				return false;
		break;
		case '*':
		case '@':
			if(!verifyInt(query[1])) 
				return false;
		break;
		default:
			return false;
	}
	return true;
}

bool verifyText(char* queryArg) {
	if(queryArg == NULL)
		return false;
	if(strlen(queryArg) == 0)
		return false;
	return true;
}

bool verifyInt(char* queryArg) {
	if(!verifyText(queryArg))
		return false;
	for(int i=0; i<strlen(queryArg); i++) {
		char c = queryArg[i];
		if(!isdigit(c))
			return false;
	}
	return true;
}

bool verifyChar(char* queryArg) {
	if(!verifyText(queryArg))
		return false;
	if(strlen(queryArg) != 1)
		return false;
}

//	=== Server Request functions ===

void addProg(char* addr, int port, char* filePath) {
	replaceProg(addr, port, -1, filePath);
}

void replaceProg(char* addr, int port, int progNum, char* filePath) {
	char* fn = "helloworld.c";
	int lenFilePath = strlen(fn);
	Request req = {
		progNum, 
		lenFilePath, 
		"helloworld.c" //TODO stub
	};
	int sockfd = initSocketClient(addr, port);
	int filefd = open(filePath, O_RDONLY, 0744);
	if(filefd < 0) {
		printf("Fichier non trouvé\n");
		return;
	}
	swrite(sockfd, &req, sizeof(Request));
	readThenWrite(filefd, sockfd);

	sshutdown(sockfd, SHUT_WR);

	CompilationResponse cResponse;
	checkNeg(
		sread(sockfd, &cResponse, sizeof(CompilationResponse)), 
		"Error reading CompilationResponse");

	if(progNum == -1){
		printf("Ajout du programme ");
	} else {
		printf("Remplacement du programme ");
	}
	printf("no. %d\nExit code: %d\nMessages d'erreur éventuels: \n",
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
	switch(eResponse.programState) {
		case NOT_EXIST:
			printf("Le programme no. %d n'existe pas!\n", progNum);
			break;
		case NOT_COMPILE:
			printf("Le programme no. %d a renvoyé une erreur à la compilation!\n", progNum);
			break;
		case WRONG_EXECUTION:
			printf("Le programme no. %d a renvoyé une erreur à l'exécution.\nTemps d'exécution: %d\nCode de sortie: %d\n", 
					progNum, eResponse.executionTime, eResponse.exitCode);
			break;
		case GOOD_EXECUTION:
			printf("Le programme no. %d s'est exécuté correctement.\nTemps d'exécution: %d\nCode de sortie: %d\n", 
				progNum, eResponse.executionTime, eResponse.exitCode);
			break;
	}
	char* stdout;
	int t = sread(sockfd, &stdout, 20);
	swrite(1, &stdout, t);
	//readThenWrite(sockfd, STDOUT_FILENO);
	/*ExecuteResponse executeResponse;
	sread(sockfd, &executeResponse, sizeof(ExecuteResponse));
	char* stdout;
	getStringFromInput(&stdout, sockfd);
	
	printf("id du programme -> %d\n", executeResponse.n);
	printf("etat du programme -> %d\n", executeResponse.programState);
	printf("temps execution -> %d\n", executeResponse.executionTime);
	printf("code de retour -> %d\n", executeResponse.exitCode);
	printf("stdout -> %s\n", stdout);*/
	sclose(sockfd);
}

void execProgReccur(int progNum, int* pipefd) {
	printf("Exécution récurente du programme no. %d\n", progNum);
	Message msg = {ADD_RECCUR, progNum};
	swrite(pipefd[1], &msg, sizeof(Message));
}

//	=== Child functions ===

// reccurent execution child
void runReccurChild(void* arg1, void* arg2, void* arg3) {
	int* pipefd = arg1;
	char* addr = arg2;
	int* port = arg3;

	int* execTable = (int*) malloc(RECCUR_TABLE_START_SIZE * sizeof(int));
	int lSize = 0;
	int pSize = RECCUR_TABLE_START_SIZE;
	
	sclose(pipefd[1]);
	pid_t pidParent = getppid();

	ssigaction(SIGUSR1, reccurSIGUSR1Handler);

	sigset_t set;
	ssigemptyset(&set);
	ssigaddset(&set, SIGUSR1);
	ssigprocmask(SIG_UNBLOCK, &set, NULL);

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
	sclose(pipefd[0]);
	free(execTable);
	skill(pidParent, SIGCHLD);
}

void reccurSIGUSR1Handler(int sig) {
	terminate_recur = 1;
}

// clock child
void runClockChild(void* arg1, void* arg2) {
	int* pipefd = arg1;
	int* delay = arg2;
	sclose(pipefd[0]);
	pid_t pidParent = getppid();

	ssigaction(SIGUSR1, clockSIGUSR1Handler); 

	sigset_t set;
	ssigemptyset(&set);
	ssigaddset(&set, SIGUSR1);
	ssigprocmask(SIG_UNBLOCK, &set, NULL); 

	while(terminate_clock == 0) {
		sleep(*delay);
		Message msg = {CLOCK_TICK, 0};
		swrite(pipefd[1], &msg, sizeof(Message));
	}
	sclose(pipefd[1]);
	skill(pidParent, SIGCHLD);
}

void clockSIGUSR1Handler(int sig) {
	terminate_clock = 1;
}