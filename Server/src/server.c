/*
 * server.c
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

//#include <assert.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <regex.h>

#include "../lib/util.c"
#include <serverlib.h>
#include <file-linked-list.h>
#include <thread-linked-list.h>
#include <logger.h>
#include <transmission-protocols.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
/* The following is the size of a buffer to contain any error messages
 encountered when the regular expression is compiled. */

#define MAX_ERROR_MSG 0x1000

struct thread_arg {
	int client_socket;
	int thread_idx;
};

int server_socket; /* Socket descriptor for server */
pthread_t *cleanup_thread;
bool clean_threads = true;

void *thread_run(void *ptr);
void *thread_cleanup_run(void *ptr);

char *create_file(char *msg);
char *update_file(char *msg);
char *delete_file(char *msg);
char *read_file(char *msg);
char *list_files(char *msg);

/**
 * Compile the regular expression described by "regex_text" into "r".
 */
static int compile_regex(regex_t * r, const char * regex_text) {
	const int deep = 3;
	int status = regcomp(r, regex_text, REG_EXTENDED | REG_NEWLINE);
	if (status != 0) {
		char error_message[MAX_ERROR_MSG];
		regerror(status, r, error_message, MAX_ERROR_MSG);
		error(deep, "Regex error compiling '%s': %s", regex_text,
				error_message);
		return 1;
	}
	return 0;

}

/**
 * Match the string in "to_match" against the compiled regular expression in "r".
 */
static int match_regex(regex_t * r, const char * to_match, char *filename,
		char *length, char *content) {
	const int deep = 3;
	/* "P" is a pointer into the string which points to the end of the
	 previous match. */
	const char * p = to_match;
	/* "N_matches" is the maximum number of matches allowed. */
	const int n_matches = 10;
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];
	debug(deep, "Catch the matches from %s", to_match);
	int matches = 0;
	while (1) {
		debug(deep, "Matching %s", p);
		int i = 0;
		int nomatch = regexec(r, p, n_matches, m, 0);
		if (nomatch) {
			if (matches == 0) {
				error(deep, "No matches!");
				return FALSE;
			} else {
				debug(deep, "No more matches");
				return TRUE;
			}
		}
		matches++;
		for (i = 0; i < n_matches; i++) {
			int start;
			int finish;
			if (m[i].rm_so == -1) {
				break;
			}
			start = m[i].rm_so + (p - to_match);
			finish = m[i].rm_eo + (p - to_match);
			if (i == 1) {
				sprintf(filename, "%.*s", (finish - start), to_match + start);
			} else if (i == 2) {
				sprintf(length, "%.*s", (finish - start), to_match + start);
			} else if (i == 3) {
				sprintf(content, "%.*s", (finish - start), to_match + start);
			}
		}
		p += m[0].rm_eo;
	}
	return FALSE;
}

void usage(char *argv0, char *msg) {
	const int deep = 0;
	error(deep, "%s\n", msg);
	printf("Usage:\n\n");

	printf("starts the file server listening on the given port\n");
	printf("%s <port> (default: 7000)\n\n", argv0);
	exit(1);
}

void stop_cleanup_threads() {
	const int deep = 1;
	debug(deep, "Stop clean running threads");
	clean_threads = false;
	pthread_join(*cleanup_thread, NULL);
}

void cleanup() {
	const int deep = 0;
	stop_cleanup_threads();
	debug(deep, "Close server socket");
	close(server_socket);
}

void sigHandler(int signum) {
	const int deep = 0;
	if (signum == SIGINT || signum == SIGUSR1) {
		info(deep, "Server will be stopped");
		cleanup();
		info(deep, "Server stopped");
		exit(0);
	}
}

