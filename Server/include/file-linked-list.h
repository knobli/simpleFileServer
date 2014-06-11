/*
 * file-linked-list.h
 *
 *  Created on: 06.06.2014
 *      Author: knobli
 */

#ifndef FILE_LINKED_LIST_H_
#define FILE_LINKED_LIST_H_

void init_linked_list();

int add_memory_file(char *filename, int length, char *content);

int update_memory_file(char *filename, int length, char *content);

int read_memory_file(char *filename, char *content);

int delete_memory_file(char* filename);

int list_memory_file(char *file_list);


#endif /* FILE_LINKED_LIST_H_ */
