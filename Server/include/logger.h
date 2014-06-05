/*
 * logger.h
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#ifndef LOGGER_H_
#define LOGGER_H_

char * append_strings(const char * old, const char * new);
char *mk_readable(char* string);

void debug(int deep, const char* message, ...);
void info(int deep, const char* message, ...);
void warn(int deep, const char* message, ...);
void error(int deep, const char* message, ...);

#define DEBUG 4
#define INFO 3
#define WARN 2
#define ERROR 1

#endif /* LOGGER_H_ */
