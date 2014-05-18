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

void usage(char *argv0, char *msg) {
	error("%s\n\n", msg);
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
		debug("found %d arguments\n", argc - 1);
		usage(argv0, "wrong number of arguments");
	}

	debug("Read port number");
	int port_number = atoi(argv[1]);

	int server_socket; /* Socket descriptor for server */
	int client_socket; /* Socket descriptor for client */
	struct sockaddr_in server_address; /* Local address */
	struct sockaddr_in client_address; /* Client address */
	unsigned int client_address_len; /* Length of client address data structure */

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
		/* Set the size of the in-out parameter */
		client_address_len = sizeof(client_address);

		/* Wait for a client to connect */
		client_socket = accept(server_socket,
				(struct sockaddr *) &client_address, &client_address_len);
		handle_error(client_socket, "accept() failed", PROCESS_EXIT);

		/* client_socket is connected to a client! */
		printf("Client connected\n");

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct thread_arg *thread_data = (struct thread_arg *) malloc(
				sizeof(struct thread_arg));

		thread_data->client_socket = client_socket;
		thread_data->thread_idx = idx;

		/* create thread: */
		if (pthread_create(thread, NULL, (void*) thread_run,
				(void*) thread_data) != 0) {
			fprintf(stderr, "pthread_create failed.\n");
		} else {
			fprintf(stdout, "pthread_create success.\n");
			pthread_detach(*thread);
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

	info("Select strategy");

	char* eom = "THE END";
	int len = strlen(eom);
	write_string(client_socket, eom, len);

	write_eot(client_socket);
	printf("Connection will be closed in one second\n");
	sleep(1);
	close(client_socket); /* Close client socket */

	return (void *) NULL;
}
