CFLAGS=-march=native -Wall -std=gnu99 -g -O2 -I./include -L./lib
LIBS=-lpthread -lfileserver

MYSERVER_SOURCE=src/server.c
MYSERVER_FILE=server.o

all: server

clean:
	rm lib/*.a lib/*.o $(MYSERVER_FILE)

server: $(MYSERVER_SOURCE) lib/libfileserver.a include/serverlib.h
	gcc $(CFLAGS) $(MYSERVER_SOURCE) $(LIBS) -o $(MYSERVER_FILE)	

lib/libfileserver.a: lib/serverlib.o lib/transmission-protocols.o lib/logger.o
	ar crs lib/libfileserver.a lib/serverlib.o lib/transmission-protocols.o lib/logger.o
	
lib/logger.o: lib/logger.c include/logger.h
	gcc -c $(CFLAGS) lib/logger.c -o lib/logger.o		

lib/serverlib.o: lib/serverlib.c
	gcc -c $(CFLAGS) lib/serverlib.c -o lib/serverlib.o	
	
lib/transmission-protocols.o: lib/transmission-protocols.c
	gcc -c $(CFLAGS) lib/transmission-protocols.c -o lib/transmission-protocols.o	