int main(int argc, char *argv[]) {
	install_segfault_handler();
	const int deep = 0;
	signal(SIGINT, sigHandler);
	signal(SIGUSR1, sigHandler);
	int retcode;
	pthread_t *thread;

	char *argv0 = argv[0];
	if (argc > 2) {
		debug(deep, "found %d arguments", argc - 1);
		usage(argv0, "wrong number of arguments");
	}

	int port_number;
	if (argc == 2) {
		debug(deep, "Read port number");
		port_number = atoi(argv[1]);
	} else {
		port_number = 7000;
	}

	init_linked_list();
	init_thread_linked_list();

	/* create cleanup thread: */
	cleanup_thread = (pthread_t *) malloc(sizeof(pthread_t));
	if (pthread_create(cleanup_thread, NULL, (void*) thread_cleanup_run, NULL) != 0) {
		error(deep, "pthread_create for cleanup thread failed");
	} else {
		debug(deep, "pthread_create for cleanup thread success");
	}

	int client_socket; /* Socket descriptor for client */
	struct sockaddr_in server_address; /* Local address */
	struct sockaddr_in client_address; /* Client address */
	/* Length of client address data structure */
	/* Set the size of the in-out parameter */
	unsigned int client_address_len = sizeof(client_address);

	/* Create socket for incoming connections */
	debug(deep, "Create socket");
	server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	handle_error(server_socket, "socket() failed", PROCESS_EXIT);

	/* Construct local address structure */
	memset(&server_address, 0, sizeof(server_address)); /* Zero out structure */
	server_address.sin_family = AF_INET; /* Internet address family */
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	server_address.sin_port = htons(port_number); /* Local port */

	/* Bind to the local address */
	debug(deep, "Bind Server Socket");
	retcode = bind(server_socket, (struct sockaddr *) &server_address,
			sizeof(server_address));
	handle_error(retcode, "bind() failed", PROCESS_EXIT);

	/* Mark the socket so it will listen for incoming connections */
	debug(deep, "Listen to server socket");
	retcode = listen(server_socket, MAXPENDING);
	handle_error(retcode, "listen() failed", PROCESS_EXIT);
	info(deep, "Server started on port %i", port_number);

	int idx = 1;
	while (TRUE) { /* Run forever */
		/* Wait for a client to connect */
		client_socket = accept(server_socket,
				(struct sockaddr *) &client_address, &client_address_len);
		handle_error(client_socket, "accept() failed", PROCESS_EXIT);

		/* client_socket is connected to a client! */
		info(deep, "Client connected");

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct thread_arg *thread_data = (struct thread_arg *) malloc(
				sizeof(struct thread_arg));

		thread_data->client_socket = client_socket;
		thread_data->thread_idx = idx;

		/* create thread: */
		if (pthread_create(thread, NULL, (void*) thread_run,
				(void*) thread_data) != 0) {
			error(deep, "pthread_create failed");
		} else {
			debug(deep, "pthread_create success");
			add_thread_element(idx, *thread);
		}

		idx++;
	}
	/* never going to happen */
	exit(1);
}

void *thread_cleanup_run(void *ptr) {
	const int deep = 1;
	debug(deep, "Cleanup thread started");
	while(clean_threads){
		sleep(2);
		info(deep, "Start cleanup running threads");
		clean_up_threads();
	}
	return (void *) NULL;
}

void *thread_run(void *ptr) {
	const int deep = 1;
	debug(deep, "Thread run entered");
	struct thread_arg *arg = (struct thread_arg *) ptr;
	int client_socket = arg->client_socket;
	debug(deep, "Client socket: %d", client_socket);
	debug(deep, "Thread index: %d", arg->thread_idx);

	char *buffer_ptr;
	info(deep, "Receive message from client");
	size_t msg_length = read_and_store_string(client_socket, &buffer_ptr);
	handle_error(msg_length, "receive failed", THREAD_EXIT);

	debug(deep, "Received string: '%s'", buffer_ptr);
	info(deep, "Select strategy");
	char *actionPointer = buffer_ptr;
	unsigned char action = actionPointer[0];

	char *result = (char*) malloc(10000 * sizeof(char));
	;
	switch (action) {
	case COMMAND_CREATE:
		result = create_file(buffer_ptr);
		break;
	case COMMAND_UPDATE:
		result = update_file(buffer_ptr);
		break;
	case COMMAND_DELETE:
		result = delete_file(buffer_ptr);
		break;
	case COMMAND_READ:
		result = read_file(buffer_ptr);
		break;
	case COMMAND_LIST:
		result = list_files(buffer_ptr);
		debug(deep, "list files done");
		break;
	default:
		result = ANSWER_UNKOWN;
		error(deep, "Wrong action %c", action);
	}

	debug(deep, "Return value: %s", result);

	write_string(client_socket, result);

	write_eot(client_socket);
	info(deep, "Connection will be closed");
	close(client_socket); /* Close client socket */

	return (void *) NULL;
}

