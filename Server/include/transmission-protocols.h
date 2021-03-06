/*
 * transmission-protocols.c
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define COMMAND_LIST 'L'
#define COMMAND_CREATE 'C'
#define COMMAND_READ 'R'
#define COMMAND_UPDATE 'U'
#define COMMAND_DELETE 'D'

/* ACK 2\n */
#define ANSWER_SUCCESS_LIST "ACK "
#define ANSWER_FAILED_CREATE "FILEEXISTS\n"
#define ANSWER_SUCCESS_CREATE "FILECREATED\n"
#define ANSWER_FAILED_READ "NOSUCHFILE\n"
/* FILECONTENT __FILENAME__\n */
#define ANSWER_SUCCESS_READ "FILECONTENT "
#define ANSWER_FAILED_UPDATE "NOSUCHFILE\n"
#define ANSWER_SUCCESS_UPDATE "UPDATED\n"
#define ANSWER_FAILED_DELETE "NOSUCHFILE\n"
#define ANSWER_SUCCESS_DELETE "DELETED\n"
#define ANSWER_INVALID "INPUT INVALID\n"
#define ANSWER_UNKOWN "UNKOWN COMMAND\n"
#define ANSWER_INTERNAL_ERROR "INTERNAL ERROR\n"

#define MAX_MESSAGE_LEN 4096
#define MAX_LENGTH_NUM 5

void write_string(int client_socket, char *str);

void write_eot(int client_socket);

/* the caller has to free the buffer, unless ulen == 0 */
size_t read_and_store_string(int client_socket, char **result);

char *select_strategy(const char *msg);
char *create_file(const char *msg);
char *update_file(const char *msg);
char *delete_file(const char *msg);
char *read_file(const char *msg);
char *list_files(const char *msg);

#endif
