/* Used by server and client */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
/* struct sockaddr_in */
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utils_v10.h"
#include "../const.h"

void requestHandler(void* arg1);
void executeProgram(int programId);
void createPath(char* path);
void fillProgram(char* path, char* content);

int main(int argc, char **argv) {

	if (argc != 2) {
		printf("Usage : %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int port = atoi(argv[1]);
	
	int sockFd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

	int newSockFd;

	while(true) {
		newSockFd = saccept(sockFd);
		fork_and_run1(requestHandler, &newSockFd);
	}
}


void requestHandler(void* arg1) {
	int* newSockFd = arg1;
	Request request;

	sread(*newSockFd, &request, sizeof(request));
	if (request.firstInt == -2)	{
		executeProgram(request.secondInt);
		return;
	}
	if (request.firstInt == -1)
		createPath(request.source);
	//TODO get content from new socket fd
	char* content;
	fillProgram(request.source, content);
	return;
}

//should return Response and output
void executeProgram(int programId) {

}


void createPath(char* path) {

}

//should return Response
void fillProgram(char* path, char* content) {

}
