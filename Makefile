CFLAGS=-march=native -Wall -std=gnu99 -O2 -I./include -L./lib
TEST_FLAGS=-g -rdynamic
LIBS=-lpthread -lfileserver
TEST_LIBS=-lcunit

MYSERVER_SOURCE=src/server.c
MYSERVER_FILE=src/server.o

TESTER_SOURCE=tests/tester.c
TESTER_FILE=tests/tester.o

CLIENT_SOURCE=tests/client-test.c
CLIENT_FILE=tests/client-test.o

CONCURRENT_SOURCE=tests/concurrent-load-test.c
CONCURRENT_FILE=tests/concurrent-load-test.o

all: server test-client
	cp -fv $(MYSERVER_FILE) run
	cp -fv $(CLIENT_FILE) test

clean:
	rm -fv run
	rm -fv test
	rm -fv lib/*.o
	rm -fv lib/*.a
	rm -fv src/*.o
	rm -fv tests/*.o

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
# Client
#######	
test-client: lib/libfileserver.a message-creator
	gcc $(CFLAGS) $(TEST_FLAGS) $(CLIENT_SOURCE) $(LIBS) $(TEST_LIBS) -o $(CLIENT_FILE)
	
#######
# Test part
#######	
	
test: clean lib/libfileserver.a logger-test thread-link-test file-link-test server-test message-creator load-test
	gcc $(CFLAGS) $(TEST_FLAGS) $(TESTER_SOURCE) $(LIBS) $(TEST_LIBS) -o $(TESTER_FILE)
	$(TESTER_FILE)

logger-test: tests/logger-test.c lib/logger.o
	gcc -c $(CFLAGS) tests/logger-test.c -o tests/logger-test.o
	
thread-link-test: tests/thread-linked-list-test.c lib/thread-linked-list.o
	gcc -c $(CFLAGS) tests/thread-linked-list-test.c -o tests/thread-linked-list-test.o
	
file-link-test: tests/file-linked-list-test.c lib/file-linked-list.o
	gcc -c $(CFLAGS) tests/file-linked-list-test.c -o tests/file-linked-list-test.o
	
server-test: tests/server-test.c lib/transmission-protocols.o
	gcc -c $(CFLAGS) tests/server-test.c -o tests/server-test.o
	
message-creator: tests/message-creator.c tests/message-creator.h lib/logger.o
	gcc -c $(CFLAGS) tests/message-creator.c -o tests/message-creator.o

load-test: tests/load-test.c tests/message-creator.o lib/logger.o
	gcc -c $(CFLAGS) tests/load-test.c -o tests/load-test.o

#######
# Concurrent test part
#######	
test-concurrent: clean lib/libfileserver.a message-creator
	gcc $(CFLAGS) $(TEST_FLAGS) $(CONCURRENT_SOURCE) $(LIBS) $(TEST_LIBS) -o $(CONCURRENT_FILE)
	$(CONCURRENT_FILE)								