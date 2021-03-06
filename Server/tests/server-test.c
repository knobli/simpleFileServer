/*
 * server-test.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <file-linked-list.h>
#include "message-creator.h"
#include <transmission-protocols.h>
#include <logger.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

const char *filename1 = "file3";
const char *content1 = "abcdefghij";
const char *content1_up = "klmnopqrstuvwxyz";
const char *filename2 = "file4";
const char *content2 = "klmnopqrstuvwx";
const char *content2_up = "abcdefgh";

const char *filename3 = "Test file33";
const char *content_special_char = "_ das / *\" content '^";
const char *filename_special_char = "Test _ das  / *\" '^";

void test_create_file() {
	CU_ASSERT_TRUE(init_linked_list());
	char *create_msg = create_create_message(filename1, content1);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg), ANSWER_SUCCESS_CREATE);
	free(create_msg);
}

void test_update_file() {
	char *update_msg = create_update_message(filename1, content1_up);
	CU_ASSERT_STRING_EQUAL(update_file(update_msg), ANSWER_SUCCESS_UPDATE);
	free(update_msg);
}

void test_read_file() {
	char *read_msg = create_read_message(filename1);
	char *response = read_file(read_msg);
	free(read_msg);
	CU_ASSERT_STRING_EQUAL(response, "FILECONTENT file3 17\nklmnopqrstuvwxyz\n");
	free(response);
}

void test_delete_file() {
	char *delete_msg = create_delete_message(filename1);
	char *response = delete_file(delete_msg);
	free(delete_msg);
	CU_ASSERT_STRING_EQUAL(response, ANSWER_SUCCESS_DELETE);
}

void test_failure_cases() {
	//add file twice
	char *create_msg = create_create_message(filename1, content1);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg), ANSWER_SUCCESS_CREATE);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg), ANSWER_FAILED_CREATE);
	free(create_msg);
	char *delete_msg = create_delete_message(filename1);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg), ANSWER_SUCCESS_DELETE);
	free(delete_msg);

	char *update_msg2 = create_update_message(filename2, content2_up);
	CU_ASSERT_STRING_EQUAL(update_file(update_msg2), ANSWER_FAILED_UPDATE);
	free(update_msg2);

	char *read_msg2 = create_read_message(filename2);
	char *content = read_file(read_msg2);
	free(read_msg2);
	CU_ASSERT_STRING_EQUAL(content, ANSWER_FAILED_READ);
	free(content);

	char *delete_msg2 = create_delete_message(filename2);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg2), ANSWER_FAILED_DELETE);
	free(delete_msg2);
}

void test_list_files() {
	char *create_msg = create_create_message(filename1, content1);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg), ANSWER_SUCCESS_CREATE);
	free(create_msg);
	char *create_msg2 = create_create_message(filename2, content2);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg2), ANSWER_SUCCESS_CREATE);
	free(create_msg2);

	char *response = list_files(list_msg);
	CU_ASSERT_STRING_EQUAL(response, "ACK 2\nfile3\nfile4\n");
	free(response);

	char *delete_msg = create_delete_message(filename1);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg), ANSWER_SUCCESS_DELETE);
	free(delete_msg);
	char *delete_msg2 = create_delete_message(filename2);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg2), ANSWER_SUCCESS_DELETE);
	free(delete_msg2);
}

void test_special_chars() {
	//special chars in file name
	char *create_msg_special_char_in_content = create_create_message(filename3, content_special_char);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg_special_char_in_content),
	ANSWER_SUCCESS_CREATE);
	free(create_msg_special_char_in_content);

	char *update_msg_special_char_in_content = create_update_message(filename3, content_special_char);
	CU_ASSERT_STRING_EQUAL(update_file(update_msg_special_char_in_content),
	ANSWER_SUCCESS_UPDATE);
	free(update_msg_special_char_in_content);

	char *read_msg_special_char_in_content = create_read_message(filename3);
	char *content = read_file(read_msg_special_char_in_content);
	free(read_msg_special_char_in_content);
	CU_ASSERT_STRING_EQUAL(content, "FILECONTENT Test file33 22\n_ das / *\" content '^\n");
	free(content);

	char *delete_msg_special_char_in_content = create_delete_message(filename3);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg_special_char_in_content),
	ANSWER_SUCCESS_DELETE);
	free(delete_msg_special_char_in_content);

	//special chars in content and file name
	char *create_msg_special_char = create_create_message(filename_special_char, content_special_char);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg_special_char),
	ANSWER_SUCCESS_CREATE);
	free(create_msg_special_char);

	char *update_msg_special_char = create_update_message(filename_special_char, content_special_char);
	CU_ASSERT_STRING_EQUAL(update_file(update_msg_special_char),
	ANSWER_SUCCESS_UPDATE);
	free(update_msg_special_char);

	char *read_msg_special_char = create_read_message(filename_special_char);
	char *content2 = read_file(read_msg_special_char);
	free(read_msg_special_char);
	CU_ASSERT_STRING_EQUAL(content2, "FILECONTENT Test _ das  / *\" '^ 22\n_ das / *\" content '^\n");
	free(content2);

	char *response = list_files(list_msg);
	CU_ASSERT_STRING_EQUAL(response, "ACK 1\nTest _ das  / *\" '^\n");
	free(response);

	char *delete_msg_special_char = create_delete_message(filename_special_char);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg_special_char),
	ANSWER_SUCCESS_DELETE);
	free(delete_msg_special_char);

}

void test_select_strategy() {
	char *create_msg = create_create_message(filename1, content1);
	char *response = select_strategy(create_msg);
	free(create_msg);
	CU_ASSERT_STRING_EQUAL(response, ANSWER_SUCCESS_CREATE);
	free(response);

	char *update_msg = create_update_message(filename1, content1_up);
	response = select_strategy(update_msg);
	free(update_msg);
	CU_ASSERT_STRING_EQUAL(response, ANSWER_SUCCESS_UPDATE);
	free(response);

	char *read_msg = create_read_message(filename1);
	char *content = select_strategy(read_msg);
	free(read_msg);
	CU_ASSERT_STRING_EQUAL(content, "FILECONTENT file3 17\nklmnopqrstuvwxyz\n");
	free(content);

	char *delete_msg = create_delete_message(filename1);
	response = select_strategy(delete_msg);
	CU_ASSERT_STRING_EQUAL(response, ANSWER_SUCCESS_DELETE);
	free(delete_msg);
	free(response);
}

void server_test_suite() {
	CU_pSuite server_suite = CU_add_suite("server", NULL, NULL);
	CU_add_test(server_suite, "test_create_file", test_create_file);
	CU_add_test(server_suite, "test_update_file", test_update_file);
	CU_add_test(server_suite, "test_read_file", test_read_file);
	CU_add_test(server_suite, "test_delete_file", test_delete_file);
	CU_add_test(server_suite, "test_failure_cases", test_failure_cases);
	CU_add_test(server_suite, "test_list_files", test_list_files);
	CU_add_test(server_suite, "test_special_chars", test_special_chars);
	CU_add_test(server_suite, "test_select_strategy", test_select_strategy);
}
