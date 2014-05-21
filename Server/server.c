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

#include <serverlib.h>
#include <logger.h>
#include <transmission-protocols.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
pthread_once_t once = PTHREAD_ONCE_INIT;

pthread_key_t key;
pthread_mutex_t list_mod_mutex;
struct thread_arg {
	int client_socket;
	int thread_idx;
};

struct memory_file {
	char* filename;
	int length;
	char* content;
	struct memory_file *next;
};

struct memory_file *head = NULL;

void *thread_run(void *ptr);

void create_file(char *msg, char *result);
void update_file(char *msg, char *result);
void delete_file(char *msg, char *result);
void read_file(char *msg, char *result);
void list_files(char *msg, char *result);

/* The following is the size of a buffer to contain any error messages
 encountered when the regular expression is compiled. */

#define MAX_ERROR_MSG 0x1000

/**
 * Compile the regular expression described by "regex_text" into "r".
 */
static int compile_regex(regex_t * r, const char * regex_text) {
	int status = regcomp(r, regex_text, REG_EXTENDED | REG_NEWLINE);
	if (status != 0) {
		char error_message[MAX_ERROR_MSG];
		regerror(status, r, error_message, MAX_ERROR_MSG);
		error("Regex error compiling '%s': %s", regex_text, error_message);
		return 1;
	}
	return 0;

}

/**
 * Match the string in "to_match" against the compiled regular expression in "r".
 */
