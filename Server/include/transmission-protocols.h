/* (C) IT Sky Consulting GmbH 2014
 * http://www.it-sky-consulting.com/
 * Author: Karl Brodowsky
 * Date: 2014-02-27
 * License: GPL v2 (See https://de.wikipedia.org/wiki/GNU_General_Public_License )
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

#define MAX_MESSAGE_LEN 2500

typedef void (*consumer_function)(const char *buff, size_t count);


/* transmit a string over a socket or pipe connnection
 * if len is given it is assumed to be the lenght of the string
 * if it is -1, the length is found out with strlen()
 * The length of string is transmitted first as 4 byte unsigned integer,
 * followed by the string itself.
 * @param client_socket  a socket or pipe.  Could be a file also.
 * @param str string to be transmitted
 * @param len length of string to be transmitted
 */
void write_string(int client_socket, char *str);

void write_eot(int client_socket);

/* the caller has to free the buffer, unless ulen == 0 */
size_t read_and_store_string(int client_socket, char **result);

char *create_file(const char *msg);
char *update_file(const char *msg);
char *delete_file(const char *msg);
char *read_file(const char *msg);
char *list_files(const char *msg);

#endif