char *create_file(char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];
	char org_length_str[5];
	char content[4096];

	const char * create_regex_text =
			"CREATE[[:blank:]]+([[:alnum:]]+)[[:blank:]]+([[:digit:]]+)[[:cntrl:]]+([[:graph:]|[:blank:]]+)";
	compile_regex(&r, create_regex_text);
	int retCode = match_regex(&r, msg, filename, org_length_str, content);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message does not match to regex!");
		return ANSWER_UNKOWN;
	}
	int orig_length = atoi(org_length_str);
	debug(deep, "Filename: %s", filename);
	debug(deep, "Length: %d", orig_length);
	debug(deep, "Content: %s", content);

	info(deep, "Create file %s", filename);
	int length = strlen(content);
	if (length != orig_length) {
		error(deep, "Message length is not correct!");
		return ANSWER_INVALID;
	}

	if (add_memory_file(filename, length, content)) {
		return ANSWER_SUCCESS_CREATE;
	} else {
		error(deep, "Could not create file");
		return ANSWER_FAILED_CREATE;
	}
}

char *update_file(char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];
	char org_length_str[5];
	char content[4096];

	const char *update_regex_text =
			"UPDATE[[:blank:]]+([[:alnum:]]+)[[:blank:]]+([[:digit:]]+)[[:cntrl:]]+([[:graph:]|[:blank:]]+)";
	compile_regex(&r, update_regex_text);
	int retCode = match_regex(&r, msg, filename, org_length_str, content);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message does not match to regex!");
		return ANSWER_UNKOWN;
	}
	int orig_length = atoi(org_length_str);
	debug(deep, "Filename: %s", filename);
	debug(deep, "Length: %d", orig_length);
	debug(deep, "Content: %s", content);

	int length = strlen(content);
	if (length != orig_length) {
		error(deep, "Message length is not correct!");
		return ANSWER_INVALID;
	}

	if (update_memory_file(filename, length, content)) {
		return ANSWER_SUCCESS_UPDATE;
	}
	return ANSWER_FAILED_UPDATE;
}

char *delete_file(char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];

	const char *delete_regex_text = "DELETE[[:blank:]]+([[:alnum:]]+)";
	compile_regex(&r, delete_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message does not match to regex!");
		return ANSWER_UNKOWN;
	}

	info(deep, "Delete file %s", filename);
	if (delete_memory_file(filename) == 0) {
		return ANSWER_SUCCESS_DELETE;
	}
	return ANSWER_FAILED_DELETE;
}

char *read_file(char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];

	const char *read_regex_text = "READ[[:blank:]]+([[:alnum:]]+)";
	compile_regex(&r, read_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message does not match to regex!");
		return ANSWER_UNKOWN;
	}

	char* returnValue;
	char content[0];
	if (read_memory_file(filename, content)) {
		if (content != NULL) {
			debug(deep, "Content of file: %s", content);
			int length = strlen(content);
			char len_string[15];
			sprintf(len_string, "%d", length);
			char *rv = append_strings(ANSWER_SUCCESS_READ, filename);
			rv = append_strings(rv, " ");
			rv = append_strings(rv, len_string);
			rv = append_strings(rv, "\n");
			rv = append_strings(rv, content);
			rv = append_strings(rv, "\n");
			returnValue = rv;
		} else {
			error(deep, "Could not read file");
			returnValue = ANSWER_INTERNAL_ERROR;
		}
	} else {
		returnValue = ANSWER_FAILED_READ;
	}
	return returnValue;
}

char *list_files(char *msg) {
	const int deep = 2;
	regex_t r;

	const char *list_regex_text = "LIST";
	compile_regex(&r, list_regex_text);
	int retCode = match_regex(&r, msg, NULL, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message does not match to regex!");
		return ANSWER_UNKOWN;
	}

	info(deep, "List files");
	char file_list[0];
	int file_counter = list_memory_file(file_list);

	debug(deep, "Output from list method: %s", file_list);
	char str[15];
	sprintf(str, "%d", file_counter);
	char *rv = append_strings(ANSWER_SUCCESS_LIST, str);
	rv = append_strings(rv, "\n");
	rv = append_strings(rv, file_list);
	rv = append_strings(rv, "\n");
	debug(deep, "list files nearly done");
	return rv;
}
