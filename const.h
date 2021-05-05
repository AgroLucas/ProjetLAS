#ifndef _CONST_H_
#define _CONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1
#define MAX_SOURCE 255

typedef struct Programme {
	int programmeID;
	char* fichierSource;
	bool hasError;
	int nombreExcecutions;
	int tempsExcecution;
} Programm;

typedef struct {
	int firstInt;
	int secondInt;
	char source[MAX_SOURCE+1];	
} Request;
typedef struct {
	int id;
	char* sourceName;
	bool hasError;
	int nbrExecution;
	int nbrTotalMls;
} Program;

#endif