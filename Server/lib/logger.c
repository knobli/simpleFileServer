/*
 * logger.c
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define MAX_MSG_LEN 1024

void log(const char* tag, const char* message) {
	time_t now;
	time(&now);
	printf("%s [%s]: %s\n", ctime(&now), tag, message);
}

void debug(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("DEBUG", message);
}

void info(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("INFO", message);
}

void warn(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("WARN", message);
}

void error(const char* message, ...) {
	va_list listPointer;
	char logLine[MAX_MSG_LEN];
	va_start(listPointer, message);
	vsprintf(logLine, message, listPointer);
	va_end(listPointer);
	log("ERROR", message);
}
