#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "utils_v10.h"

#define MAX_QUERY_ARGS 3
#define MAX_QUERY_ARG_LENGHT 128
#define BUFF_SIZE 3*MAX_QUERY_ARG_LENGHT

/**
*PRE:	Child process "clock" and "recurrent exec" exist
*POST: 	All child process are killed
*RES: 	Exit code (failure if fail to kill children)
*/
bool shutdownChildren();

/**
*PRE: 	-
*POST:	A line is read on stdin (each entry in the returned array has to be freed)
*RES:	An array of strings (that were seperated with " " in the input on stdin)
*/
char** readQuery();

/**
*PRE:	The file referenced by filePath exists
*POST:	The program has been stored on the server
*/
void addProg(char*, int, char*);

/**
*PRE:	The file referenced by filePath exists
*POST:	The program has been overwritten on the server
*/
void replaceProg(char*, int, int, char*);

/**
*POST:	The program has been executed until completion,
*		and its writes on stdout (server) has been displayed on stdout (client)
*/
void execProgOnce(char*, int, int);

/**
*PRE:	Child process "clock" and "recurrent exec" exist
*POST:	Program array updated in "recurrent exec" child
*/
void execProgReccur(int);

//	=== main ===

int main(int argc, char const *argv[])
{
	if(argc != 4) {
		perror("Nombre d'arguments invalide.");
		exit(EXIT_FAILURE);
	}
	char* addr = argv[1];
	int port = atoi(argv[2]);
	int delay = atoi(argv[3]);

	//TODO forks

	bool quit = false;
	while(!quit) {
		char** query = readQuery();
		char queryType = query[0][0];
		switch(queryType) {
			case 'q':
				quit = true;
				break;
			case '+':
				char* filePath = query[1];
				addProg(addr, port, filePath);
				break;
			case '.':
				int progNum = atoi(query[1]);
				char* filePath = query[2];
				replaceProg(addr, port, progNum, filePath);
				break;
			case '*':
				int progNum = atoi(query[1]);
				execProgReccur(progNum);
				break;
			case '@':
				int progNum = atoi(query[1]);
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
	//TODO
	return EXIT_SUCCESS;
}

char** readQuery() {
	char* res[MAX_QUERY_ARGS];

	char buffRd[BUFF_SIZE];

	int nRd = sread(STDIN_FILENO, bufRd, BUFF_SIZE);
	checkNeg(nRd, "erreur read stdin");

	char* token = strtok(buffRd, " ");
	for(int i=0; i<MAX_QUERY_ARGS; i++) {
		//malloc
		char* arg = (char*) malloc(MAX_QUERY_ARG_LENGHT * sizeof(char));
		strcpy(arg, token);
		res[i] = arg;
		token = strtok(NULL, " ");
	}
	return res;
}

void addProg(char* addr, int port, char* filePath) {
	printf("add prog %s\n", filePath);
	//TODO
}

void replaceProg(char* addr, int port, int progNum, char* filePath) {
	printf("replace prog num. %d with %s\n", progNum, filePath);
	//TODO
}

void execProgOnce(char* addr, int port, int progNum) {
	printf("execute prog num. %d\n", progNum);
	//TODO
}

void execProgReccur(int progNum) {
	printf("add prog num. %d to reccurent programs\n", progNum);
	//TODO
}
