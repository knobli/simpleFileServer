DEFAULT_DIR=Server/
CFLAGS=-march=native -Wall -std=gnu99 -O2 -I./$(DEFAULT_DIR)include -L./$(DEFAULT_DIR)lib
TEST_FLAGS=-g -rdynamic
LIBS=-lpthread -lfileserver
TEST_LIBS=-lcunit

MYSERVER_SOURCE=$(DEFAULT_DIR)src/server.c
MYSERVER_FILE=$(DEFAULT_DIR)src/server.o

TESTER_SOURCE=$(DEFAULT_DIR)tests/tester.c
TESTER_FILE=$(DEFAULT_DIR)tests/tester.o

CLIENT_SOURCE=$(DEFAULT_DIR)tests/client-test.c
CLIENT_FILE=$(DEFAULT_DIR)tests/client-test.o

CONCURRENT_SOURCE=$(DEFAULT_DIR)tests/concurrent-load-test.c
CONCURRENT_FILE=$(DEFAULT_DIR)tests/concurrent-load-test.o

all: server test-client
	cp -fv $(MYSERVER_FILE) run
	cp -fv $(CLIENT_FILE) test

clean:
	rm -fv $(DEFAULT_DIR)run
	rm -fv $(DEFAULT_DIR)test
	rm -fv $(DEFAULT_DIR)lib/*.o
	rm -fv $(DEFAULT_DIR)lib/*.a
	rm -fv $(DEFAULT_DIR)src/*.o
	rm -fv $(DEFAULT_DIR)tests/*.o

server: $(MYSERVER_SOURCE) libfileserver $(DEFAULT_DIR)include/serverlib.h
	gcc $(CFLAGS) $(MYSERVER_SOURCE) $(LIBS) -o $(MYSERVER_FILE)
	
libfileserver: serverlib transmission-protocols logger file-linked-list thread-linked-list regex-handle
	ar crs $(DEFAULT_DIR)lib/libfileserver.a $(DEFAULT_DIR)lib/serverlib.o $(DEFAULT_DIR)lib/transmission-protocols.o $(DEFAULT_DIR)lib/logger.o $(DEFAULT_DIR)lib/file-linked-list.o $(DEFAULT_DIR)lib/thread-linked-list.o $(DEFAULT_DIR)lib/regex-handle.o

file-linked-list: $(DEFAULT_DIR)lib/file-linked-list.c $(DEFAULT_DIR)include/file-linked-list.h
	gcc -c $(CFLAGS) $(DEFAULT_DIR)lib/file-linked-list.c -o $(DEFAULT_DIR)lib/file-linked-list.o
	
thread-linked-list: $(DEFAULT_DIR)lib/thread-linked-list.c $(DEFAULT_DIR)include/thread-linked-list.h
	gcc -c $(CFLAGS) $(DEFAULT_DIR)lib/thread-linked-list.c -o $(DEFAULT_DIR)lib/thread-linked-list.o	
	
logger: $(DEFAULT_DIR)lib/logger.c $(DEFAULT_DIR)include/logger.h
	gcc -c $(CFLAGS) $(DEFAULT_DIR)lib/logger.c -o $(DEFAULT_DIR)lib/logger.o
	
regex-handle: $(DEFAULT_DIR)lib/regex-handle.c $(DEFAULT_DIR)include/regex-handle.h
	gcc -c $(CFLAGS) $(DEFAULT_DIR)lib/regex-handle.c -o $(DEFAULT_DIR)lib/regex-handle.o				

serverlib: $(DEFAULT_DIR)lib/serverlib.c
	gcc -c $(CFLAGS) $(DEFAULT_DIR)lib/serverlib.c -o $(DEFAULT_DIR)lib/serverlib.o	
	
transmission-protocols: $(DEFAULT_DIR)lib/transmission-protocols.c $(DEFAULT_DIR)include/transmission-protocols.h regex-handle
	gcc -c $(CFLAGS) $(DEFAULT_DIR)lib/transmission-protocols.c -o $(DEFAULT_DIR)lib/transmission-protocols.o	

#######
# Client
#######	
test-client: libfileserver message-creator
	gcc $(CFLAGS) $(TEST_FLAGS) $(CLIENT_SOURCE) $(LIBS) $(TEST_LIBS) -o $(CLIENT_FILE)
	
#######
# Test part
#######	
	
test: clean libfileserver logger-test thread-link-test file-link-test server-test message-creator load-test
	gcc $(CFLAGS) $(TEST_FLAGS) $(TESTER_SOURCE) $(LIBS) $(TEST_LIBS) -o $(TESTER_FILE)
	$(TESTER_FILE)

logger-test: $(DEFAULT_DIR)tests/logger-test.c logger
	gcc -c $(CFLAGS) $(DEFAULT_DIR)tests/logger-test.c -o $(DEFAULT_DIR)tests/logger-test.o
	
thread-link-test: tests/thread-linked-list-test.c thread-linked-list
	gcc -c $(CFLAGS) $(DEFAULT_DIR)tests/thread-linked-list-test.c -o $(DEFAULT_DIR)tests/thread-linked-list-test.o
	
file-link-test: $(DEFAULT_DIR)tests/file-linked-list-test.c file-linked-list
	gcc -c $(CFLAGS) $(DEFAULT_DIR)tests/file-linked-list-test.c -o $(DEFAULT_DIR)tests/file-linked-list-test.o
	
server-test: $(DEFAULT_DIR)tests/server-test.c transmission-protocols
	gcc -c $(CFLAGS) $(DEFAULT_DIR)tests/server-test.c -o $(DEFAULT_DIR)tests/server-test.o
	
message-creator: $(DEFAULT_DIR)tests/message-creator.c $(DEFAULT_DIR)tests/message-creator.h logger
	gcc -c $(CFLAGS) $(DEFAULT_DIR)tests/message-creator.c -o $(DEFAULT_DIR)tests/message-creator.o

load-test: $(DEFAULT_DIR)tests/load-test.c message-creator logger
	gcc -c $(CFLAGS) $(DEFAULT_DIR)tests/load-test.c -o $(DEFAULT_DIR)tests/load-test.o

#######
# Concurrent test part
#######	
test-concurrent: clean libfileserver message-creator
	gcc $(CFLAGS) $(TEST_FLAGS) $(CONCURRENT_SOURCE) $(LIBS) $(TEST_LIBS) -o $(CONCURRENT_FILE)
	$(CONCURRENT_FILE)								
