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
	char *msg = "";
	msg = append_strings(msg, filename);
	size_t length = strlen(content);
	char len_string[15];
	sprintf(len_string, " %zu\n", length);
	msg = append_strings(msg, len_string);
	msg = append_strings(msg, content);
	msg = append_strings(msg, "\n");
	return msg;
}

char *create_create_message(const char *filename, const char *content) {
	char *msg = "CREATE ";
	msg = append_strings(msg, create_content_message(filename, content));
	return msg;
}

void test_create_create_message() {
	char *filename = "file3";
	char *content = "Test content";
	char *msg = create_create_message(filename, content);
	CU_ASSERT_STRING_EQUAL(msg, "CREATE file3 12\nTest content\n");
	free(msg);
}

char *create_update_message(const char *filename, const char *content) {
	char *msg = "UPDATE ";
	msg = append_strings(msg, create_content_message(filename, content));
	return msg;
}

void test_create_update_message() {
	char *filename = "file3";
	char *content = "Test content";
	char *msg = create_update_message(filename, content);
	CU_ASSERT_STRING_EQUAL(msg, "UPDATE file3 12\nTest content\n");
	free(msg);
}

char *create_read_message(const char *filename) {
	char *msg = "READ ";
	msg = append_strings(msg, filename);
	msg = append_strings(msg, "\n");
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
	msg = append_strings(msg, filename);
	msg = append_strings(msg, "\n");
	return msg;
}

void test_create_delete_message() {
	char *filename = "file3";
	char *msg = create_delete_message(filename);
	CU_ASSERT_STRING_EQUAL(msg, "DELETE file3\n");
	free(msg);
}

char *create_numbered_filename(const char *filename_base, size_t num) {
	char *filename = "";
	filename = append_strings(filename, filename_base);
	char num_string[15];
	sprintf(num_string, "%zu", num);
	filename = append_strings(filename, num_string);
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
	CU_ASSERT_STRING_EQUAL(msg, "CREATE file_test12 12\nTest content\n");
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
	CU_ASSERT_STRING_EQUAL(msg, "UPDATE file_test12 12\nTest content\n");
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
