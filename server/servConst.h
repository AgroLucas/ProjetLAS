#ifndef _SERVCONST_H_
#define _SERVCONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1

#define PERM 0666
#define SEMA_KEY  5
#define SHAREDMEM_KEY  6
#define NBR_PROGS 1000
//sharedMem 2 en 1 --> indice du tableau + structure associée
// aka int + taille de la structure * nbre de structures existantes
#define SHAREDMEMSIZE sizeof(int) + NBR_PROGS * sizeof(Programm)

#define EXECUTION_VALUE -2
#define ADD_VALUE -1

#define NO_SEMAPHORE 1

typedef struct {
	int programmeID;
	char* fichierSource;
	bool hasError;
	int nombreExcecutions;
	int tempsExcecution;
} Programm;

typedef struct {
	int logicalSize;
	Programm programTab[NBR_PROGS];
} SharedMemoryContent;

#endif