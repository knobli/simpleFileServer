/*
 * logger.h
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#ifndef LOGGER_H_
#define LOGGER_H_

char * append_strings(const char * old, const char * new);
char *rm_last_cr(const char* string);
char *mk_readable(const char* string);

void debug(const char* message, ...);
void info(const char* message, ...);
void warn(const char* message, ...);
void error(const char* message, ...);

#endif /* LOGGER_H_ */
