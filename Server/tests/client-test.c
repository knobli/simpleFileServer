/*
 * client.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "../lib/util.c"
#include <serverlib.h>
#include <file-linked-list.h>
#include <thread-linked-list.h>
#include <transmission-protocols.h>
#include <logger.h>
#include "message-creator.c"

const char *filename_base_client = "file_client";
const char *file_content_client = "just a small content";
const size_t content_lengt_client = 21;
const char *file_content_up_client = "just a small content with more information";
const size_t content_up_lengt_client = 43;
const size_t max_files_to_test = 100;
const size_t max_listing_test = 10;

struct file_details {
	char *filename;
	char *content;
};

char *send_message(const char *msg) {
	int retcode;
	int sock;
	int deep = 3;
	struct sockaddr_in server_address;
	unsigned short server_port = 7000; /* 7000 is a free port */
	char *server_ip = "127.0.0.1";

	/* Create a reliable, stream socket using TCP */
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	handle_error(sock, "socket() failed", THREAD_EXIT);

	/* Construct the server address structure */
	memset(&server_address, 0, sizeof(server_address)); /* Zero out structure */
	server_address.sin_family = AF_INET; /* Internet address family */
	server_address.sin_addr.s_addr = inet_addr(server_ip); /* Server IP address */
	server_address.sin_port = htons(server_port); /* Server port: htons host to network byte order */

	/* Establish the connection to the server */
	debug(deep, "Try to connect to %s on port %d\n", server_ip, server_port);
	retcode = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
	handle_error(retcode, "connect() failed", THREAD_EXIT);

	unsigned int input_string_len = strlen(msg);

	/* Send the string to the server */
	int count = send(sock, msg, input_string_len, 0);
	if (count != input_string_len) {
		error(deep, "send() sent a different number of bytes than expected");
	}

	/* Receive the same string containing the square back from the server */
	char *response = "";
	while (true) {
		debug(deep, "Read line\n");
		char *buffer_ptr;
		size_t bytes_received = read_and_store_string(sock, &buffer_ptr);
		if (bytes_received < 0) {
			error(deep, "recv() failed or connection closed prematurely\n");
		}
		if (bytes_received == 0) {
			debug(deep, "terminated by server\n");
			break;
		}
		response = append_strings(response, buffer_ptr);
	}

	close(sock);
	return response;
}

void *create_file_run(void *ptr) {
	int deep = 2;
	struct file_details *arg = (struct file_details *) ptr;
	info(deep, "Create file: %s", arg->filename);
	char *create_msg = create_create_message(arg->filename, arg->content);
	char *response = send_message(create_msg);
	if (strcmp(response, ANSWER_SUCCESS_CREATE) != 0) {
		printf("CREATE: expected: '%s' actual: '%s'\n", ANSWER_SUCCESS_CREATE, response);
	}
	free(response);
	return (void *) NULL;
}

void create_files_by_client() {
	if (!init_linked_list()) {
		printf("could not init linked list\n");
	}
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 1000;
	for (i = 0; i < max_files_to_test; i++) {
		char *filename = create_numbered_filename(filename_base_client, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		thread_data->content = malloc(content_lengt_client);
		strncpy(thread_data->content, file_content_client, content_lengt_client);
		if (pthread_create(thread, NULL, (void*) create_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *update_file_run(void *ptr) {
	int deep = 2;
	struct file_details *arg = (struct file_details *) ptr;
	info(deep, "Update file: %s", arg->filename);
	char *update_msg = create_update_message(arg->filename, arg->content);
	char *response = send_message(update_msg);
	if (strcmp(response, ANSWER_SUCCESS_UPDATE) != 0 && strcmp(response, ANSWER_FAILED_UPDATE) != 0) {
		printf("UPDATE: expected: '%s' or '%s' actual: '%s'\n", ANSWER_SUCCESS_UPDATE, ANSWER_FAILED_UPDATE, response);
	}
	free(response);
	return (void *) NULL;
}

void update_files_by_client() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 2000;
	for (i = 0; i < max_files_to_test; i++) {
		char *filename = create_numbered_filename(filename_base_client, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		thread_data->content = malloc(content_lengt_client);
		strncpy(thread_data->content, file_content_client, content_lengt_client);
		if (pthread_create(thread, NULL, (void*) update_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *read_file_run(void *ptr) {
	int deep = 2;
	struct file_details *arg = (struct file_details *) ptr;
	info(deep, "Read file: %s", arg->filename);
	char *read_msg = create_read_message(arg->filename);
	char *response = send_message(read_msg);
	if (strncmp(response, ANSWER_SUCCESS_READ, 12) != 0 && strcmp(response, ANSWER_FAILED_READ) != 0) {
		printf("READ: expected: '%s...' or '%s' actual: '%s'\n", ANSWER_SUCCESS_READ, ANSWER_FAILED_READ, response);
	}
	return (void *) NULL;
}

void read_files_by_client() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 3000;
	for (i = 0; i < max_files_to_test; i++) {
		char *filename = create_numbered_filename(filename_base_client, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		if (pthread_create(thread, NULL, (void*) read_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *list_file_run(void *ptr) {
	int deep = 2;
	char *response = send_message(list_msg);
	info(deep, "List files");
	if (strncmp(response, ANSWER_SUCCESS_LIST, 4) != 0) {
		printf("LIST: expected: '%s...' actual: '%s'\n", ANSWER_SUCCESS_LIST, response);
	}
	return (void *) NULL;
}

void list_files_by_client() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 4000;
	for (i = 0; i < max_listing_test; i++) {
		thread = (pthread_t *) malloc(sizeof(pthread_t));
		if (pthread_create(thread, NULL, (void*) list_file_run, NULL) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *delete_file_run(void *ptr) {
	int deep = 2;
	struct file_details *arg = (struct file_details *) ptr;
	info(deep, "Delete file: %s", arg->filename);
	char *delete_msg = create_delete_message(arg->filename);
	char *response = send_message(delete_msg);
	if (strcmp(response, ANSWER_SUCCESS_DELETE) != 0 && strcmp(response, ANSWER_FAILED_DELETE) != 0) {
		printf("DELETE: expected: '%s' or '%s' actual: '%s'\n", ANSWER_SUCCESS_DELETE, ANSWER_FAILED_DELETE, response);
	}
	return (void *) NULL;
}

void delete_files_by_client() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 5000;
	for (i = 0; i < max_files_to_test; i++) {
		char *filename = create_numbered_filename(filename_base_client, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		if (pthread_create(thread, NULL, (void*) delete_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

int main(int argc, char *argv[]) {
	int deep = 0;
	install_segfault_handler();

	set_log_lvl(INFO);
	init_thread_linked_list(true);

	info(deep, "Start %d threads, which create a file", max_files_to_test);
	create_files_by_client();
	info(deep, "Start %d threads, which update a file", max_files_to_test);
	update_files_by_client();
	info(deep, "Start %d threads, which read a file", max_files_to_test);
	read_files_by_client();
	info(deep, "Start %d threads, which get the list of files", max_listing_test);
	list_files_by_client();
	info(deep, "Start %d threads, which remove a file", max_files_to_test);
	delete_files_by_client();

	stop_cleanup_threads();
}
