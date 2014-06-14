/*
 * file-linked-list.h
 *
 *  Created on: 06.06.2014
 *      Author: knobli
 */

#ifndef FILE_LINKED_LIST_H_
#define FILE_LINKED_LIST_H_

#include <stdbool.h>
#include <stddef.h>

bool init_linked_list();

bool add_memory_file(const char *filename, const size_t length, const char *content);

bool update_memory_file(const char *filename, const size_t length, const char *content);

bool read_memory_file(const char *filename, char **content);

bool delete_memory_file(const char *filename);

int list_memory_file(char **file_list);


#endif /* FILE_LINKED_LIST_H_ */
