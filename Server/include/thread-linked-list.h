/*
 * thread-linked-list.h
 *
 *  Created on: 06.06.2014
 *      Author: knobli
 */

#ifndef THREAD_LINKED_LIST_H_
#define THREAD_LINKED_LIST_H_

void init_thread_linked_list();

int add_thread_element(int thread_idx, pthread_t thread);

int clean_up_threads();


#endif /* THREAD_LINKED_LIST_H_ */
