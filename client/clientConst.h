#ifndef _CLIENTCONST_H_
#define _CLIENTCONST_H_

#define MAX_QUERY_ARGS 3
#define MAX_QUERY_ARG_LENGHT 128
#define QUERY_BUFF_SIZE 3*MAX_QUERY_ARG_LENGHT
#define RECCUR_TABLE_START_SIZE 10

typedef enum {ADD_RECCUR, CLOCK_TICK} MessageType;

typedef struct {
	MessageType messageType;
	int progNum;
} Message;

#endif