CFLAGS=-std=c11 -pedantic -Wall -Wvla -Werror -D_POSIX_C_SOURCE
ALL = 

all: $(ALL)


clean: 
	rm -f *.o
	rm -f $(ALL)