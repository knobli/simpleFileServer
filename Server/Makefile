CFLAGS=-march=native -Wall -std=gnu99 -O2 -I./include -L./lib
TEST_FLAGS=-g -rdynamic
LIBS=-lpthread -lfileserver
TEST_LIBS=-lcunit

MYSERVER_SOURCE=src/server.c
MYSERVER_FILE=server.o

TESTER_SOURCE=test/tester.c
TESTER_FILE=test/tester.o

CONCURRENT_SOURCE=test/concurrent-load-test.c
CONCURRENT_FILE=test/concurrent-load-test.o

all: server

clean:
	rm -fv lib/*.o
	rm -fv lib/*.a
	rm -fv src/*.o
	rm -fv test/*.o

server: $(MYSERVER_SOURCE) lib/libfileserver.a include/serverlib.h
	gcc $(CFLAGS) $(MYSERVER_SOURCE) $(LIBS) -o $(MYSERVER_FILE)
	
lib/libfileserver.a: lib/serverlib.o lib/transmission-protocols.o lib/logger.o lib/file-linked-list.o lib/thread-linked-list.o lib/regex-handle.o
	ar crs lib/libfileserver.a lib/serverlib.o lib/transmission-protocols.o lib/logger.o lib/file-linked-list.o lib/thread-linked-list.o lib/regex-handle.o

lib/file-linked-list.o: lib/file-linked-list.c include/file-linked-list.h
	gcc -c $(CFLAGS) lib/file-linked-list.c -o lib/file-linked-list.o
	
lib/thread-linked-list.o: lib/thread-linked-list.c include/thread-linked-list.h
	gcc -c $(CFLAGS) lib/thread-linked-list.c -o lib/thread-linked-list.o	
	
lib/logger.o: lib/logger.c include/logger.h
	gcc -c $(CFLAGS) lib/logger.c -o lib/logger.o
	
lib/regex-handle.o: lib/regex-handle.c include/regex-handle.h
	gcc -c $(CFLAGS) lib/regex-handle.c -o lib/regex-handle.o				

lib/serverlib.o: lib/serverlib.c
	gcc -c $(CFLAGS) lib/serverlib.c -o lib/serverlib.o	
	
lib/transmission-protocols.o: lib/transmission-protocols.c include/transmission-protocols.h lib/regex-handle.o
	gcc -c $(CFLAGS) lib/transmission-protocols.c -o lib/transmission-protocols.o	
	
#######
# Test part
#######	
	
test: clean lib/libfileserver.a logger-test thread-link-test file-link-test server-test message-creator load-test
	gcc $(CFLAGS) $(TEST_FLAGS) $(TESTER_SOURCE) $(LIBS) $(TEST_LIBS) -o $(TESTER_FILE)
	$(TESTER_FILE)

logger-test: test/logger-test.c lib/logger.o
	gcc -c $(CFLAGS) test/logger-test.c -o test/logger-test.o
	
thread-link-test: test/thread-linked-list-test.c lib/thread-linked-list.o
	gcc -c $(CFLAGS) test/thread-linked-list-test.c -o test/thread-linked-list-test.o
	
file-link-test: test/file-linked-list-test.c lib/file-linked-list.o
	gcc -c $(CFLAGS) test/file-linked-list-test.c -o test/file-linked-list-test.o
	
server-test: test/server-test.c lib/transmission-protocols.o
	gcc -c $(CFLAGS) test/server-test.c -o test/server-test.o
	
message-creator: test/message-creator.c test/message-creator.h lib/logger.o
	gcc -c $(CFLAGS) test/message-creator.c -o test/message-creator.o

load-test: test/load-test.c test/message-creator.o lib/logger.o
	gcc -c $(CFLAGS) test/load-test.c -o test/load-test.o

#######
# Concurrent test part
#######	
test-concurrent: clean lib/libfileserver.a message-creator
	gcc $(CFLAGS) $(TEST_FLAGS) $(CONCURRENT_SOURCE) $(LIBS) $(TEST_LIBS) -o $(CONCURRENT_FILE)
	$(CONCURRENT_FILE)								