static int match_regex(regex_t * r, const char * to_match, char *filename,
		char *length, char *content) {
	/* "P" is a pointer into the string which points to the end of the
	 previous match. */
	const char * p = to_match;
	/* "N_matches" is the maximum number of matches allowed. */
	const int n_matches = 10;
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];
	debug("Catch the matches from %s", to_match);
	int matches = 0;
	while (1) {
		debug("Matching %s", p);
		int i = 0;
		int nomatch = regexec(r, p, n_matches, m, 0);
		if (nomatch) {
			if (matches == 0) {
				error("No matches!");
				return FALSE;
			} else {
				debug("No more matches");
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
	error("%s\n", msg);
	printf("Usage:\n\n");

	printf("starts the file server listening on the given port\n");
	printf("%s <port>\n\n", argv0);
	exit(1);
}

void cleanup() {
	//TODO: wait for threads

	debug("Destroy list mod mutex");
	pthread_mutex_destroy(&list_mod_mutex);
}

void sigHandler(int signum) {
	if (signum == SIGINT || signum == SIGUSR1) {
		info("Server will be stopped");
		cleanup();
		info("Server stopped");
		exit(0);
	}
}

int main(int argc, char *argv[]) {
	signal(SIGINT, sigHandler);
	signal(SIGUSR1, sigHandler);
	int retcode;
	pthread_t *thread;

	retcode = pthread_key_create(&key, NULL);

	char *argv0 = argv[0];
	if (argc != 2) {
		debug("found %d arguments", argc - 1);
		usage(argv0, "wrong number of arguments");
	}

	debug("Read port number");
	int port_number = atoi(argv[1]);

	int server_socket; /* Socket descriptor for server */
	int client_socket; /* Socket descriptor for client */
	struct sockaddr_in server_address; /* Local address */
	struct sockaddr_in client_address; /* Client address */
	/* Length of client address data structure */
	/* Set the size of the in-out parameter */
	unsigned int client_address_len = sizeof(client_address);

	/* Create socket for incoming connections */
	debug("Create socket");
	server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	handle_error(server_socket, "socket() failed", PROCESS_EXIT);

	/* Construct local address structure */
	memset(&server_address, 0, sizeof(server_address)); /* Zero out structure */
	server_address.sin_family = AF_INET; /* Internet address family */
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	server_address.sin_port = htons(port_number); /* Local port */

	/* Bind to the local address */
	debug("Bind Server Socket");
	retcode = bind(server_socket, (struct sockaddr *) &server_address,
			sizeof(server_address));
	handle_error(retcode, "bind() failed", PROCESS_EXIT);

	/* Mark the socket so it will listen for incoming connections */
	debug("Listen to server socket");
	retcode = listen(server_socket, MAXPENDING);
	handle_error(retcode, "listen() failed", PROCESS_EXIT);
	info("Server started on port %i", port_number);

	debug("Init mutex");
	pthread_mutex_init(&list_mod_mutex, NULL);

	int idx = 0;
	while (TRUE) { /* Run forever */
		/* Wait for a client to connect */
		client_socket = accept(server_socket,
				(struct sockaddr *) &client_address, &client_address_len);
		handle_error(client_socket, "accept() failed", PROCESS_EXIT);

		/* client_socket is connected to a client! */
		info("Client connected");

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct thread_arg *thread_data = (struct thread_arg *) malloc(
				sizeof(struct thread_arg));

		thread_data->client_socket = client_socket;
		thread_data->thread_idx = idx;

		/* create thread: */
		if (pthread_create(thread, NULL, (void*) thread_run,
				(void*) thread_data) != 0) {
			error("pthread_create failed");
		} else {
			info("pthread_create success");
		}

		idx++;
	}
	/* never going to happen */
	exit(1);
}

void *thread_run(void *ptr) {
	debug("Thread run entered");
	struct thread_arg *arg = (struct thread_arg *) ptr;
	int client_socket = arg->client_socket;
	debug("Client socket: %d", client_socket);
	debug("Thread index: %d", arg->thread_idx);

	char *buffer_ptr[0];
	info("Receive message from client");
	size_t msg_length = read_and_store_string(client_socket, buffer_ptr);
	handle_error(msg_length, "receive failed", THREAD_EXIT);

	debug("Received string: '%s'", *buffer_ptr);
	info("Select strategy");
	char *actionPointer = buffer_ptr[0];
	unsigned char action = actionPointer[0];

	char result[100];
	switch (action) {
	case COMMAND_CREATE:
		create_file(*buffer_ptr, result);
		break;
	case COMMAND_UPDATE:
		update_file(*buffer_ptr, result);
		break;
	case COMMAND_DELETE:
		delete_file(*buffer_ptr, result);
		break;
	case COMMAND_READ:
		read_file(*buffer_ptr, result);
		break;
	case COMMAND_LIST:
		list_files(*buffer_ptr, result);
		break;
	default:
		error("Wrong action %s", action);
	}

	debug("Return value: %s", result);

	write_string(client_socket, result);

	write_eot(client_socket);
	info("Connection will be closed in one seconds");
	sleep(1);
	close(client_socket); /* Close client socket */

	return (void *) NULL;
}

struct memory_file* search_file(char *filename, struct memory_file **prev) {
	struct memory_file *ptr = head;
	struct memory_file *tmp = NULL;
	bool found = false;
	info("Searching file '%s' in the list", filename);
	while (ptr != NULL) {
		if (strcmp(ptr->filename, filename) == 0) {
			found = true;
			break;
		} else {
			tmp = ptr;
			ptr = ptr->next;
		}
	}
	if (true == found) {
		if (prev)
			*prev = tmp;
		return ptr;
	} else {
		return NULL;
	}
}

struct memory_file* add_memory_file(char *filename, int length, char *content) {

	info("Adding file '%s' to beginning of list", filename);
	struct memory_file *ptr = (struct memory_file*) malloc(
			sizeof(struct memory_file));
	if (NULL == ptr) {
		error("Node creation failed");
		return NULL;
	}
	ptr->filename = filename;
	ptr->content = content;
	ptr->length = length;

	int returnCode;
	debug("Lock list mod mutex - create case");
	returnCode = pthread_mutex_lock(&list_mod_mutex);
	handle_thread_error(returnCode, "Could not lock list mod mutex",
			THREAD_EXIT);

	ptr->next = head;
	head = ptr;

	debug("Unlock list mod mutex - create case");
	returnCode = pthread_mutex_unlock(&list_mod_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex",
			THREAD_EXIT);
	return ptr;
}

int delete_memory_file(char* filename) {
	struct memory_file *prev = NULL;
	struct memory_file *del = NULL;

	info("Deleting file '%s' from list", filename);

	int returnCode;
	int rv = 0;
	debug("Lock list mod mutex - delete case");
	returnCode = pthread_mutex_lock(&list_mod_mutex);
	handle_thread_error(returnCode, "Could not lock list mod mutex",
			THREAD_EXIT);
	del = search_file(filename, &prev);
	if (del == NULL) {
		rv = -1;
	} else {
		if (prev != NULL)
			prev->next = del->next;

		if (del == head) {
			head = del->next;
		}
	}
	debug("Unlock list mod mutex - delete case");
	returnCode = pthread_mutex_unlock(&list_mod_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex",
			THREAD_EXIT);
	if (rv != -1) {
		free(del);
		del = NULL;
		rv = 0;
	}

	return rv;
}

int list_memory_file(char *file_list) {
	struct memory_file *ptr = head;
	int file_counter = 0;
	char *tmp_file_list = "";
	info("Go trough files");
	while (ptr != NULL) {
		char *filename = ptr->filename;
		debug("Found file '%s'", filename);
		if (file_counter == 0) {
			tmp_file_list = append_strings(tmp_file_list, filename);
		} else {
			tmp_file_list = append_strings(tmp_file_list, "\n");
			tmp_file_list = append_strings(tmp_file_list, filename);
		}
		ptr = ptr->next;
		file_counter++;
	}
	strcpy(file_list, tmp_file_list);

	return file_counter;
}

void create_file(char *msg, char *result) {
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
		error("Message does not match to regex!");
		strcpy(result, ANSWER_UNKOWN);
		return;
	}
	int orig_length = atoi(org_length_str);
	debug("Test filename: %s", filename);
	debug("Test length: %d", orig_length);
	debug("Test content: %s", content);

	info("Create file %s", filename);
	int length = strlen(content);
	if (length != orig_length) {
		error("Message length is not correct!");
		strcpy(result, ANSWER_INVALID);
		return;
	}
	struct memory_file* file = search_file(filename, NULL);
	if (file == NULL) {
		file = add_memory_file(filename, length, content);
		if (file != NULL) {
			strcpy(result, ANSWER_SUCCESS_CREATE);
		} else {
			error("Could not create file");
			strcpy(result, ANSWER_FAILED_CREATE);
		}
	} else {
		strcpy(result, ANSWER_FAILED_CREATE);
	}
}

void update_file(char *msg, char *result) {
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
		error("Message does not match to regex!");
		strcpy(result, ANSWER_UNKOWN);
		return;
	}
	int orig_length = atoi(org_length_str);
	debug("Test filename: %s", filename);
	debug("Test length: %d", orig_length);
	debug("Test content: %s", content);

	info("Update file %s", filename);
	int length = strlen(content);
	if (length != orig_length) {
		error("Message length is not correct!");
		strcpy(result, ANSWER_INVALID);
		return;
	}
	struct memory_file* file = search_file(filename, NULL);
	if (file != NULL) {
		file->length = length;
		file->content = content;
		strcpy(result, ANSWER_SUCCESS_UPDATE);
	} else {
		strcpy(result, ANSWER_FAILED_UPDATE);
	}
}

