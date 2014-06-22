/*
 * file-linked-list-test.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <logger.h>
#include <file-linked-list.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

const char *test_filename1 = "Test file";
const char *test_file1_content = "Test content";
const char *test_file1_content_up = "new";

const char *test_filename2 = "Test file2";
const char *test_file2_content = "Test content2";
const char *test_file2_content_up = "new2";

const char *filename_special_chars = "Test _ das  / *\" '^";
const char *content_special_chars = "_ das / *\" content '^";

void test_add_memory_file() {
	CU_ASSERT_TRUE(init_linked_list());
	CU_ASSERT_TRUE(add_memory_file(test_filename1, strlen(test_file1_content) + 1, test_file1_content));
}

void test_update_memory_file() {
	CU_ASSERT_TRUE(update_memory_file(test_filename1, strlen(test_file1_content_up) + 1, test_file1_content_up));
}

void test_read_memory_file() {
	char *content;
	CU_ASSERT_TRUE(read_memory_file(test_filename1, &content));
	CU_ASSERT_STRING_EQUAL(content, test_file1_content_up);
	free(content);

}

void test_delete_memory_file() {
	CU_ASSERT_TRUE(delete_memory_file(test_filename1));
}

void test_failure_cases_list() {
	//add file twice
	CU_ASSERT_TRUE(add_memory_file(test_filename1, strlen(test_file1_content) + 1, test_file1_content));
	CU_ASSERT_FALSE(add_memory_file(test_filename1, strlen(test_file1_content_up) + 1, test_file1_content_up));
	CU_ASSERT_TRUE(delete_memory_file(test_filename1));

	CU_ASSERT_FALSE(update_memory_file(test_filename2, strlen(test_file2_content_up) + 1, test_file2_content_up));

	char *content;
	CU_ASSERT_FALSE(read_memory_file(test_filename2, &content));
	CU_ASSERT_PTR_NULL(content);
	free(content);

	CU_ASSERT_FALSE(delete_memory_file(test_filename2));
}

void test_list_memory_files() {
	CU_ASSERT_TRUE(add_memory_file(test_filename1, strlen(test_file1_content) + 1, test_file1_content));
	CU_ASSERT_TRUE(add_memory_file(test_filename2, strlen(test_file2_content) + 1, test_file2_content));

	char *file_list;
	CU_ASSERT_EQUAL(list_memory_file(&file_list), 2);
	CU_ASSERT_STRING_EQUAL(file_list, "Test file\nTest file2");
	free(file_list);

	CU_ASSERT_TRUE(delete_memory_file(test_filename1));
	CU_ASSERT_TRUE(delete_memory_file(test_filename2));
}

void test_special_chars_list() {
	CU_ASSERT_TRUE(add_memory_file(filename_special_chars, strlen(content_special_chars) + 1, content_special_chars));
	CU_ASSERT_TRUE(update_memory_file(filename_special_chars, strlen(content_special_chars) + 1, content_special_chars));

	char *content;
	CU_ASSERT_TRUE(read_memory_file(filename_special_chars, &content));
	CU_ASSERT_STRING_EQUAL(content, content_special_chars);
	free(content);

	char *file_list;
	CU_ASSERT_EQUAL(list_memory_file(&file_list), 1);
	CU_ASSERT_STRING_EQUAL(file_list, filename_special_chars);
	free(file_list);

	CU_ASSERT_TRUE(delete_memory_file(filename_special_chars));
}

void test_destroy_list() {
	CU_ASSERT_TRUE(add_memory_file(test_filename1, strlen(test_file1_content) + 1, test_file1_content));
	CU_ASSERT_TRUE(add_memory_file(test_filename2, strlen(test_file2_content) + 1, test_file2_content));

	CU_ASSERT_EQUAL(destroy_linked_list(), 2);
}

void file_list_test_suite() {
	CU_pSuite file_list_suite = CU_add_suite("file_list", NULL, NULL);
	CU_add_test(file_list_suite, "test_add_memory_file", test_add_memory_file);
	CU_add_test(file_list_suite, "test_update_memory_file", test_update_memory_file);
	CU_add_test(file_list_suite, "test_read_memory_file", test_read_memory_file);
	CU_add_test(file_list_suite, "test_delete_memory_file", test_delete_memory_file);
	CU_add_test(file_list_suite, "test_failure_cases", test_failure_cases_list);
	CU_add_test(file_list_suite, "test_list_memory_files", test_list_memory_files);
	CU_add_test(file_list_suite, "test_special_chars", test_special_chars_list);
	CU_add_test(file_list_suite, "test_destroy_list", test_destroy_list);
}
