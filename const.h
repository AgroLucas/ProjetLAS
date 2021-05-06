#ifndef _CONST_H_
#define _CONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1
#define MAX_SOURCE 255


typedef struct {
	int firstInt;					//-1 if to add  numProg if to modify  -2 if to execute
	int secondInt;					//path length if to add/modify  numProg if to execute
	char source[MAX_SOURCE+1];		//path length if to add/modify  null to execute
} Request;


typedef struct {
	int n;
	int exitCode;
} ModificationResponse;


typedef struct {
	int n;
	int programState;
	int executionTime;
	int exitCode;
} ExecuteResponse;


typedef struct {
	int id;
	char* sourceName;
	bool hasError;
	int nbrExecution;
	int nbrTotalMls;
} Program;

#endif