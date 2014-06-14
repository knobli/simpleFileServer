/*
 * logger.h
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdbool.h>
#include <stddef.h>

char *append_strings(const char * old, const char * new);
char *mk_readable(char* string);

bool debug(int deep, const char* message, ...);
bool info(int deep, const char* message, ...);
bool warn(int deep, const char* message, ...);
bool error(int deep, const char* message, ...);

bool set_log_lvl(size_t lvl);

#define DEBUG 4
#define INFO 3
#define WARN 2
#define ERROR 1

#endif /* LOGGER_H_ */
