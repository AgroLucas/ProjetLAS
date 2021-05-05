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

int nbrProgs = 0;

int main(int argc, char **argv) {

	if (argc != 2) {
		printf("Usage : %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int port = atoi(argv[1]);
	
	int sockfd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

	int newsockfd;
	Request request;

	while(true) {
		if (nbrProgs >= MAX_RUNNING_PROGS) {
			sleep(1);
			continue;
		}
		newsockfd = saccept(sockfd);
		nbrProgs++;
		sread(newsockfd, &request, sizeof(request));
		if (request.firstInt == -1)	{
			//TODO ajouter/remplacer progs
		} else {
			//TODO executer
		}
	}

} 	