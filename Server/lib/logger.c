/*
 * logger.c
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#include <logger.h>

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_MSG_LEN 4096
//TODO: all debug and other output to stderr

static size_t LOG_LEVEL = INFO;

int append_strings(const char *old, const char *new, char **out) {
	const size_t old_len = strlen(old);
	const size_t new_len = strlen(new);
	const size_t out_len = old_len + new_len + 1;

	*out = malloc(out_len);

	memcpy(*out, old, old_len);
	memcpy(*out + old_len, new, new_len + 1);
	return out_len;
}

char *mk_readable(char* string) {
	char *delimiter = "\n";
	char *saveptr;
	char *part = strtok_r(string, delimiter, &saveptr);
	char *rv = malloc(1);
	rv[0] = '\000';
	char *tmp_string;
	size_t tmp_length;
	int i = 0;
	while (part != NULL) {
		if (i > 0) {
			tmp_length = append_strings(rv, "\\n", &tmp_string);
			free(rv);
			rv = malloc(tmp_length);
			strncpy(rv, tmp_string, tmp_length);
			free(tmp_string);
		}
		tmp_length = append_strings(rv, part, &tmp_string);
		free(rv);
		rv = malloc(tmp_length);
		strncpy(rv, tmp_string, tmp_length);
		free(tmp_string);
		part = strtok_r(NULL, delimiter, &saveptr);
		i++;
	}
	if (i > 1) {
		tmp_length = append_strings(rv, "\\n", &tmp_string);
		free(rv);
		rv = malloc(tmp_length);
		strncpy(rv, tmp_string, tmp_length);
		free(tmp_string);
	}
	return rv;
}

void log_msg(const char* tag, int deep, char* message) {
	long thread_id = (long) pthread_self();
	time_t now;
	time(&now);
	char *saveptr;
	char *timestamp = strtok_r(ctime(&now), "\n", &saveptr);
	message = mk_readable(message);
	int i;
	char *tmp_string;
	int tmp_length;
	for (i = 0; i < deep; i++) {
		tmp_length = append_strings("  ", message, &tmp_string);
		free(message);
		message = malloc(tmp_length);
		strncpy(message, tmp_string, tmp_length);
		free(tmp_string);
	}
	printf("%ld %s [%s]: %s\n", thread_id, timestamp, tag, message);
	free(message);
}

void log_err(const char* tag, int deep, char* message) {
	long thread_id = (long) pthread_self();
	time_t now;
	time(&now);
	char *saveptr;
	char *timestamp = strtok_r(ctime(&now), "\n", &saveptr);
	message = mk_readable(message);
	int i;
	char *tmp_string;
	int tmp_length;
	for (i = 0; i < deep; i++) {
		tmp_length = append_strings("  ", message, &tmp_string);
		free(message);
		message = malloc(tmp_length);
		strncpy(message, tmp_string, tmp_length);
		free(tmp_string);
	}
	fprintf(stderr, "%ld %s [%s]: %s\n", thread_id, timestamp, tag, message);
	free(message);
}

bool make_log_msg(va_list args, const char* message, char **msg) {
	bool rv = true;
	size_t message_length = strlen(message) + MAX_MSG_LEN;
	char log_line[message_length];
	size_t write_length;
	if ((write_length = vsnprintf(log_line, message_length, message, args)) > message_length) {
		printf("Error during vsnprintf size does not match (written: %zu max: %zu)!\n", write_length, message_length);
		rv = false;
	}
	size_t msg_length = write_length + 1;
	*msg = malloc(msg_length);
	strncpy(*msg, log_line, msg_length);
	return rv;
}

bool finest(int deep, const char* message, ...) {
	if (LOG_LEVEL < FINEST) {
		return false;
	}
	bool rv;
	va_list args;
	va_start(args, message);
	char *log_line;
	rv = make_log_msg(args, message, &log_line);
	va_end(args);
	log_msg("FINEST", deep, log_line);
	free(log_line);
	return rv;
}

bool debug(int deep, const char* message, ...) {
	if (LOG_LEVEL < DEBUG) {
		return false;
	}
	bool rv;
	va_list args;
	va_start(args, message);
	char *log_line;
	rv = make_log_msg(args, message, &log_line);
	va_end(args);
	log_msg("DEBUG", deep, log_line);
	free(log_line);
	return rv;
}

bool info(int deep, const char* message, ...) {
	if (LOG_LEVEL < INFO) {
		return false;
	}
	bool rv;
	va_list args;
	va_start(args, message);
	char *log_line;
	rv = make_log_msg(args, message, &log_line);
	va_end(args);
	log_msg("INFO ", deep, log_line);
	free(log_line);
	return rv;
}

bool warn(int deep, const char* message, ...) {
	if (LOG_LEVEL < WARN) {
		return false;
	}
	bool rv;
	va_list args;
	va_start(args, message);
	char *log_line;
	rv = make_log_msg(args, message, &log_line);
	va_end(args);
	log_msg("WARN ", deep, log_line);
	free(log_line);
	return rv;
}

bool error(int deep, const char* message, ...) {
	if (LOG_LEVEL < ERROR) {
		return false;
	}
	bool rv;
	va_list args;
	va_start(args, message);
	char *log_line;
	rv = make_log_msg(args, message, &log_line);
	va_end(args);
	log_err("ERROR", deep, log_line);
	free(log_line);
	return rv;
}

bool set_log_lvl(size_t lvl) {
	info(0, "Set log level to %zu", lvl);
	LOG_LEVEL = lvl;
	return true;
}
