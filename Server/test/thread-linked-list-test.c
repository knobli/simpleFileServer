/*
 * thread-linked-list-test.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include <thread-linked-list.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

void *thread_run(void *ptr) {
	printf("Thread started\n");
	return (void *) NULL;
}

void test_add_one_thread_element() {
	CU_ASSERT_TRUE(init_thread_linked_list(false));

	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t));
	CU_ASSERT_EQUAL_FATAL(pthread_create(thread, NULL, (void*) thread_run, NULL), 0);
	add_thread_element(1, thread);

	CU_ASSERT_EQUAL(cleanup_threads(), 1);
}

void test_add_two_thread_element() {
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t));
	CU_ASSERT_EQUAL_FATAL(pthread_create(thread, NULL, (void*) thread_run, NULL), 0);
	add_thread_element(2, thread);

	pthread_t *thread2 = (pthread_t *) malloc(sizeof(pthread_t));
	CU_ASSERT_EQUAL_FATAL(pthread_create(thread2, NULL, (void*) thread_run, NULL), 0);
	add_thread_element(3, thread2);

	CU_ASSERT_EQUAL(cleanup_threads(), 2);
}

void thread_list_test_suite() {
	CU_pSuite thread_list_suite = CU_add_suite("thread_list", NULL, NULL);
	CU_add_test(thread_list_suite, "test_add_one_thread_element", test_add_one_thread_element);
	CU_add_test(thread_list_suite, "test_add_two_thread_element", test_add_two_thread_element);
}
