/*
 * message-creator.c
 *
 *  Created on: 15.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "message-creator.h"

#include <logger.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

char *create_content_message(const char *filename, const char *content) {
	char *msg = malloc(1);
	msg[0] = '\000';
	char *tmp_string;
	int tmp_length;

	tmp_length = append_strings(msg, filename, &tmp_string);
	free(msg);
	msg = malloc(tmp_length);
	strncpy(msg, tmp_string, tmp_length);
	free(tmp_string);

	size_t length = strlen(content) + 1;
	char len_string[15];
	sprintf(len_string, " %zu\n", length);

	tmp_length = append_strings(msg, len_string, &tmp_string);
	free(msg);
	msg = malloc(tmp_length);
	strncpy(msg, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(msg, content, &tmp_string);
	free(msg);
	msg = malloc(tmp_length);
	strncpy(msg, tmp_string, tmp_length);
	free(tmp_string);

	tmp_length = append_strings(msg, "\n", &tmp_string);
	free(msg);
	msg = malloc(tmp_length);
	strncpy(msg, tmp_string, tmp_length);
	free(tmp_string);
	return msg;
}

char *create_create_message(const char *filename, const char *content) {
	char *msg = "CREATE ";
	char *content_msg = create_content_message(filename, content);
	append_strings(msg, content_msg, &msg);
	free(content_msg);
	return msg;
}

void test_create_create_message() {
	char *filename = "file3";
	char *content = "Test content";
	char *msg = create_create_message(filename, content);
	CU_ASSERT_STRING_EQUAL(msg, "CREATE file3 13\nTest content\n");
	free(msg);
}

char *create_update_message(const char *filename, const char *content) {
	char *msg = "UPDATE ";
	char *content_msg = create_content_message(filename, content);
	char *msg_final;
	append_strings(msg, content_msg, &msg_final);
	free(content_msg);
	return msg_final;
}

void test_create_update_message() {
	char *filename = "file3";
	char *content = "Test content";
	char *msg = create_update_message(filename, content);
	CU_ASSERT_STRING_EQUAL(msg, "UPDATE file3 13\nTest content\n");
	free(msg);
}

char *create_read_message(const char *filename) {
	char *msg = "READ ";
	char *msg_with_name;
	append_strings(msg, filename, &msg_with_name);
	append_strings(msg_with_name, "\n", &msg);
	free(msg_with_name);
	return msg;
}

void test_create_read_message() {
	char *filename = "file3";
	char *msg = create_read_message(filename);
	CU_ASSERT_STRING_EQUAL(msg, "READ file3\n");
	free(msg);
}

char *create_delete_message(const char *filename) {
	char *msg = "DELETE ";
	char *msg_with_name;
	append_strings(msg, filename, &msg_with_name);
	append_strings(msg_with_name, "\n", &msg);
	free(msg_with_name);
	return msg;
}

void test_create_delete_message() {
	char *filename = "file3";
	char *msg = create_delete_message(filename);
	CU_ASSERT_STRING_EQUAL(msg, "DELETE file3\n");
	free(msg);
}

char *create_numbered_filename(const char *filename_base, size_t num) {
	char num_string[15];
	sprintf(num_string, "%zu", num);
	char *filename;
	append_strings(filename_base, num_string, &filename);
	return filename;
}

void test_create_numbered_filename() {
	char *filename_base = "file_test";
	size_t num = 12;
	char *filename = create_numbered_filename(filename_base, num);
	CU_ASSERT_STRING_EQUAL(filename, "file_test12");
	free(filename);
}

char *create_create_message_numbered(const char *filename_base, size_t num, const char *file_content) {
	char *filename = create_numbered_filename(filename_base, num);
	char *msg = create_create_message(filename, file_content);
	free(filename);
	return msg;
}

void test_create_create_message_numbered() {
	char *filename_base = "file_test";
	size_t num = 12;
	char *content = "Test content";
	char *msg = create_create_message_numbered(filename_base, num, content);
	CU_ASSERT_STRING_EQUAL(msg, "CREATE file_test12 13\nTest content\n");
	free(msg);
}

char *create_update_message_numbered(const char *filename_base, size_t num, const char *file_content) {
	char *filename = create_numbered_filename(filename_base, num);
	char *msg = create_update_message(filename, file_content);
	free(filename);
	return msg;
}

void test_create_update_message_numbered() {
	char *filename_base = "file_test";
	size_t num = 12;
	char *content = "Test content";
	char *msg = create_update_message_numbered(filename_base, num, content);
	CU_ASSERT_STRING_EQUAL(msg, "UPDATE file_test12 13\nTest content\n");
	free(msg);
}

char *create_read_message_numbered(const char *filename_base, size_t num) {
	char *filename = create_numbered_filename(filename_base, num);
	char *msg = create_read_message(filename);
	free(filename);
	return msg;
}

void test_create_read_message_numbered() {
	char *filename_base = "file_test";
	size_t num = 12;
	char *msg = create_read_message_numbered(filename_base, num);
	CU_ASSERT_STRING_EQUAL(msg, "READ file_test12\n");
	free(msg);
}

char *create_delete_message_numbered(const char *filename_base, size_t num) {
	char *filename = create_numbered_filename(filename_base, num);
	char *msg = create_delete_message(filename);
	free(filename);
	return msg;
}

void test_create_delete_message_numbered() {
	char *filename_base = "file_test";
	size_t num = 12;
	char *msg = create_delete_message_numbered(filename_base, num);
	CU_ASSERT_STRING_EQUAL(msg, "DELETE file_test12\n");
	free(msg);
}

void message_creator_test_suite() {
	CU_pSuite message_creator_suite = CU_add_suite("message_creator", NULL, NULL);
	CU_add_test(message_creator_suite, "test_create_create_message", test_create_create_message);
	CU_add_test(message_creator_suite, "test_create_update_message", test_create_update_message);
	CU_add_test(message_creator_suite, "test_create_read_message", test_create_read_message);
	CU_add_test(message_creator_suite, "test_create_delete_message", test_create_delete_message);

	CU_add_test(message_creator_suite, "test_create_numbered_filename", test_create_numbered_filename);
	CU_add_test(message_creator_suite, "test_create_create_message_numbered", test_create_create_message_numbered);
	CU_add_test(message_creator_suite, "test_create_update_message_numbered", test_create_update_message_numbered);
	CU_add_test(message_creator_suite, "test_create_read_message_numbered", test_create_read_message_numbered);
	CU_add_test(message_creator_suite, "test_create_delete_message_numbered", test_create_delete_message_numbered);
}
