/* (C) IT Sky Consulting GmbH 2014
 * http://www.it-sky-consulting.com/
 * Author: Karl Brodowsky
 * Date: 2014-02-27
 * License: GPL v2 (See https://de.wikipedia.org/wiki/GNU_General_Public_License )
 */

/*
 * library functions for transmission of strings through pipes, sockets and the like.
 * Just as an example how this could be achieved...
 */

#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() and inet_ntoa() */
#include <errno.h>
#include <stdio.h>      /* for printf() and fprintf() and ... */
#include <stdlib.h>     /* for atoi() and exit() and ... */
#include <string.h>     /* for memset() and ... */
#include <sys/types.h>
#include <unistd.h>     /* for close() */
#include <regex.h>

#include <regex-handle.h>
#include <file-linked-list.h>
#include <serverlib.h>
#include <logger.h>
#include <transmission-protocols.h>

const char *EMPTY_BUFFER = "";

/* transmit a string over a socket or pipe connnection
 * if len is given it is assumed to be the lenght of the string
 * if it is -1, the length is found out with strlen()
 * The length of string is transmitted first as 4 byte unsigned integer,
 * followed by the string itself.
 * @param client_socket  a socket or pipe.  Could be a file also.
 * @param str string to be transmitted
 * @param len length of string to be transmitted
 */
void write_string(int client_socket, char *str) {
	const int deep = 2;
	int len = strlen(str);

	int send_len = write(client_socket, str, len);
	handle_error(send_len, "could not write", THREAD_EXIT);

	if(send_len != len){
		error(deep,"Write to client socket failed");
		exit_by_type(THREAD_EXIT);
	}
}

void write_eot(int client_socket) {
	write_string(client_socket, "");
}

/* the caller has to free the buffer, unless ulen == 0 */
size_t read_and_store_string(int client_socket, char **result) {
	const int deep = 2;
	char buffer[MAX_MESSAGE_LEN + 1];
	size_t bytes_received = 0;

	debug(deep,"Read from client socket");
	bytes_received = read(client_socket, buffer, MAX_MESSAGE_LEN);
	handle_error(bytes_received,
			"recv() failed or connection closed prematurely", THREAD_EXIT);
	debug(deep,"Received bytes: %zu", bytes_received);

	if (bytes_received == 0) {
		debug(deep,"Return empty buffer");
		*result = (char *) EMPTY_BUFFER; /* actually the same as empty string */
		return bytes_received;
	}

	buffer[bytes_received] = '\000';

	*result = (char *) malloc(bytes_received + 1);
	strncpy(*result, buffer, bytes_received + 1);
	return bytes_received;
}

char *select_strategy(const char *msg){
	int deep = 1;
	debug(deep, "Received string: '%s'", msg);
	info(deep, "Select strategy");
	unsigned char action = msg[0];

	char *result;

	switch (action) {
	case COMMAND_CREATE:
		result = create_file(msg);
		break;
	case COMMAND_UPDATE:
		result = update_file(msg);
		break;
	case COMMAND_DELETE:
		result = delete_file(msg);
		break;
	case COMMAND_READ:
		result = read_file(msg);
		break;
	case COMMAND_LIST:
		result = list_files(msg);
		break;
	default:
		result = ANSWER_UNKOWN;
		error(deep, "Wrong action %c", action);
	}

	info(deep, "Return value: '%s'", result);
	return result;
}

char *create_file(const char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];
	char org_length_str[5];
	char content[4096];

	const char * create_regex_text =
			"CREATE[[:blank:]]+([[:graph:]|[:blank:]]+)[[:blank:]]+([[:digit:]]+)[[:cntrl:]]+([[:graph:]|[:blank:]]+)";
	compile_regex(&r, create_regex_text);
	int retCode = match_regex(&r, msg, filename, org_length_str, content);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message '%s' does not match to regex!", msg);
		return ANSWER_UNKOWN;
	}
	int orig_length = atoi(org_length_str);
	debug(deep, "Filename: %s", filename);
	debug(deep, "Length: %d", orig_length);
	debug(deep, "Content: %s", content);

	info(deep, "Create file %s", filename);
	int length = strlen(content) + 1;
	if (length != orig_length) {
		error(deep, "Message length is not correct!");
		return ANSWER_INVALID;
	}

	if (add_memory_file(filename, length, content)) {
		return ANSWER_SUCCESS_CREATE;
	} else {
		debug(deep, "File '%s' already exist", filename);
		return ANSWER_FAILED_CREATE;
	}
}

