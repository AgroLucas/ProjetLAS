#ifndef _SERVCONST_H_
#define _SERVCONST_H_

#include <stdbool.h>

#define SERVER_IP 127.0.0.1

#define PERM 0666
#define SEMA_KEY  5
#define SHAREDMEM_KEY  6

#define EXECUTION_VALUE -2
#define ADD_VALUE -1

#define NO_SEMAPHORE 1

typedef enum {
	NOT_EXIST = -2,
	NOT_COMPILE = 1,
	WRONG_EXECUTION = 0,
	GOOD_EXECUTION = 1
} ProgramState;

typedef struct {
	int programmeID;
	char* fichierSource;
	bool hasError;
	int nombreExcecutions;
	int tempsExcecution;
} Programm;

#endif