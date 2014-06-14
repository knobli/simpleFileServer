/*
 * thread-linked-list.c
 *
 *  Created on: 06.06.2014
 *      Author: knobli
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <logger.h>
#include <serverlib.h>
#include <thread-linked-list.h>

struct thread_element {
	size_t thread_idx;
	pthread_t *thread;
	struct thread_element *prev;
};

pthread_mutex_t head_thread_mutex;
struct thread_element *head_thread = NULL;
struct thread_element *end_thread = NULL;

struct thread_element* create_thread_element(size_t thread_idx, pthread_t *thread) {
	const int deep = 4;
	debug(deep, "Create thread element");
	struct thread_element *thread_element = (struct thread_element*) malloc(
			sizeof(struct thread_element));
	if (NULL == thread_element) {
		error(deep, "Thread node creation failed");
		return NULL;
	}
	thread_element->thread_idx = thread_idx;
	thread_element->thread = thread;
	thread_element->prev = NULL;

	debug(deep, "New thread element %p created", thread_element);
	return thread_element;
}

bool init_thread_linked_list() {
	int deep = 1;
	debug(deep, "Init thread linked list");
	head_thread = create_thread_element(0, 0);
	end_thread = head_thread;

	int returnCode;
	debug(deep, "Init thread head link mod mutex");
	returnCode = pthread_mutex_init(&head_thread_mutex, NULL);
	handle_thread_error(returnCode, "Could not init link mod mutex",
			THREAD_EXIT);
	return true;
}

bool add_thread_element(size_t thread_idx, pthread_t *thread) {

	const int deep = 3;
	struct thread_element *thread_element = create_thread_element(thread_idx, thread);
	int returnCode;

	debug(deep, "Lock head thread mutex - add case");
	returnCode = pthread_mutex_lock(&head_thread_mutex);
	handle_thread_error(returnCode,
			"Could not lock link mod mutex - add", THREAD_EXIT);

	info(deep, "Adding thread element '%d' to the beginning of the list", thread_idx);
	head_thread->prev = thread_element;
	head_thread = thread_element;

	debug(deep, "Unlock head thread mutex - add case");
	returnCode = pthread_mutex_unlock(&head_thread_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create",
			THREAD_EXIT);
	return true;
}

size_t clean_up_threads() {
	const int deep = 3;
	size_t threads_joined = 0;
	struct thread_element *ptr = end_thread->prev;
	struct thread_element *last = end_thread;
	int returnCode;
	info(deep, "Go trough thread elements");
	while (ptr != NULL) {
		info(deep, "Join thread '%d'", ptr->thread_idx);
		pthread_join(*ptr->thread, NULL);
		threads_joined++;

		if(ptr->prev == NULL){
			debug(deep, "Special case: remove last element from list");
			debug(deep, "Lock head thread link mod mutex - cleanup case");
			returnCode = pthread_mutex_lock(&head_thread_mutex);
			handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup",
					THREAD_EXIT);
		}

		last->prev = ptr->prev;

		if(ptr->prev == NULL){
			debug(deep, "Set initial element as head of list");
			head_thread = last;

			debug(deep, "Unlock head thread link mod mutex - cleanup case");
			returnCode = pthread_mutex_unlock(&head_thread_mutex);
			handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup",
					THREAD_EXIT);
		}
		free(ptr->thread);
		free(ptr);

		ptr = ptr->prev;
	}
	return threads_joined;
}
