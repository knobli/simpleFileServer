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

struct memory_file* create_memory_file(const char *filename, const int length, const char *content) {
	const int deep = 4;
	debug(deep, "Create new memory file");
	struct memory_file *file = (struct memory_file*) malloc(sizeof(struct memory_file));
	if (NULL == file) {
		error(deep, "Node creation failed");
		return NULL;
	}
	size_t filename_size = strlen(filename) + 1;
	file->filename = malloc(filename_size);
	strncpy(file->filename, filename, filename_size);
	size_t content_size = length;
	file->content = malloc(content_size);
	strncpy(file->content, content, content_size);
	file->length = length;
	file->next = NULL;

	int returnCode;
	debug(deep, "Init link mod mutex");
	pthread_mutex_t mutex;
	memset(&mutex, 0, sizeof(mutex)); /* Zero out structure */
	/* pthread_mutexattr_t psharedm;
	 pthread_mutexattr_init(&psharedm);
	 pthread_mutexattr_setpshared(&psharedm, PTHREAD_MUTEX_ERRORCHECK); */
	returnCode = pthread_mutex_init(&mutex, NULL);
	handle_thread_error(returnCode, "Could not init link mod mutex", THREAD_EXIT);
	file->link_mod_mutex = mutex;

	debug(deep, "Init rwlock mutex");
	pthread_rwlock_t rwlock;
	memset(&rwlock, 0, sizeof(rwlock)); /* Zero out structure */
	/*	pthread_rwlockattr_t rwlock_attr;
	 pthread_rwlockattr_init(&rwlock_attr);
	 pthread_rwlockattr_setpshared(&rwlock_attr, PTHREAD_MUTEX_ERRORCHECK); */
	returnCode = pthread_rwlock_init(&rwlock, NULL);
	handle_thread_error(returnCode, "Could not init rwlock mutex", THREAD_EXIT);
	file->rwlock = rwlock;

	finest(deep, "New memory file %p created", file);
	return file;
}

bool init_linked_list() {
	int deep = 1;
	debug(deep, "Init file linked list");
	head = create_memory_file("", 0, "");
	return true;
}

struct memory_file* search_file(const char *filename, struct memory_file **prev) {
	const int deep = 4;
	int returnCode;
	struct memory_file *ptr = head;
	struct memory_file *last = NULL;
	struct memory_file *second_last = NULL;
	bool found = false;
	info(deep, "Searching file '%s' in the list", filename);
	while (ptr != NULL) {
		//unlock second last file
		if (second_last != NULL) {
			finest(deep, "Unlock second last link mod mutex on %p - search case", second_last);
			returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not unlock second last link mod mutex - search", THREAD_EXIT);
		}

		//Lock current file
		finest(deep, "Lock current link mod mutex on %p - search case", ptr);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex - search", THREAD_EXIT);

		//compare file names
		if (strcmp(ptr->filename, filename) == 0) {
			debug(deep, "File '%s' found!", filename);
			found = true;

			//unlock current file, because only pointer to current file has to be locked
			finest(deep, "Unlock matching link mod mutex on %p - search case", ptr);
			returnCode = pthread_mutex_unlock(&ptr->link_mod_mutex);
			handle_thread_error(returnCode, "Could not unlock matching link mod mutex - search", THREAD_EXIT);
			break;
		} else {
			//file name does not match, go one step forward
			second_last = last;
			last = ptr;
			ptr = ptr->next;
		}
	}

