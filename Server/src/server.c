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
#include <logger.h>
#include <transmission-protocols.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
pthread_once_t once = PTHREAD_ONCE_INIT;

struct thread_arg {
	int client_socket;
	int thread_idx;
};

struct memory_file {
	char* filename;
	int length;
	char* content;
	pthread_mutex_t link_mod_mutex;
	pthread_rwlock_t rwlock;
	struct memory_file *next;
};

struct memory_file *head = NULL;

void *thread_run(void *ptr);

struct memory_file* create_memory_file(char *filename, int length,
		char *content);
char *create_file(char *msg);
char *update_file(char *msg);
char *delete_file(char *msg);
char *read_file(char *msg);
char *list_files(char *msg);

/* The following is the size of a buffer to contain any error messages
 encountered when the regular expression is compiled. */

#define MAX_ERROR_MSG 0x1000

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
	printf("%s <port>\n\n", argv0);
	exit(1);
}

void cleanup_threads() {
	const int deep = 1;
	debug(deep, "Cleanup all threads");
}

void cleanup() {
	const int deep = 0;
	cleanup_threads();
	debug(deep, "Destroy link mod mutex");
	//pthread_mutex_destroy(&list_mod_mutex);
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
	if (argc != 2) {
		debug(deep, "found %d arguments", argc - 1);
		usage(argv0, "wrong number of arguments");
	}

	debug(deep, "Read port number");
	int port_number = atoi(argv[1]);

	head = create_memory_file("", 0, "");

	int server_socket; /* Socket descriptor for server */
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

	int idx = 0;
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
		}

		idx++;
		cleanup_threads();
	}
	/* never going to happen */
	exit(1);
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
	info(deep, "Connection will be closed in one seconds");
	close(client_socket); /* Close client socket */

	return (void *) NULL;
}

struct memory_file* search_file(char *filename, struct memory_file **prev,
		int lock) {
	const int deep = 3;
	int returnCode;
	struct memory_file *ptr = head;
	struct memory_file *last = NULL;
	struct memory_file *second_last = NULL;
	bool found = false;
	info(deep, "Searching file '%s' in the list", filename);
	while (ptr != NULL) {
		if (second_last != NULL && lock) {
			debug(deep, "Unlock second last link mod mutex on %p - search case",
					second_last);
			debug(deep, "Link mod mutex %p - search case", &second_last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex - search",
					THREAD_EXIT);
		}
		if (last != NULL && !lock) {
			debug(deep, "Unlock last link mod mutex on %p - search case", last);
			debug(deep, "Link mod mutex %p - search case", &last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex - search",
					THREAD_EXIT);
		}
		//TEST
		debug(deep, "Unlock current link mod mutex on %p - search case", ptr);
		debug(deep, "Link mod mutex %p - search case", &ptr->link_mod_mutex);
		returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not release link mod mutex - search",
				THREAD_EXIT);
		//TEST END
		debug(deep, "Lock current link mod mutex on %p - search case", ptr);
		debug(deep, "Link mod mutex %p - search case", &ptr->link_mod_mutex);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex - search",
				THREAD_EXIT);
		if (strcmp(ptr->filename, filename) == 0) {
			debug(deep, "File '%s' found!", filename);
			found = true;
			break;
		} else {
			debug(deep, "Set second_last to: %p", last);
			second_last = last;
			debug(deep, "Set last to: %p", ptr);
			last = ptr;
			debug(deep, "Set ptr to: %p", ptr->next);
			ptr = ptr->next;
		}
	}

	if (second_last != NULL && lock) {
		debug(deep, "Unlock second last link mod mutex on %p - search case",
				second_last);
		debug(deep, "Link mod mutex %p - search case", &second_last->link_mod_mutex);
		returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not release link mod mutex - search",
				THREAD_EXIT);
	}

	if (prev) {
		debug(deep, "Set previous: %p", last);
		*prev = last;
	}

	if (true == found) {
		if (ptr != NULL && !lock) {
			debug(deep, "Unlock current link mod mutex on %p - search case", ptr);
			debug(deep, "Link mod mutex %p - search case", &ptr->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex - search",
					THREAD_EXIT);
		}
		return ptr;
	} else {
		if (last != NULL && !lock) {
			debug(deep, "Unlock last link mod mutex on %p - search case test", last);
			debug(deep, "Link mod mutex %p - search case", &last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex - search",
					THREAD_EXIT);
		}
		return NULL;
	}
}
struct memory_file* create_memory_file(char *filename, int length,
		char *content) {
	const int deep = 4;
	debug(deep, "Create new memory file");
	struct memory_file *file = (struct memory_file*) malloc(
			sizeof(struct memory_file));
	if (NULL == file) {
		error(deep, "Node creation failed");
		return NULL;
	}
	file->filename = filename;
	file->content = content;
	file->length = length;
	file->next = NULL;

	int returnCode;
	debug(deep, "Init link mod mutex");
	pthread_mutex_t mutex;
	returnCode = pthread_mutex_init(&mutex, NULL);
	handle_thread_error(returnCode, "Could not init link mod mutex",
			THREAD_EXIT);
	file->link_mod_mutex = mutex;

	debug(deep, "Init rwlock mutex");
	pthread_rwlock_t rwlock;
	returnCode = pthread_rwlock_init(&rwlock, NULL);
	handle_thread_error(returnCode, "Could not init rwlock mutex", THREAD_EXIT);
	file->rwlock = rwlock;

	debug(deep, "New memory file %p created", file);
	return file;
}
struct memory_file* add_memory_file(char *filename, int length, char *content) {
	const int deep = 3;
	struct memory_file *ptr = create_memory_file(filename, length, content);
	struct memory_file *endNode = NULL;
	struct memory_file* file = search_file(filename, &endNode, TRUE);
	debug(deep, "Test add %p", file);
	if (file == NULL) {
		info(deep, "Adding file '%s' to the end of the list", filename);
		debug(deep, "Set next pointer of %p to new file %p", endNode, ptr);
		endNode->next = ptr;
	} else {
		info(deep, "File '%s' already exist", filename);
		ptr = NULL;
	}