char *update_file(const char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];
	char org_length_str[5];
	char content[4096];

	const char *update_regex_text =
			"UPDATE[[:blank:]]+([[:graph:]|[:blank:]]+)[[:blank:]]+([[:digit:]]+)[[:cntrl:]]+([[:graph:]|[:blank:]]+)";
	compile_regex(&r, update_regex_text);
	int retCode = match_regex(&r, msg, filename, org_length_str, content);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message '%s' does not match to regex!", msg);
		return ANSWER_UNKOWN;
	}
	int orig_length = atoi(org_length_str);
	debug(deep, "Filename: %s", filename);
	debug(deep, "Length: %d", orig_length);
	debug(deep, "Content: %s", content);

	int length = strlen(content) + 1;
	if (length != orig_length) {
		error(deep, "Message length is not correct!");
		return ANSWER_INVALID;
	}

	if (update_memory_file(filename, length, content)) {
		return ANSWER_SUCCESS_UPDATE;
	}
	return ANSWER_FAILED_UPDATE;
}

char *delete_file(const char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];

	const char *delete_regex_text = "DELETE[[:blank:]]+([[:graph:]|[:blank:]]+)[[:cntrl:]]+";
	compile_regex(&r, delete_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message '%s' does not match to regex!", msg);
		return ANSWER_UNKOWN;
	}

	info(deep, "Delete file %s", filename);
	if (delete_memory_file(filename)) {
		return ANSWER_SUCCESS_DELETE;
	}
	return ANSWER_FAILED_DELETE;
}

char *read_file(const char *msg) {
	const int deep = 2;
	regex_t r;
	char filename[4096];

	const char *read_regex_text = "READ[[:blank:]]+([[:graph:]|[:blank:]]+)[[:cntrl:]]+";
	compile_regex(&r, read_regex_text);
	int retCode = match_regex(&r, msg, filename, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message '%s' does not match to regex!", msg);
		return ANSWER_UNKOWN;
	}

	char *returnValue;
	char *content;
	if (read_memory_file(filename, &content)) {
		if (content != NULL) {
			debug(deep, "Content of file: %s", content);
			int length = strlen(content) + 1;
			char len_string[15];
			sprintf(len_string, " %d\n", length);
			char *rv = append_strings(ANSWER_SUCCESS_READ, filename);
			rv = append_strings(rv, len_string);
			rv = append_strings(rv, content);
			rv = append_strings(rv, "\n");
			returnValue = rv;
		} else {
			error(deep, "Could not read file");
			returnValue = ANSWER_INTERNAL_ERROR;
		}
	} else {
		returnValue = ANSWER_FAILED_READ;
	}
	free(content);
	return returnValue;
}

char *list_files(const char *msg) {
	const int deep = 2;
	regex_t r;

	const char *list_regex_text = "LIST[[:cntrl:]]+";
	compile_regex(&r, list_regex_text);
	int retCode = match_regex(&r, msg, NULL, NULL, NULL);
	regfree(&r);
	if (!retCode) {
		error(deep, "Message '%s' does not match to regex!", msg);
		return ANSWER_UNKOWN;
	}

	info(deep, "List files");
	char *file_list;
	int file_counter = list_memory_file(&file_list);

	//debug(deep, "Output from list method: %s", file_list);
	char str[15];
	sprintf(str, "%d", file_counter);
	char *rv = append_strings(ANSWER_SUCCESS_LIST, str);
	rv = append_strings(rv, "\n");
	if(file_counter > 0){
		rv = append_strings(rv, file_list);
		rv = append_strings(rv, "\n");
	}

	free(file_list);
	return rv;
}
