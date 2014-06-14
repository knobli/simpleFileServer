/*
 * regex-handle.h
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */

#ifndef REGEX_HANDLE_H_
#define REGEX_HANDLE_H_

#include <regex.h>

int compile_regex(regex_t * r, const char * regex_text);

int match_regex(regex_t * r, const char * to_match, char *filename, char *length, char *content);

#endif /* REGEX_HANDLE_H_ */
