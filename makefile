CC=gcc
CFLAGS=-std=c11 -pedantic -Wall -Wvla -Werror -D_POSIX_C_SOURCE
ALL = server/server client/client  server/maint server/stat

all: $(ALL)

# client directory

client/client : client.o utils_v10.o
	$(CC) $(CCFLAGS) -o client/client client.o utils_v10.o

client.o : client/client.c utils_v10.h const.h client/clientConst.h
	$(CC) $(CCFLAGS) -c client/client.c

# server directory
server/server : server.o utils_v10.o
	$(CC) $(CCFLAGS) -o server/server server.o utils_v10.o

server.o: server/server.c const.h server/servConst.h
	$(CC) $(CCFLAGS) -c server/server.c

server/maint : maint.o utils_v10.o
	$(CC) $(CCFLAGS) -o server/maint maint.o utils_v10.o

maint.o : server/maint.c utils_v10.h
	$(CC) $(CCFLAGS) -c server/maint.c

server/stat : stat.o utils_v10.o
	$(CC) $(CCFLAGS) -o server/stat stat.o utils_v10.o

stat.o : server/stat.c utils_v10.h
	$(CC) $(CCFLAGS) -c server/stat.c	

# utils
utils_v10.o: utils_v10.c utils_v10.h
	$(CC) $(CCFLAGS) -c utils_v10.c

clean: 
	rm -f *.o
	rm -f $(ALL)