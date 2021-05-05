CFLAGS=-std=c11 -pedantic -Wall -Wvla -Werror -D_POSIX_C_SOURCE
ALL = serverProg

all: $(ALL)


serverProg : server.o utils_v10.o
	$(CC) $(CCFLAGS) -o serverProg server.o utils_v10.o


server.o: server/server.c const.h
	$(CC) $(CCFLAGS) -c server/server.c


utils_v10.o: utils_v10.c utils_v10.h
	$(CC) $(CCFLAGS) -c utils_v10.c

clean: 
	rm -f *.o
	rm -f $(ALL)