/*
 * load-test.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <file-linked-list.h>
#include <transmission-protocols.h>
#include <logger.h>
#include "message-creator.h"

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

const char *filename_base = "file";
const char *file_content = "just a small content";
const char *file_content_up = "just a small content with more information";
const size_t max_files = 100;

void test_create_files() {
	CU_ASSERT_TRUE(init_linked_list());
	size_t i;
	for (i = 0; i < max_files; i++) {
		char *create_msg = create_create_message_numbered(filename_base, i, file_content);
		CU_ASSERT_STRING_EQUAL(create_file(create_msg), ANSWER_SUCCESS_CREATE);
		free(create_msg);
	}
}

void test_update_files() {
	size_t i;
	for (i = 0; i < max_files; i++) {
		char *update_msg = create_update_message_numbered(filename_base, i, file_content_up);
		CU_ASSERT_STRING_EQUAL(update_file(update_msg), ANSWER_SUCCESS_UPDATE);
		free(update_msg);
	}
}

void test_read_files() {
	size_t i;
	for (i = 0; i < max_files; i++) {
		char *read_msg = create_read_message_numbered(filename_base, i);
		CU_ASSERT_NSTRING_EQUAL(read_file(read_msg), "FILECONTENT ", 12);
		free(read_msg);
	}
}

void test_list_files_load() {
	CU_ASSERT_NSTRING_EQUAL(list_files(list_msg), "ACK ", 4);
}

void test_delete_files() {
	size_t i;
	for (i = 0; i < max_files; i++) {
		char *delete_msg = create_delete_message_numbered(filename_base, i);
		CU_ASSERT_STRING_EQUAL(delete_file(delete_msg), ANSWER_SUCCESS_DELETE);
		free(delete_msg);
	}
}

void load_test_suite() {
	CU_pSuite load_test_suite = CU_add_suite("load", NULL, NULL);
	CU_add_test(load_test_suite, "test_create_files", test_create_files);
	CU_add_test(load_test_suite, "test_update_files", test_update_files);
	CU_add_test(load_test_suite, "test_read_files", test_read_files);
	CU_add_test(load_test_suite, "test_list_files_load", test_list_files_load);
	CU_add_test(load_test_suite, "test_delete_files", test_delete_files);
}
