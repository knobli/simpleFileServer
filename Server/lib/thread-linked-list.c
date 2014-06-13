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
	int thread_idx;
	pthread_mutex_t link_mod_mutex;
	pthread_t thread;
	struct thread_element *prev;
};

pthread_mutex_t head_thread_mutex;
struct thread_element *head_thread = NULL;
struct thread_element *end_thread = NULL;

struct thread_element* create_thread_element(int thread_idx, pthread_t thread) {
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

	int returnCode;
	debug(deep, "Init link mod mutex");
	pthread_mutex_t mutex;
	returnCode = pthread_mutex_init(&mutex, NULL);
	handle_thread_error(returnCode, "Could not init link mod mutex",
			THREAD_EXIT);
	thread_element->link_mod_mutex = mutex;

	debug(deep, "New thread element %p created", thread_element);
	return thread_element;
}

void init_thread_linked_list() {
	int deep = 1;
	debug(deep, "Init thread linked list");
	head_thread = create_thread_element(0, 0);
	end_thread = head_thread;
}

int add_thread_element(int thread_idx, pthread_t thread) {
	const int deep = 3;
	int rv;
	struct thread_element *thread_element = create_thread_element(thread_idx, thread);
	int returnCode;

	debug(deep, "Lock head thread mutex - add case");
	returnCode = pthread_mutex_lock(&head_thread_mutex);
	handle_thread_error(returnCode,
			"Could not lock link mod mutex - add", THREAD_EXIT);

	debug(deep, "Lock head thread link mod mutex - add case", head_thread);
	returnCode = pthread_mutex_lock(&head_thread->link_mod_mutex);
	handle_thread_error(returnCode,
			"Could not lock link mod mutex - search", THREAD_EXIT);
	/*
	 * Debug
	 */
	if(head_thread->prev != NULL){
		error(deep, "Previous of head thread not NULL!");
	}
	/*
		 * Debug
		 */

	info(deep, "Adding thread element '%d' to the beginning of the list", thread_idx);
	head_thread->prev = thread_element;
	head_thread = thread_element;
	rv = true;

	debug(deep, "Unlock head thread link mod mutex - add casee", head_thread);
	int returnCode = pthread_mutex_unlock(&head_thread->link_mod_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create",
			THREAD_EXIT);

	debug(deep, "Unlock head thread mutex - add case");
	int returnCode = pthread_mutex_unlock(&head_thread_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create",
			THREAD_EXIT);
	return rv;
}

int clean_up_threads() {
	const int deep = 3;
	struct thread_element *ptr = end_thread->prev;
	struct thread_element *last = end_thread;
	int returnCode;
	info(deep, "Go trough thread elements");
	while (ptr != NULL) {
		debug(deep, "Join thread '%d'", ptr->thread_idx);
		pthread_join(ptr->thread, NULL);

		debug(deep, "Lock link mod mutex on %p - cleanup case", last);
		returnCode = pthread_mutex_lock(&last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup",
				THREAD_EXIT);

		if(ptr->prev == NULL){
			debug(deep, "Special case: remove last element from list");
			debug(deep, "Lock head thread link mod mutex - cleanup case");
			returnCode = pthread_mutex_lock(&head_thread_mutex);
			handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup",
					THREAD_EXIT);
		}

		last->prev = ptr->prev;

		if(ptr->prev == NULL){
			debug(deep, "Unlock head thread link mod mutex - cleanup case");
			returnCode = pthread_mutex_unlock(&head_thread_mutex);
			handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup",
					THREAD_EXIT);
		}

		debug(deep, "Unlock last link mod mutex on %p - list case", last);
		returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not release link mod mutex - cleanup",
				THREAD_EXIT);

		free(ptr);

		ptr = ptr->prev;
	}
	return true;
}