	debug(deep, "Unlock list mod mutex on %p - create case", endNode);
	int returnCode = pthread_mutex_unlock(&endNode->link_mod_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create",
			THREAD_EXIT);
	return ptr;
}

int delete_memory_file(char* filename) {
	const int deep = 3;
	struct memory_file *prev = NULL;
	struct memory_file *del = NULL;

	info(deep, "Deleting file '%s' from list", filename);

	int returnCode;
	int rv = 0;

	del = search_file(filename, &prev, TRUE);
	if (del == NULL) {
		rv = -1;
	} else {
		prev->next = del->next;
	}
	debug(deep, "Unlock link mod mutex on %p - delete case", prev);
	returnCode = pthread_mutex_unlock(&prev->link_mod_mutex);
	handle_thread_error(returnCode, "Could not release link mod mutex - delete",
			THREAD_EXIT);
	if (rv != -1) {
		free(del);
		del = NULL;
		rv = 0;
	}

	return rv;
}

int list_memory_file(char *file_list) {
	const int deep = 3;
	struct memory_file *ptr = head;
	int returnCode;
	int file_counter = 0;
	char *tmp_file_list = "";
	info(deep, "Go trough files");
	debug(deep, "Test list");
	while (ptr != NULL) {
		debug(deep, "Lock link mod mutex on %p - list case", ptr);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex",
				THREAD_EXIT);

		char *filename = ptr->filename;
		debug(deep, "Found file '%s'", filename);
		if (file_counter == 0) {
			tmp_file_list = append_strings(tmp_file_list, filename);
		} else {
			tmp_file_list = append_strings(tmp_file_list, "\n");
			tmp_file_list = append_strings(tmp_file_list, filename);
		}
		ptr = ptr->next;
		debug(deep, "Release link mod mutex on %p - list case", ptr);
		returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not release link mod mutex",
				THREAD_EXIT);
		file_counter++;
	}
	strcpy(file_list, tmp_file_list);

	return file_counter;
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
	struct memory_file* file = add_memory_file(filename, length, content);
	if (file != NULL) {
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
	int returnCode;

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

	info(deep, "Update file %s", filename);
	int length = strlen(content);
	if (length != orig_length) {
		error(deep, "Message length is not correct!");
		return ANSWER_INVALID;
	}
	struct memory_file* file = search_file(filename, NULL, FALSE);
	if (file != NULL) {
		debug(deep, "Lock rw mutex on %p - update case", file);
		returnCode = pthread_rwlock_wrlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex", THREAD_EXIT);

		file->length = length;
		file->content = content;

		debug(deep, "Release rw mutex on %p - update case", file);
		returnCode = pthread_rwlock_unlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not release rw mutex",
				THREAD_EXIT);
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
	int returnCode;
	char filename[4096];

	const char *read_regex_text = "READ[[:blank:]]+([[:alnum:]]+)";
	compile_regex(&r, read_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message does not match to regex!");
		return ANSWER_UNKOWN;
	}

	info(deep, "Read file %s", filename);
	struct memory_file* file = search_file(filename, NULL, FALSE);
	if (file != NULL) {
		debug(deep, "Lock rw mutex on %p - read case", file);
		returnCode = pthread_rwlock_wrlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex - read", THREAD_EXIT);
		char* content;
		if ((content = file->content) != NULL) {
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
			return rv;
		} else {
			error(deep, "Could not read file");
			return ANSWER_INTERNAL_ERROR;
		}
		debug(deep, "Release rw mutex on %p - read case", file);
		returnCode = pthread_rwlock_unlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not release rw mutex - read",
				THREAD_EXIT);
	}
	return ANSWER_FAILED_READ;
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
