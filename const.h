#ifndef _CONST_H_
#define _CONST_H_

#define SERVER_IP 127.0.0.1
#define SERVER_PORT 5642

typedef struct Programme {
	int programmeID;
	char* fichierSource;
	bool hasError;
	int nombreExcecutions;
	int tempsExcecution;
} Programm;
typedef struct Data {
	//TODO
} Data;

#endif