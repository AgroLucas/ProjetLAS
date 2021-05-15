#ifndef _CONST_H_
#define _CONST_H_

#define MAX_PROG_NAME 255


typedef enum {
	NOT_EXIST = -2,
	NOT_COMPILE = -1,
	WRONG_EXECUTION = 0,
	GOOD_EXECUTION = 1
} ProgramState;

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