	//unlock second_last after while loop
	if (second_last != NULL) {
		finest(deep, "Unlock second last link mod mutex on %p - search case", second_last);
		returnCode = pthread_mutex_unlock(&second_last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not unlock second last link mod mutex out of loop - search", THREAD_EXIT);
	}

	finest(deep, "Set previous: %p", last);
	*prev = last;

	if (true == found) {
		return ptr;
	} else {
		return NULL;
	}
}

bool add_memory_file(const char *filename, const size_t length, const char *content) {
	const int deep = 3;
	bool rv;
	struct memory_file *ptr = create_memory_file(filename, length, content);
	struct memory_file *endNode = NULL;
	struct memory_file *file = search_file(filename, &endNode);
	if (file == NULL) {
		info(deep, "Adding file '%s' to the end of the list", filename);
		finest(deep, "Set next pointer of %p to new file %p", endNode, ptr);
		endNode->next = ptr;
		rv = true;
	} else {
		info(deep, "File '%s' already exist", filename);
		rv = false;
	}

	finest(deep, "Unlock list mod mutex on %p - create case", endNode);
	int returnCode = pthread_mutex_unlock(&endNode->link_mod_mutex);
	handle_thread_error(returnCode, "Could not unlock list mod mutex - create", THREAD_EXIT);
	return rv;
}

bool update_memory_file(const char *filename, const size_t length, const char *content) {
	const int deep = 3;
	int returnCode;
	bool rv;
	info(deep, "Update file %s", filename);
	struct memory_file *prev_file = NULL;
	struct memory_file* file = search_file(filename, &prev_file);
	if (file != NULL) {
		finest(deep, "Lock rw mutex on %p - update case", file);
		returnCode = pthread_rwlock_wrlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex", THREAD_EXIT);

		finest(deep, "Unlock list mod mutex on %p - update case", prev_file);
		int returnCode = pthread_mutex_unlock(&prev_file->link_mod_mutex);
		handle_thread_error(returnCode, "Could not unlock list mod mutex - update", THREAD_EXIT);

		debug(deep, "Update content of file '%s'", filename);
		file->length = length;
		char *saved_content = file->content;
		file->content = malloc(length);
		strncpy(file->content, content, length);
		free(saved_content);

		finest(deep, "Unlock rw mutex on %p - update case", file);
		returnCode = pthread_rwlock_unlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not unlock rw mutex", THREAD_EXIT);
		rv = true;
	} else {
		finest(deep, "Unlock list mod mutex on %p - update case", prev_file);
		int returnCode = pthread_mutex_unlock(&prev_file->link_mod_mutex);
		handle_thread_error(returnCode, "Could not unlock list mod mutex - update", THREAD_EXIT);
		rv = false;
	}
	return rv;
}

bool read_memory_file(const char *filename, char **content) {
	const int deep = 3;
	int returnCode;
	bool rv;
	info(deep, "Read file %s", filename);
	struct memory_file *prev_file = NULL;
	struct memory_file* file = search_file(filename, &prev_file);
	if (file != NULL) {
		finest(deep, "Lock rw mutex on %p - read case", file);
		returnCode = pthread_rwlock_wrlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex - read", THREAD_EXIT);

		finest(deep, "Unlock list mod mutex on %p - read case", prev_file);
		int returnCode = pthread_mutex_unlock(&prev_file->link_mod_mutex);
		handle_thread_error(returnCode, "Could not unlock list mod mutex - read", THREAD_EXIT);

		debug(deep, "Read content %s", file->content);
		size_t content_length = file->length + 1;
		*content = malloc(content_length);
		strncpy(*content, file->content, file->length + 1);

		finest(deep, "Unlock rw mutex on %p - read case", file);
		returnCode = pthread_rwlock_unlock(&file->rwlock);
		handle_thread_error(returnCode, "Could not unlock rw mutex - read", THREAD_EXIT);

		rv = true;
	} else {
		finest(deep, "Unlock list mod mutex on %p - read case", prev_file);
		int returnCode = pthread_mutex_unlock(&prev_file->link_mod_mutex);
		handle_thread_error(returnCode, "Could not unlock list mod mutex - read", THREAD_EXIT);
		*content = NULL;
		rv = false;
	}
	return rv;
}

bool delete_memory_file(const char* filename) {
	const int deep = 3;
	int returnCode;
	bool rv = true;
	struct memory_file *prev = NULL;
	struct memory_file *del = NULL;

	info(deep, "Deleting file '%s' from list", filename);

	del = search_file(filename, &prev);
	if (del == NULL) {
		rv = false;
	} else {
		prev->next = del->next;
	}
	finest(deep, "Unlock link mod mutex on %p - delete case", prev);
	returnCode = pthread_mutex_unlock(&prev->link_mod_mutex);
	handle_thread_error(returnCode, "Could not unlock link mod mutex - delete", THREAD_EXIT);
	if (rv) {
		finest(deep, "Lock rw mutex on %p - delete case", del);
		returnCode = pthread_rwlock_wrlock(&del->rwlock);
		handle_thread_error(returnCode, "Could not lock rw mutex - delete", THREAD_EXIT);

		finest(deep, "Unlock rw mutex on %p - delete case", del);
		returnCode = pthread_rwlock_unlock(&del->rwlock);
		handle_thread_error(returnCode, "Could not unlock rw mutex - delete", THREAD_EXIT);

		/*
		 * TODO: destroy mutex

		 finest(deep, "Lock link mod mutex on %p - delete case", del);
		 returnCode = pthread_mutex_lock(&del->link_mod_mutex);
		 handle_thread_error(returnCode, "Could not lock link mod mutex - delete", THREAD_EXIT);

		 finest(deep, "Unlock link mod mutex on %p - delete case", del);
		 returnCode = pthread_mutex_unlock(&del->link_mod_mutex);
		 handle_thread_error(returnCode, "Could not unlock link mod mutex - delete", THREAD_EXIT);

		 returnCode = pthread_mutex_destroy(&del->link_mod_mutex);
		 handle_thread_error(returnCode, "Could not destroy link mod mutex - delete", THREAD_EXIT);

		 returnCode = pthread_rwlock_destroy(&del->rwlock);
		 handle_thread_error(returnCode, "Could not destroy rw mutex - delete", THREAD_EXIT);

		 */

		debug(deep, "Free filename");
		free(del->filename);
		debug(deep, "Free content");
		free(del->content);
		debug(deep, "Free file");
		free(del);
		del = NULL;
	}

	return rv;
}

int list_memory_file(char **file_list) {
	const int deep = 3;
	struct memory_file *ptr = head;
	struct memory_file *last = NULL;
	int returnCode;
	int file_counter = 0;
	char *tmp_file_list = malloc(1);
	tmp_file_list[0] = '\000';
	char *tmp_string;
	size_t tmp_length;
	info(deep, "Go trough files");
	while (ptr != NULL) {
		if (last != NULL) {
			finest(deep, "Unlock last link mod mutex on %p - list case", last);
			returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
			handle_thread_error(returnCode, "Could not unlock last link mod mutex", THREAD_EXIT);
		}
		finest(deep, "Lock link mod mutex on %p - list case", ptr);
		returnCode = pthread_mutex_lock(&ptr->link_mod_mutex);
		handle_thread_error(returnCode, "Could not lock link mod mutex", THREAD_EXIT);

		if (strcmp(ptr->filename, "") != 0) {
			char *filename = ptr->filename;
			debug(deep, "Found file '%s'", filename);
			if (file_counter == 0) {
				tmp_length = append_strings(tmp_file_list, filename, &tmp_string);

				free(tmp_file_list);
				tmp_file_list = malloc(tmp_length);
				strncpy(tmp_file_list, tmp_string, tmp_length);
				free(tmp_string);
			} else {
				tmp_length = append_strings(tmp_file_list, "\n", &tmp_string);

				free(tmp_file_list);
				tmp_file_list = malloc(tmp_length);
				strncpy(tmp_file_list, tmp_string, tmp_length);
				free(tmp_string);

				tmp_length = append_strings(tmp_file_list, filename, &tmp_string);

				free(tmp_file_list);
				tmp_file_list = malloc(tmp_length);
				strncpy(tmp_file_list, tmp_string, tmp_length);
				free(tmp_string);
			}
			file_counter++;
		}

		last = ptr;
		ptr = ptr->next;
	}
	if (last != NULL) {
		finest(deep, "Unlock last link mod mutex on %p - list case", last);
		returnCode = pthread_mutex_unlock(&last->link_mod_mutex);
		handle_thread_error(returnCode, "Could not unlock last link mod mutex out of loop", THREAD_EXIT);
	}
	size_t list_length = strlen(tmp_file_list) + 1;
	*file_list = malloc(list_length);
	strncpy(*file_list, tmp_file_list, list_length);
	free(tmp_file_list);
	return file_counter;
}
