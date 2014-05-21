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
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include <stdbool.h>
#include <stdio.h>      /* for printf() and fprintf() and ... */
#include <stdlib.h>     /* for atoi() and exit() and ... */
#include <string.h>     /* for memset() and ... */
//#include <sys/socket.h> /* for socket(), bind(), recv, send(), and connect() */
#include <sys/types.h>
#include <unistd.h>     /* for close() */

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
	int len = strlen(str);

	int send_len = write(client_socket, str, len);
	handle_error(send_len, "could not write", THREAD_EXIT);

	if(send_len != len){
		error("Write to client socket failed");
		exit_by_type(THREAD_EXIT);
	}
}

void write_eot(int client_socket) {
	write_string(client_socket, "");
}

/* the caller has to free the buffer, unless ulen == 0 */
size_t read_and_store_string(int client_socket, char **result) {
	char buffer[MAX_MESSAGE_LEN + 1];
	size_t bytes_received = 0;

	debug("Read from client socket");
	bytes_received = read(client_socket, buffer, MAX_MESSAGE_LEN);
	handle_error(bytes_received,
			"recv() failed or connection closed prematurely", THREAD_EXIT);
	debug("Received bytes: %zu", bytes_received);

	if (bytes_received == 0) {
		debug("Return empty buffer");
		*result = (char *) EMPTY_BUFFER; /* actually the same as empty string */
		return bytes_received;
	}

	buffer[bytes_received] = '\000';

	*result = (char *) malloc(bytes_received + 1);
	strncpy(*result, buffer, bytes_received + 1);
	return bytes_received;
}
