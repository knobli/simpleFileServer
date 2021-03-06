/*
 * serverlib.h
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#ifndef _SERVER_LIB
#define _SERVER_LIB

#include <time.h>
#include <fcntl.h>

#define MAX_BLOCK_COUNT 100000
#define MAX_BLOCK_SIZE  100000

typedef char *char_ptr;

enum exit_type {
	PROCESS_EXIT, THREAD_EXIT, NO_EXIT
};

enum file_type {
	NOT_EXISTENT, DIRECTORY, REGULAR_FILE, OTHER
};

void exit_by_type(enum exit_type et);

void handle_thread_error(int retcode, const char *msg, enum exit_type et);

/* helper function for dealing with errors */
void handle_error(long return_code, const char *msg, enum exit_type et);

void handle_error_myerrno(long return_code, int myerrno, const char *msg, enum exit_type et);

#endif

