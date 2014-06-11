/*
 * file-linked-list.c
 *
 *  Created on: 06.06.2014
 *      Author: knobli
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <logger.h>
#include <serverlib.h>
#include <file-linked-list.h>

struct memory_file {
	char* filename;
	int length;
	char* content;
	pthread_mutex_t link_mod_mutex;
	pthread_rwlock_t rwlock;
	struct memory_file *next;
};

struct memory_file *head = NULL;

struct memory_file* create_memory_file(char *filename, int length,
		char *content) {
	const int deep = 4;
	debug(deep, "Create new memory file");
	struct memory_file *file = (struct memory_file*) malloc(
			sizeof(struct memory_file));
	if (NULL == file) {
		error(deep, "Node creation failed");
		return NULL;
	}
	size_t filename_size = strlen(filename) + 1;
	file->filename = malloc(filename_size);
	strncpy(file->filename,filename,filename_size);
	size_t content_size = length + 1;
	file->content = malloc(content_size);
	strncpy(file->content,content,content_size);
	file->length = length;
	file->next = NULL;

	int returnCode;
	debug(deep, "Init link mod mutex");
	pthread_mutex_t mutex;
	returnCode = pthread_mutex_init(&mutex, NULL);
	handle_thread_error(returnCode, "Could not init link mod mutex",
			THREAD_EXIT);
	file->link_mod_mutex = mutex;

	debug(deep, "Init rwlock mutex");
	pthread_rwlock_t rwlock;
	returnCode = pthread_rwlock_init(&rwlock, NULL);
	handle_thread_error(returnCode, "Could not init rwlock mutex", THREAD_EXIT);
	file->rwlock = rwlock;

	debug(deep, "New memory file %p created", file);
	return file;
}

void init_linked_list() {
	int deep = 1;
	debug(deep, "Init file linked list");
	head = create_memory_file("", 0, "");
}

struct memory_file* search_file(char *filename, struct memory_file **prev,
		int lock) {
	const int deep = 4;
	int returnCode;
	struct memory_file *ptr = head;
	struct memory_file *last = NULL;
	struct memory_file *second_last = NULL;
	bool found = false;
	info(deep, "Searching file '%s' in the list", filename);
	while (ptr != NULL) {
		if (second_last != NULL && lock) {
			debug(deep, "Unlock second last link mod mutex on %p - search case",
					second_last);
			//debug(deep, "Link mod mutex %p - search case", &second_last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
			handle_thread_error(returnCode,
					"Could not release link mod mutex - search", THREAD_EXIT);
		}
		if (last != NULL && !lock) {
			debug(deep, "Unlock last link mod mutex on %p - search case", last);
			//debug(deep, "Link mod mutex %p - search case", &last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode,
					"Could not release link mod mutex - search", THREAD_EXIT);
		}
		debug(deep, "Lock current link mod mutex on %p - search case", ptr);
		//debug(deep, "Link mod mutex %p - search case", &ptr->link_mod_mutex);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode,
				"Could not lock link mod mutex - search", THREAD_EXIT);
		if (strcmp(ptr->filename, filename) == 0) {
			debug(deep, "File '%s' found!", filename);
			found = true;

			if(lock == true){
				debug(deep, "Unlock matching link mod mutex on %p - search case", ptr);
				//debug(deep, "Link mod mutex %p - search ptr",&ptr->link_mod_mutex);
				returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
				handle_thread_error(returnCode,
						"Could not release link mod mutex - search", THREAD_EXIT);
			}

			break;
		} else {
			debug(deep, "Set second_last to: %p", last);
			second_last = last;
			debug(deep, "Set last to: %p", ptr);
			last = ptr;
			debug(deep, "Set ptr to: %p", ptr->next);
			ptr = ptr->next;
		}
	}

	if (second_last != NULL && lock) {
		debug(deep, "Unlock second last link mod mutex on %p - search case",
				second_last);
		//debug(deep, "Link mod mutex %p - search case", &second_last->link_mod_mutex);
		returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
		handle_thread_error(returnCode,
				"Could not release link mod mutex - search", THREAD_EXIT);
	}

	if (prev) {
		debug(deep, "Set previous: %p", last);
		*prev = last;
	}

	if (true == found) {
		if (ptr != NULL && !lock) {
			debug(deep, "Unlock current link mod mutex on %p - search case",
					ptr);
			//debug(deep, "Link mod mutex %p - search case", &ptr->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
			handle_thread_error(returnCode,
					"Could not release link mod mutex - search", THREAD_EXIT);
		}
		return ptr;
	} else {
		if (last != NULL && !lock) {
			debug(deep, "Unlock last link mod mutex on %p - search case test",
					last);
			//debug(deep, "Link mod mutex %p - search case", &last->link_mod_mutex);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode,
					"Could not release link mod mutex - search", THREAD_EXIT);
		}
		return NULL;
	}
}

int add_memory_file(char *filename, int length, char *content) {
	const int deep = 3;
	int rv;
	struct memory_file *ptr = create_memory_file(filename, length, content);
	struct memory_file *endNode = NULL;
	struct memory_file *file = search_file(filename, &endNode, true);
	if (file == NULL) {
		info(deep, "Adding file '%s' to the end of the list", filename);
		debug(deep, "Set next pointer of %p to new file %p", endNode, ptr);
		endNode->next = ptr;
		rv = true;
	} else {
		info(deep, "File '%s' already exist", filename);
		rv = false;
	}

	debug(deep, "Unlock list mod mutex on %p - create case", endNode);
	int returnCode = pthread_mutex_unlock(&endNode->link_mod_mutex);
	handle_thread_error(returnCode, "Could not release list mod mutex - create",
			THREAD_EXIT);
	return rv;
}

int update_memory_file(char *filename, int length, char *content) {
	const int deep = 3;
	int returnCode;
	int rv;
	info(deep, "Update file %s", filename);
	struct memory_file* file = search_file(filename, NULL, false);
	if (file != NULL) {
		debug(deep, "Lock rw mutex on %p - update case", file);
		returnCode = pthread_rwlock_wrlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex", THREAD_EXIT);

		file->length = length;
		char *saved_content = file->content;
		file->content = malloc(length + 1);
		strncpy(file->content,content,length + 1);
		free(saved_content);

		debug(deep, "Release rw mutex on %p - update case", file);
		returnCode = pthread_rwlock_unlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not release rw mutex",
				THREAD_EXIT);
		rv = true;
	} else {
		rv = false;
	}
	return rv;
}

int read_memory_file(char *filename, char *content) {
	const int deep = 3;
	int returnCode;
	int rv;
	info(deep, "Read file %s", filename);
	struct memory_file* file = search_file(filename, NULL, FALSE);
	if (file != NULL) {
		debug(deep, "Lock rw mutex on %p - read case", file);
		//debug(deep, "Lock rw mutex %p", &file->rwlock);
		returnCode = pthread_rwlock_wrlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex - read",
				THREAD_EXIT);
		debug(deep, "Read content %s", file->content);
		strncpy(content, file->content, file->length + 1);
		debug(deep, "Release rw mutex on %p - read case", file);
		returnCode = pthread_rwlock_unlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not release rw mutex - read",
				THREAD_EXIT);
		rv = true;
	} else {
		content = NULL;
		rv = false;
	}
	return rv;
}

int delete_memory_file(char* filename) {
	const int deep = 3;
	struct memory_file *prev = NULL;
	struct memory_file *del = NULL;

	info(deep, "Deleting file '%s' from list", filename);

	int returnCode;
	int rv = 0;

	del = search_file(filename, &prev, true);
	if (del == NULL) {
		rv = -1;
	} else {
		prev->next = del->next;
	}
	debug(deep, "Unlock link mod mutex on %p - delete case", prev);
	returnCode = pthread_mutex_unlock(&prev->link_mod_mutex);
	handle_thread_error(returnCode, "Could not release link mod mutex - delete",
			THREAD_EXIT);
	if (rv != -1) {
		debug(deep, "Free filename");
		free(del->filename);
		debug(deep, "Free content");
		free(del->content);
		debug(deep, "Free file");
		free(del);
		del = NULL;
		rv = 0;
	}

	return rv;
}

int list_memory_file(char **file_list) {
	const int deep = 3;
	struct memory_file *ptr = head;
	struct memory_file *last = NULL;
	int returnCode;
	int file_counter = 0;
	char *tmp_file_list = "";
	info(deep, "Go trough files");
	while (ptr != NULL) {
		if (last != NULL) {
			debug(deep, "Release last link mod mutex on %p - list case", last);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not release link mod mutex",
					THREAD_EXIT);
		}
		debug(deep, "Lock link mod mutex on %p - list case", ptr);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex",
				THREAD_EXIT);

		if (strcmp(ptr->filename, "") != 0) {
			char *filename = ptr->filename;
			debug(deep, "Found file '%s'", filename);
			if (file_counter == 0) {
				tmp_file_list = append_strings(tmp_file_list, filename);
			} else {
				tmp_file_list = append_strings(tmp_file_list, "\n");
				tmp_file_list = append_strings(tmp_file_list, filename);
			}
			file_counter++;
		}

		last = ptr;
		ptr = ptr->next;
	}
	if (last != NULL) {
		debug(deep, "Release last link mod mutex on %p - list case", last);
		returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not release link mod mutex",
				THREAD_EXIT);
	}
	size_t list_length = strlen(tmp_file_list) + 1;
	*file_list = malloc(list_length);
	strncpy(*file_list, tmp_file_list, list_length);

	return file_counter;
}
