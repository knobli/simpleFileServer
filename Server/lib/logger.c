/*
 * logger.c
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#include <logger.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LEN 1024

char * append_strings(const char * old, const char * new) {
	// find the size of the string to allocate
	const size_t old_len = strlen(old), new_len = strlen(new);
	const size_t out_len = old_len + new_len + 1;

	// allocate a pointer to the new string
	char *out = malloc(out_len);

	// concat both strings and return
	memcpy(out, old, old_len);
	memcpy(out + old_len, new, new_len + 1);
	return out;
}

char *rm_last_cr(const char* string) {
	char *delimiter = "\n";
	char *saveptr;
	char *part = strtok_r(string, delimiter, &saveptr);
	char *rv = "";
	int i = 0;
	while (part != NULL) {
		if (i > 0) {
			rv = append_strings(rv, "\n");
		}
		rv = append_strings(rv, part);
		part = strtok_r(NULL, delimiter, &saveptr);
		i++;
	}
	return rv;
}

char *mk_readable(const char* string) {
	char *delimiter = "\n";
	char *saveptr;
	char *part = strtok_r(string, delimiter, &saveptr);
	char *rv = "";
	int i = 0;
	while (part != NULL) {
		if (i > 0) {
			rv = append_strings(rv, "\\n");
		}
		rv = append_strings(rv, part);
		part = strtok_r(NULL, delimiter, &saveptr);
		i++;
	}
	if (i > 1) {
		rv = append_strings(rv, "\\n");
	}
	return rv;
}

void log(const char* tag, const char* message) {
	time_t now;
	time(&now);
	char *saveptr;
	char *timestamp = strtok_r(ctime(&now), "\n", &saveptr);
	message = mk_readable(message);
	printf("%s [%s]: %s\n", timestamp, tag, message);
}

void debug(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("DEBUG", logLine);
}

void info(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("INFO", logLine);
}

void warn(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("WARN", logLine);
}

void error(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("ERROR", logLine);
}