void delete_file(char *msg, char *result) {
	regex_t r;
	char filename[4096];

	const char *delete_regex_text = "DELETE[[:blank:]]+([[:alnum:]]+)";
	compile_regex(&r, delete_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error("Message does not match to regex!");
		strcpy(result, ANSWER_UNKOWN);
		return;
	}

	info("Delete file %s", filename);
	if (delete_memory_file(filename) == 0) {
		strcpy(result, ANSWER_SUCCESS_DELETE);
	} else {
		strcpy(result, ANSWER_FAILED_DELETE);
	}
}

void read_file(char *msg, char *result) {
	regex_t r;
	char filename[4096];

	const char *read_regex_text = "READ[[:blank:]]+([[:alnum:]]+)";
	compile_regex(&r, read_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error("Message does not match to regex!");
		strcpy(result, ANSWER_UNKOWN);
		return;
	}

	info("Read file %s", filename);
	struct memory_file* file = search_file(filename, NULL);
	if (file != NULL) {
		char* content;
		if ((content = file->content) != NULL) {
			debug("Content of file: %s", content);
			char *rv = append_strings(ANSWER_SUCCESS_READ, filename);
			rv = append_strings(rv, "\n");
			rv = append_strings(rv, content);
			rv = append_strings(rv, "\n");
			strcpy(result, rv);
		} else {
			error("Could not read file");
			strcpy(result, ANSWER_FAILED_READ);
		}
	} else {
		strcpy(result, ANSWER_FAILED_READ);
	}
}

void list_files(char *msg, char *result) {
	regex_t r;

	const char *list_regex_text = "LIST";
	compile_regex(&r, list_regex_text);
	int retCode = match_regex(&r, msg, NULL, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error("Message does not match to regex!");
		strcpy(result, ANSWER_UNKOWN);
		return;
	}

	info("List files");
	char file_list[0];
	int file_counter = list_memory_file(file_list);
	debug("Output from list method: %s", file_list);
	char str[15];
	sprintf(str, "%d", file_counter);
	char *rv = append_strings(ANSWER_SUCCESS_LIST, str);
	rv = append_strings(rv, "\n");
	rv = append_strings(rv, file_list);
	rv = append_strings(rv, "\n");
	strcpy(result, rv);
}
