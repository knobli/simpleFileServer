/*
 * thread-linked-list.h
 *
 *  Created on: 06.06.2014
 *      Author: knobli
 */

#ifndef THREAD_LINKED_LIST_H_
#define THREAD_LINKED_LIST_H_

#include <stdbool.h>
#include <stddef.h>

bool init_thread_linked_list(bool start_cleanup);

bool add_thread_element(size_t thread_idx, pthread_t *thread);

size_t cleanup_threads();

void stop_cleanup_threads();

bool destroy_thread_linked_list();

#endif /* THREAD_LINKED_LIST_H_ */
