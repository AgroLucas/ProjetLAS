#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../utils_v10.h"
#include "../const.h"

#define MAX_QUERY_ARGS 3
#define MAX_QUERY_ARG_LENGHT 128
#define BUFF_SIZE 3*MAX_QUERY_ARG_LENGHT
#define SOCK_BUFF_SIZE 256

/*
*PRE: 	Children process are running.
*POST: 	SIGUSR1 signal are send to both children, shuting them down.
*RET:	Boolean (true = successful shutdown)
*/
bool shutdownChildren();

/**
*PRE:	query is an array of char* (length = MAX_QUERY_ARGS)
*POST:	A line is read on stdin. query is filled with char* (each entry in the query array has to be freed)
*/
void readQuery(char**);

void addProg(const char*, int, char*);
void replaceProg(const char*, int, int, char*);
void execProgOnce(const char*, int, int);
void execProgReccur(int);

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

	//TODO forks

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
	exit(shutdownChildren());
}

//	=== Business functions ===

bool shutdownChildren() {
	printf("shutdownChildren\n");
	return EXIT_SUCCESS;
}

void readQuery(char** query) {
	char buffRd[BUFF_SIZE];
	if (sread(STDIN_FILENO, buffRd, BUFF_SIZE) == -1) {
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

void addProg(const char* addr, int port, char* filePath) {
	printf("add prog '%s'\n", filePath);
	//TODO
}

void replaceProg(const char* addr, int port, int progNum, char* filePath) {
	printf("replace prog num. %d with '%s'\n", progNum, filePath);
	//TODO
}

void execProgOnce(const char* addr, int port, int progNum) {
	printf("execute prog num. %d\n", progNum);
	Request req = {-2, progNum, NULL};

	int sockfd = initSocketClient(addr, port);
	swrite(sockfd, &req, sizeof(Request));
	shutdown(sockfd, SHUT_WR);

	//read
	char buffRd[SOCK_BUFF_SIZE];

	int nbRd;
	do {
		nbRd = sread(sockfd, buffRd, SOCK_BUFF_SIZE);
		checkCond(
			swrite(STDOUT_FILENO, buffRd, nbRd) != SOCK_BUFF_SIZE, 
			"Error writing on STDOUT");	
	}while(nbRd == SOCK_BUFF_SIZE);
}

void execProgReccur(int progNum) {
	printf("add prog num. %d to reccurent programs\n", progNum);
	//TODO
}
