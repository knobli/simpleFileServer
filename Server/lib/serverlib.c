/*
 * serverlib.c
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <logger.h>
#include <serverlib.h>

#define ERROR_SIZE 4096

void exit_by_type(enum exit_type et) {
	int deep=1;
	switch (et) {
	case PROCESS_EXIT:
		exit(1);
		break;
	case THREAD_EXIT:
		pthread_exit(NULL);
		break;
	case NO_EXIT:
		debug(deep, "continuing\n");
		break;
	default:
		error(deep, "unknown exit_type=%d\n", et);
		exit(2);
		break;
	}
}

/* helper function for dealing with errors */
void handle_error_myerrno(long return_code, int myerrno, const char *msg, enum exit_type et) {
	if (return_code < 0) {
		char extra_msg[ERROR_SIZE];
		char error_msg[ERROR_SIZE];
		const char *error_str = strerror(myerrno);
		if (msg != NULL) {
			sprintf(extra_msg, "%s", msg);
		} else {
			extra_msg[0] = '\000';
		}
		sprintf(error_msg, "return_code=%ld | message=%s | error=%s", return_code, error_str, extra_msg);
		error(0, error_msg);
		exit_by_type(et);
	}
}

void handle_thread_error(int retcode, const char *msg, enum exit_type et) {
	if (retcode != 0) {
		handle_error_myerrno(-abs(retcode), retcode, msg, et);
	}
}

/* helper function for dealing with errors */
void handle_error(long return_code, const char *msg, enum exit_type et) {
	int myerrno = errno;
	handle_error_myerrno(return_code, myerrno, msg, et);
}
