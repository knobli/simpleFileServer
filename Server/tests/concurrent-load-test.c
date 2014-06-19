/*
 * concurrent-load-test.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "../lib/util.c"
#include <file-linked-list.h>
#include <thread-linked-list.h>
#include <transmission-protocols.h>
#include <logger.h>
#include "message-creator.c"

const char *filename_base_concurrent = "file_concurrent";
const char *file_content_concurrent = "just a small content";
const size_t content_lengt_concurrent = 21;
const char *file_content_up_concurrent = "just a small content with more information";
const size_t content_up_lengt_concurrent = 43;
const size_t max_files_concurrent = 100;

struct file_details {
	char *filename;
	char *content;
};

void *create_file_run(void *ptr) {
	struct file_details *arg = (struct file_details *) ptr;
	char *create_msg = create_create_message(arg->filename, arg->content);
	char *response = create_file(create_msg);
	free(create_msg);
	if (strcmp(response, ANSWER_SUCCESS_CREATE) != 0) {
		printf("expected: '%s' actual: '%s'\n", response, ANSWER_SUCCESS_CREATE);
	}
	free(arg->filename);
	free(arg->content);
	free(arg);
	return (void *) NULL;
}

void test_create_files_concurrent() {
	if (!init_linked_list()) {
		printf("could not init linked list\n");
	}
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 1000;
	for (i = 0; i < max_files_concurrent; i++) {
		char *filename = create_numbered_filename(filename_base_concurrent, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		free(filename);
		thread_data->content = malloc(content_lengt_concurrent);
		strncpy(thread_data->content, file_content_concurrent, content_lengt_concurrent);
		if (pthread_create(thread, NULL, (void*) create_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *update_file_run(void *ptr) {
	struct file_details *arg = (struct file_details *) ptr;
	char *update_msg = create_update_message(arg->filename, arg->content);
	char *response = update_file(update_msg);
	free(update_msg);
	if (strcmp(response, ANSWER_SUCCESS_UPDATE) != 0) {
		printf("expected: '%s' actual: '%s'\n", response, ANSWER_SUCCESS_UPDATE);
	}
	free(arg->filename);
	free(arg->content);
	free(arg);
	return (void *) NULL;
}

void test_update_files_concurrent() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 2000;
	for (i = 0; i < max_files_concurrent; i++) {
		char *filename = create_numbered_filename(filename_base_concurrent, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		free(filename);
		thread_data->content = malloc(content_lengt_concurrent);
		strncpy(thread_data->content, file_content_concurrent, content_lengt_concurrent);
		if (pthread_create(thread, NULL, (void*) update_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *read_file_run(void *ptr) {
	struct file_details *arg = (struct file_details *) ptr;
	char *read_msg = create_read_message(arg->filename);
	char *response = read_file(read_msg);
	free(read_msg);
	if (strncmp(response, ANSWER_SUCCESS_READ, 12) != 0) {
		printf("expected: '%s' actual: '%s...'\n", response, ANSWER_SUCCESS_READ);
	}
	free(response);
	free(arg->filename);
	free(arg);
	return (void *) NULL;
}

void test_read_files_concurrent() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 3000;
	for (i = 0; i < max_files_concurrent; i++) {
		char *filename = create_numbered_filename(filename_base_concurrent, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		free(filename);
		if (pthread_create(thread, NULL, (void*) read_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *list_file_run(void *ptr) {
	char *response = list_files(list_msg);
	if (strncmp(response, ANSWER_SUCCESS_LIST, 4) != 0) {
		printf("expected: '%s' actual: '%s...'\n", response, ANSWER_SUCCESS_LIST);
	}
	free(response);
	return (void *) NULL;
}

void test_list_files_concurrent() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 4000;
	for (i = 0; i < max_files_concurrent; i++) {
		thread = (pthread_t *) malloc(sizeof(pthread_t));
		if (pthread_create(thread, NULL, (void*) list_file_run, NULL) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

void *delete_file_run(void *ptr) {
	struct file_details *arg = (struct file_details *) ptr;
	char *delete_msg = create_delete_message(arg->filename);
	char *response = delete_file(delete_msg);
	free(delete_msg);
	if (strcmp(response, ANSWER_SUCCESS_DELETE) != 0) {
		printf("expected: '%s' actual: '%s'\n", response, ANSWER_SUCCESS_DELETE);
	}
	free(arg->filename);
	free(arg);
	return (void *) NULL;
}

void test_delete_files_concurrent() {
	pthread_t *thread;
	size_t i;
	size_t thread_no_base = 5000;
	for (i = 0; i < max_files_concurrent; i++) {
		char *filename = create_numbered_filename(filename_base_concurrent, i);
		size_t filename_lenght = strlen(filename) + 1;

		thread = (pthread_t *) malloc(sizeof(pthread_t));
		struct file_details *thread_data = (struct file_details *) malloc(sizeof(struct file_details));

		thread_data->filename = malloc(filename_lenght);
		strncpy(thread_data->filename, filename, filename_lenght);
		free(filename);
		if (pthread_create(thread, NULL, (void*) delete_file_run, (void*) thread_data) != 0) {
			printf("Could not start thread %zu\n", i);
		} else {
			add_thread_element((i + thread_no_base), thread);
		}
	}
}

int main(int argc, char *argv[]) {
	install_segfault_handler();

	set_log_lvl(FINEST);
	init_thread_linked_list(true);

	test_create_files_concurrent();
	test_update_files_concurrent();
	test_read_files_concurrent();
	test_list_files_concurrent();
	test_delete_files_concurrent();

	stop_cleanup_threads();
}
