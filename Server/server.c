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

#include <serverlib.h>
#include <logger.h>
#include <transmission-protocols.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
pthread_once_t once = PTHREAD_ONCE_INIT;

pthread_key_t key;
struct thread_arg {
	int client_socket;
	int thread_idx;
};

void *thread_run(void *ptr);

void create_file(char *filename, char *content, char *result);
void update_file(char *filename, char *content, char *result);
void delete_file(char *filename, char *result);
void read_file(char *filename, char *result);
void list_files(char *result);

void usage(char *argv0, char *msg) {
	error("%s\n", msg);
	printf("Usage:\n\n");

	printf("starts the file server listening on the given port\n");
	printf("%s <port>\n\n", argv0);
	exit(1);
}

int main(int argc, char *argv[]) {
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
	/* never going to happen: */
	exit(0);
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

	char *filename = "testFile";
	char *content = "blablabla";
	char result[100];
	switch (action) {
	case COMMAND_CREATE:
		create_file(filename, content, result);
		break;
	case COMMAND_UPDATE:
		update_file(filename, content, result);
		break;
	case COMMAND_DELETE:
		delete_file(filename, result);
		break;
	case COMMAND_READ:
		read_file(filename, result);
		break;
	case COMMAND_LIST:
		list_files(result);
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

void create_file(char *filename, char *content, char *result) {
	info("Create file %s", filename);
	strcpy(result, ANSWER_SUCCESS_CREATE);
}

void update_file(char *filename, char *content, char *result) {
	info("Update file %s", filename);
	result = ANSWER_SUCCESS_UPDATE;
}

void delete_file(char *filename, char *result) {
	info("Delete file %s", filename);
	result = ANSWER_SUCCESS_DELETE;
}

void read_file(char *filename, char *result) {
	info("Read file %s", filename);
	strcpy(result, ANSWER_SUCCESS_READ);
}

void list_files(char *result) {
	info("List files");
	result = ANSWER_SUCCESS_LIST;
}
