#ifndef _CONST_H_
#define _CONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1
#define MAX_PROG_NAME 255

#define PERM 0666
#define SEMA_KEY  5
#define SHAREDMEM_KEY  6

#define EXECUTION_VALUE -2
#define ADD_VALUE -1


typedef enum {
	NOT_EXIST = -2,
	NOT_COMPILE =1,
	WRONG_EXECUTION = 0,
	GOOD_EXECUTION = 1
} ProgramState;


typedef struct Programme {
	int programmeID;
	char* fichierSource;
	bool hasError;
	int nombreExcecutions;
	int tempsExcecution;
} Programm;


typedef struct {
	int firstInt;						//-1 if to add  numProg if to modify  -2 if to execute
	int secondInt;						//path length if to add/modify  numProg if to execute
	char progName[MAX_PROG_NAME+1];		//prog name if to add/modify  null to execute
} Request;


typedef struct {
	int n;
	int exitCode;
} CompilationResponse;


typedef struct {
	int n;
	ProgramState programState;
	int executionTime;
	int exitCode;
} ExecuteResponse;

#endif