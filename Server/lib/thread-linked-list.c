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
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

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

pthread_t *cleanup_thread = NULL;
bool clean_threads_flag = true;

void *thread_cleanup_run(void *ptr) {
	const int deep = 1;
	info(deep, "Cleanup thread started");
	while (clean_threads_flag) {
		sleep(2);
		info(deep, "Start cleanup running threads");
		cleanup_threads();
	}
	return (void *) NULL;
}

void stop_cleanup_threads() {
	const int deep = 2;
	debug(deep, "Stop clean running threads");
	clean_threads_flag = false;
	if (cleanup_thread != NULL) {
		pthread_join(*cleanup_thread, NULL);
	}
	free(cleanup_thread);
}

struct thread_element* create_thread_element(size_t thread_idx, pthread_t *thread) {
	const int deep = 4;
	debug(deep, "Create thread element");
	struct thread_element *thread_element = (struct thread_element*) malloc(sizeof(struct thread_element));
	if (NULL == thread_element) {
		error(deep, "Thread node creation failed");
		return NULL;
	}
	thread_element->thread_idx = thread_idx;
	thread_element->thread = thread;
	thread_element->prev = NULL;

	finest(deep, "New thread element %p created", thread_element);
	return thread_element;
}

bool init_thread_linked_list(bool start_cleanup) {
	int deep = 1;
	debug(deep, "Init thread linked list");
	head_thread = create_thread_element(0, 0);
	end_thread = head_thread;

	int returnCode;
	debug(deep, "Init thread head link mod mutex");
	memset(&head_thread_mutex, 0, sizeof(head_thread_mutex)); /* Zero out structure */
	returnCode = pthread_mutex_init(&head_thread_mutex, NULL);
	handle_thread_error(returnCode, "Could not init link mod mutex", THREAD_EXIT);

	if (start_cleanup) {
		/* create cleanup thread: */
		cleanup_thread = (pthread_t *) malloc(sizeof(pthread_t));
		if (pthread_create(cleanup_thread, NULL, (void*) thread_cleanup_run, NULL) != 0) {
			error(deep, "pthread_create for cleanup thread failed");
		} else {
			debug(deep, "pthread_create for cleanup thread success");
		}
	}

	return true;
}

bool add_thread_element(size_t thread_idx, pthread_t *thread) {

	const int deep = 3;
	struct thread_element *thread_element = create_thread_element(thread_idx, thread);
	int returnCode;

	debug(deep, "Lock head thread mutex - add case");
	returnCode = pthread_mutex_lock(&head_thread_mutex);
	handle_thread_error(returnCode, "Could not lock link mod mutex - add", THREAD_EXIT);

	debug(deep, "Adding thread element '%d' to the beginning of the list", thread_idx);
	head_thread->prev = thread_element;
	head_thread = thread_element;

	debug(deep, "Unlock head thread mutex - add case");
	returnCode = pthread_mutex_unlock(&head_thread_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create", THREAD_EXIT);
	return true;
}

size_t cleanup_threads() {
	const int deep = 3;
	size_t threads_joined = 0;
	struct thread_element *ptr = end_thread->prev;
	struct thread_element *last = end_thread;
	int returnCode;
	info(deep, "Go trough thread elements");
	while (ptr != NULL) {
		debug(deep, "Join thread '%d'", ptr->thread_idx);
		pthread_join(*ptr->thread, NULL);
		threads_joined++;

		if (ptr->prev == NULL) {
			debug(deep, "Special case: remove last element from list");
			debug(deep, "Lock head thread link mod mutex - cleanup case");
			if (pthread_mutex_trylock(&head_thread_mutex) == 0) {
				last->prev = ptr->prev;

				debug(deep, "Set initial element as head of list");
				head_thread = last;

				ptr->prev = NULL;
				free(ptr->thread);
				free(ptr);

				debug(deep, "Unlock head thread link mod mutex - cleanup case");
				returnCode = pthread_mutex_unlock(&head_thread_mutex);
				handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup", THREAD_EXIT);
			} else {
				error(deep, "Could not lock head thread link mod mutex, will come later!");
				break;
			}
		} else {
			last->prev = ptr->prev;

			ptr->prev = NULL;
			free(ptr->thread);
			free(ptr);
		}

		ptr = last->prev;
	}
	info(deep, "Joined %zu thread(s)", threads_joined);
	return threads_joined;
}

bool destroy_thread_linked_list(){
	int deep = 1;
	int returnCode;
	stop_cleanup_threads();
	returnCode=pthread_mutex_destroy(&head_thread_mutex);
	handle_thread_error(returnCode, "Could not lock link mod mutex - cleanup", THREAD_EXIT);
	debug(deep, "Reset head and end thread");
	head_thread = NULL;
	end_thread->prev = NULL;
	free(end_thread->thread);
	free(end_thread);
	return true;
}
