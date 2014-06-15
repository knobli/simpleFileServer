/*
 * tester.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "../lib/util.c"
#include "logger-test.c"
#include "thread-linked-list-test.c"
#include "file-linked-list-test.c"
#include "server-test.c"
#include "message-creator.c"
#include "load-test.c"

int main(int argc, char **argv) {
	install_segfault_handler();
	assert(CUE_SUCCESS == CU_initialize_registry());

	printf("*****Start Tests*****\n");

	logger_test_suite();
	thread_list_test_suite();
	file_list_test_suite();
	message_creator_test_suite();
	server_test_suite();
	load_test_suite();

	CU_basic_run_tests();
	CU_cleanup_registry();

	return 0;
}
