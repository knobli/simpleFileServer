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

void log(const char* tag, const char* message) {
	time_t now;
	time(&now);
	char *timestamp = strtok(ctime(&now), "\n");
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
