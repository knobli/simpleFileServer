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

bool finest(int deep, const char* message, ...);
bool debug(int deep, const char* message, ...);
bool info(int deep, const char* message, ...);
bool warn(int deep, const char* message, ...);
bool error(int deep, const char* message, ...);

bool set_log_lvl(size_t lvl);

#define FINEST 5
#define DEBUG 4
#define INFO 3
#define WARN 2
#define ERROR 1
#define NONE 0

#endif /* LOGGER_H_ */
