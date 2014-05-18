/*
 * logger.h
 *
 *  Created on: 17.05.2014
 *      Author: knobli
 */

#ifndef LOGGER_H_
#define LOGGER_H_

void debug(const char* message, ...);
void info(const char* message, ...);
void warn(const char* message, ...);
void error(const char* message, ...);

#endif /* LOGGER_H_ */
