CC=gcc
CFLAGS=-std=c11 -pedantic -Wall -Wvla -Werror -D_POSIX_C_SOURCE
ALL = server/server client/client

all: $(ALL)

client/client : client.o utils_v10.o
	$(CC) $(CCFLAGS) -o client/client client.o utils_v10.o

client.o : client/client.c utils_v10.h
	$(CC) $(CCFLAGS) -c client/client.c

server/server : server.o utils_v10.o
	$(CC) $(CCFLAGS) -o server/server server.o utils_v10.o


server.o: server/server.c const.h
	$(CC) $(CCFLAGS) -c server/server.c

utils_v10.o: utils_v10.c utils_v10.h
	$(CC) $(CCFLAGS) -c utils_v10.c

clean: 
	rm -f *.o
	rm -f $(ALL)