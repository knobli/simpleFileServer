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

#include <transmission-protocols.h>
#include <logger.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

const char *create_msg = "CREATE file3 10\nabcdefghij\n";
const char *update_msg = "UPDATE file3 16\nklmnopqrstuvwxyz\n";
const char *read_msg = "READ file3\n";
const char *delete_msg = "DELETE file3\n";

const char *create_msg2 = "CREATE file4 14\nklmnopqrstuvwx\n";
const char *update_msg2 = "UPDATE file4 8\nabcdefgh\n";
const char *read_msg2 = "READ file4\n";
const char *delete_msg2 = "DELETE file4\n";

const char *create_msg_special_char_in_content = "CREATE Test file33 21\n_ das / *\" content '^\n";
const char *update_msg_special_char_in_content = "UPDATE Test file33 21\n_ das / *\" content '^\n";
const char *read_msg_special_char_in_content = "READ Test file33\n";
const char *delete_msg_special_char_in_content = "DELETE Test file33\n";

const char *create_msg_special_char = "CREATE Test _ das  / *\" '^ 21\n_ das / *\" content '^\n";
const char *update_msg_special_char = "UPDATE Test _ das  / *\" '^ 21\n_ das / *\" content '^\n";
const char *read_msg_special_char = "READ Test _ das  / *\" '^\n";
const char *delete_msg_special_char = "DELETE Test _ das  / *\" '^\n";

const char *list_msg = "LIST\n";

void test_create_file() {
	CU_ASSERT_TRUE(init_linked_list());
	CU_ASSERT_STRING_EQUAL(create_file(create_msg),ANSWER_SUCCESS_CREATE);
}

void test_update_file() {
	CU_ASSERT_STRING_EQUAL(update_file(update_msg),ANSWER_SUCCESS_UPDATE);
}

void test_read_file() {
	char *response = read_file(read_msg);
	CU_ASSERT_STRING_EQUAL(response, "FILECONTENT file3 16\nklmnopqrstuvwxyz\n");
}

void test_delete_file() {
	char *response = delete_file(delete_msg);
	CU_ASSERT_STRING_EQUAL(response,ANSWER_SUCCESS_DELETE);
}

void test_failure_cases() {
	//add file twice
	CU_ASSERT_STRING_EQUAL(create_file(create_msg),ANSWER_SUCCESS_CREATE);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg),ANSWER_FAILED_CREATE);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg),ANSWER_SUCCESS_DELETE);

	CU_ASSERT_STRING_EQUAL(update_file(update_msg2),ANSWER_FAILED_UPDATE);

	CU_ASSERT_STRING_EQUAL(read_file(read_msg2), ANSWER_FAILED_READ);

	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg2),ANSWER_FAILED_DELETE);
}

void test_list_files() {
	CU_ASSERT_STRING_EQUAL(create_file(create_msg),ANSWER_SUCCESS_CREATE);
	CU_ASSERT_STRING_EQUAL(create_file(create_msg2),ANSWER_SUCCESS_CREATE);

	char *response = list_files(list_msg);
	CU_ASSERT_STRING_EQUAL(response, "ACK 2\nfile3\nfile4\n");

	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg),ANSWER_SUCCESS_DELETE);
	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg2),ANSWER_SUCCESS_DELETE);
}

void test_special_chars() {


	CU_ASSERT_STRING_EQUAL(create_file(create_msg_special_char_in_content),ANSWER_SUCCESS_CREATE);

	CU_ASSERT_STRING_EQUAL(update_file(update_msg_special_char_in_content),ANSWER_SUCCESS_UPDATE);

	CU_ASSERT_STRING_EQUAL(read_file(read_msg_special_char_in_content), "FILECONTENT Test file33 21\n_ das / *\" content '^\n");

	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg_special_char_in_content),ANSWER_SUCCESS_DELETE);

	CU_ASSERT_STRING_EQUAL(create_file(create_msg_special_char),ANSWER_SUCCESS_CREATE);

	CU_ASSERT_STRING_EQUAL(update_file(update_msg_special_char),ANSWER_SUCCESS_UPDATE);

	CU_ASSERT_STRING_EQUAL(read_file(read_msg_special_char), "FILECONTENT Test _ das  / *\" '^ 21\n_ das / *\" content '^\n");

	CU_ASSERT_STRING_EQUAL(list_files(list_msg), "ACK 1\nTest _ das  / *\" '^\n");

	CU_ASSERT_STRING_EQUAL(delete_file(delete_msg_special_char),ANSWER_SUCCESS_DELETE);

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
}
