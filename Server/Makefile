CFLAGS=-march=native -Wall -std=gnu99 -g -O2 -I./include -L./lib
LIBS=-lpthread -litsky

MYSERVER_SOURCE=server.c
MYSERVER_FILE=server.o

all: server

clean:
	rm lib/*.a lib/*.o $(MYSERVER_FILE)

server: $(MYSERVER_SOURCE) lib/libitsky.a include/itskylib.h lib/logger.o
	gcc $(CFLAGS) $(MYSERVER_SOURCE) $(LIBS) -o $(MYSERVER_FILE)	

lib/libitsky.a: lib/lib.o lib/transmission-protocols.o
	ar crs lib/libitsky.a lib/lib.o lib/transmission-protocols.o
	
lib/logger.o: lib/logger.c include/logger.h
	gcc -c $(CFLAGS) lib/logger.c -o lib/logger.o		

lib/lib.o: lib/lib.c
	gcc -c $(CFLAGS) lib/lib.c -o lib/lib.o	
	
lib/transmission-protocols.o: lib/transmission-protocols.c
	gcc -c $(CFLAGS) lib/transmission-protocols.c -o lib/transmission-protocols.o	