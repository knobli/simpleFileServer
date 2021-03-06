/*
 * logger-test.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <logger.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

void test_debug_level() {
	CU_ASSERT_TRUE(set_log_lvl(DEBUG));
	char *test_msg = "Test msg";
	int deep = 1;
	CU_ASSERT_TRUE(debug(deep, test_msg));
	CU_ASSERT_TRUE(info(deep, test_msg));
	CU_ASSERT_TRUE(warn(deep, test_msg));
	CU_ASSERT_TRUE(error(deep, test_msg));
}

void test_info_level() {
	CU_ASSERT_TRUE(set_log_lvl(INFO));
	char *test_msg = "Test msg";
	int deep = 1;
	CU_ASSERT_FALSE(debug(deep, test_msg));
	CU_ASSERT_TRUE(info(deep, test_msg));
	CU_ASSERT_TRUE(warn(deep, test_msg));
	CU_ASSERT_TRUE(error(deep, test_msg));
}

void test_warn_level() {
	CU_ASSERT_TRUE(set_log_lvl(WARN));
	char *test_msg = "Test msg";
	int deep = 1;
	CU_ASSERT_FALSE(debug(deep, test_msg));
	CU_ASSERT_FALSE(info(deep, test_msg));
	CU_ASSERT_TRUE(warn(deep, test_msg));
	CU_ASSERT_TRUE(error(deep, test_msg));
}

void test_error_level() {
	CU_ASSERT_TRUE(set_log_lvl(ERROR));
	char *test_msg = "Test msg";
	int deep = 1;
	CU_ASSERT_FALSE(debug(deep, test_msg));
	CU_ASSERT_FALSE(info(deep, test_msg));
	CU_ASSERT_FALSE(warn(deep, test_msg));
	CU_ASSERT_TRUE(error(deep, test_msg));
}

void test_long_message() {
	CU_ASSERT_TRUE(set_log_lvl(DEBUG));
	char *long_msg =
			"123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123";
	int deep = 1;
	CU_ASSERT_TRUE(debug(deep, long_msg));
}

void test_too_long_message() {
	CU_ASSERT_TRUE(set_log_lvl(DEBUG));
	char *too_long_msg =
			"123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A12345";
	int deep = 1;
	CU_ASSERT_TRUE(debug(deep, too_long_msg));
}

void test_messages_with_additional_args() {
	CU_ASSERT_TRUE(set_log_lvl(DEBUG));
	char *message_template = "Test msg with number %zu and string '%s'";
	int deep = 1;
	size_t number = 15;
	char *test_string = "Test string";
	CU_ASSERT_TRUE(debug(deep, message_template, number, test_string));
}

void test_append_strings() {
	char *final_string = malloc(1);
	final_string[0] = '\000';
	char *tmp_string;
	int tmp_length;

	char *string1 = "Bla";
	char *string2 = "Test";
	char *string3 = "End";

	tmp_length = append_strings(final_string, string1, &tmp_string);
	free(final_string);
	final_string = malloc(tmp_length);
	strncpy(final_string, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(final_string, " ", &tmp_string);
	free(final_string);
	final_string = malloc(tmp_length);
	strncpy(final_string, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(final_string, string2, &tmp_string);
	free(final_string);
	final_string = malloc(tmp_length);
	strncpy(final_string, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(final_string, "\n", &tmp_string);
	free(final_string);
	final_string = malloc(tmp_length);
	strncpy(final_string, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(final_string, string3, &tmp_string);
	free(final_string);
	final_string = malloc(tmp_length);
	strncpy(final_string, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(final_string, "\n", &tmp_string);
	free(final_string);
	final_string = malloc(tmp_length);
	strncpy(final_string, tmp_string, tmp_length);
	free(tmp_string);

	CU_ASSERT_STRING_EQUAL(final_string, "Bla Test\nEnd\n");
	free(final_string);
}

void logger_test_suite() {
	CU_pSuite logger_suite = CU_add_suite("logger", NULL, NULL);
	CU_add_test(logger_suite, "test_debug_level", test_debug_level);
	CU_add_test(logger_suite, "test_info_level", test_info_level);
	CU_add_test(logger_suite, "test_warn_level", test_warn_level);
	CU_add_test(logger_suite, "test_error_level", test_error_level);
	CU_add_test(logger_suite, "test_long_message", test_long_message);
	CU_add_test(logger_suite, "test_too_long_message", test_too_long_message);
	CU_add_test(logger_suite, "test_messages_with_additional_args", test_messages_with_additional_args);
	CU_add_test(logger_suite, "test_append_strings", test_append_strings);
}
