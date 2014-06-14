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

#include "../lib/util.c"
#include <serverlib.h>
#include <file-linked-list.h>
#include <thread-linked-list.h>
#include <logger.h>
#include <transmission-protocols.h>

#define MAXPENDING 100    /* Maximum outstanding connection requests */

struct thread_arg {
	int client_socket;
	int thread_idx;
};

int server_socket; /* Socket descriptor for server */
pthread_t *cleanup_thread;
bool clean_threads = true;

void *thread_run(void *ptr);
void *thread_cleanup_run(void *ptr);

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

int main(int argc, char **argv) {
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
	while (true) { /* Run forever */
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
			add_thread_element(idx, thread);
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
		break;
	default:
		result = ANSWER_UNKOWN;
		error(deep, "Wrong action %c", action);
	}

	info(deep, "Return value: '%s' to client socket %d", result, client_socket);

	write_string(client_socket, result);

	write_eot(client_socket);
	info(deep, "Connection will be closed");
	close(client_socket); /* Close client socket */

	return (void *) NULL;
}
