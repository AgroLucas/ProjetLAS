#ifndef _CONST_H_
#define _CONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1

typedef struct {
	int firstInt;
	int secondInt;
	char* source;
	char* content;	
} Request;

typedef struct {
	int id;
	char* sourceName;
	bool hasError;
	int nbrExecution;
	int nbrTotalMls;
} Program;

#endif