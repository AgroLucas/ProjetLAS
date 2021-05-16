#ifndef _SERVCONST_H_
#define _SERVCONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1

#define PERM 0666
#define SEMA_KEY 10
#define SHAREDMEM_KEY 8
#define NBR_PROGS 1000
#define SHAREDMEMSIZE sizeof(int) + NBR_PROGS * sizeof(Programm)
#define MAX_PROG_NAME 255

#define EXECUTION_VALUE -2
#define ADD_VALUE -1

#define NO_SEMAPHORE 1

typedef struct {
	int programmeID;
	char fichierSource[MAX_PROG_NAME+1];
	bool hasError;
	int nombreExcecutions;
	int tempsExcecution;
} Programm;

typedef struct {
	int logicalSize;
	Programm programTab[NBR_PROGS];
} SharedMemoryContent;

#endif