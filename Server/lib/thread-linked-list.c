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
	struct thread_element *next;
};

struct thread_element *head_thread = NULL;

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
	thread_element->next = NULL;

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
}

struct thread_element* search_thread_element(int thread_idx, struct thread_element **prev,
		int lock) {
	const int deep = 4;
	int returnCode;
	struct thread_element *ptr = head_thread;
	struct thread_element *last = NULL;
	struct thread_element *second_last = NULL;
	bool found = false;
	info(deep, "Searching thread element '%d' in the list", thread_idx);
	while (ptr != NULL) {
		if (second_last != NULL && lock) {
			debug(deep, "Unlock second last link mod mutex on %p - search case",
					second_last);
			//debug(deep, "Link mod mutex %p - search case", &second_last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
			handle_thread_error(returnCode,
					"Could not release link mod mutex - search", THREAD_EXIT);
		}
		debug(deep, "Lock current link mod mutex on %p - search case", ptr);
		//debug(deep, "Link mod mutex %p - search case", &ptr->link_mod_mutex);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode,
				"Could not lock link mod mutex - search", THREAD_EXIT);
		if (ptr->thread_idx == thread_idx) {
			debug(deep, "Thread element '%d' found!", thread_idx);
			found = true;

			if(lock == true){
				debug(deep, "Unlock matching link mod mutex on %p - search case", ptr);
				//debug(deep, "Link mod mutex %p - search ptr", &ptr->link_mod_mutex);
				returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
				handle_thread_error(returnCode,
						"Could not release link mod mutex - search", THREAD_EXIT);
			}

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
		//debug(deep, "Link mod mutex %p - search case", &second_last->link_mod_mutex);
		returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
		handle_thread_error(returnCode,
				"Could not release link mod mutex - search", THREAD_EXIT);
	}

	if (prev) {
		debug(deep, "Set previous: %p", last);
		*prev = last;
	}

	if (true == found) {
		return ptr;
	} else {
		return NULL;
	}
}

int add_thread_element(int thread_idx, pthread_t thread) {
	const int deep = 3;
	int rv;
	struct thread_element *ptr = create_thread_element(thread_idx, thread);
	struct thread_element *endNode = NULL;
	struct thread_element *thread_element = search_thread_element(thread_idx, &endNode, true);
	if (thread_element == NULL) {
		info(deep, "Adding thread element '%d' to the end of the list", thread_idx);
		debug(deep, "Set next pointer of %p to new thread element %p", endNode, ptr);
		endNode->next = ptr;
		rv = true;
	} else {
		error(deep, "Thread element '%d' already exist", thread_idx);
		rv = false;
	}

	debug(deep, "Unlock list mod mutex on %p - create case", endNode);
	int returnCode = pthread_mutex_unlock(&endNode->link_mod_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create",
			THREAD_EXIT);
	return rv;
}

int clean_up_threads() {
	const int deep = 3;
	struct thread_element *ptr = head_thread;
	struct thread_element *last = NULL;
	int returnCode;
	info(deep, "Go trough thread elements");
	while (ptr != NULL) {
		if (last != NULL) {
			debug(deep, "Release last link mod mutex on %p - list case", last);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex",
					THREAD_EXIT);
		}
		debug(deep, "Lock link mod mutex on %p - list case", ptr);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex",
				THREAD_EXIT);

		if (ptr->thread_idx != 0) {
			debug(deep, "Join thread '%d'", ptr->thread_idx);
			pthread_join(ptr->thread, NULL);
			debug(deep, "Delete thread '%d'", ptr->thread_idx);
			last->next = ptr->next;
			debug(deep, "Unlock link mod mutex on %p - delete case", last);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex - delete",
					THREAD_EXIT);
			debug(deep, "Free pointer %p of thread element", ptr);
			free(ptr);
			ptr = NULL;
			debug(deep, "Set pointer %p as curr pointer", last->next);
			ptr = last->next;
		} else {
			last = ptr;
			ptr = ptr->next;
		}

	}
	if (last != NULL) {
		debug(deep, "Release last link mod mutex on %p - list case", last);
		returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not release link mod mutex",
				THREAD_EXIT);
	}

	return true;
}
