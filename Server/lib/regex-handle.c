/*
 * regex-handle.c
 *
 *  Created on: 14.06.2014
 *      Author: knobli
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <regex-handle.h>
#include <logger.h>

/* The following is the size of a buffer to contain any error messages
 encountered when the regular expression is compiled. */
#define MAX_ERROR_MSG 0x1000

/**
 * Compile the regular expression described by "regex_text" into "r".
 */
int compile_regex(regex_t * r, const char * regex_text) {
	const int deep = 3;
	int status = regcomp(r, regex_text, REG_EXTENDED | REG_NEWLINE);
	if (status != 0) {
		char error_message[MAX_ERROR_MSG];
		regerror(status, r, error_message, MAX_ERROR_MSG);
		error(deep, "Regex error compiling '%s': %s", regex_text, error_message);
		return 1;
	}
	return 0;

}

/**
 * Match the string in "to_match" against the compiled regular expression in "r".
 */
int match_regex(regex_t * r, const char * to_match, char *filename, char *length, char *content) {
	const int deep = 3;
	/* "P" is a pointer into the string which points to the end of the
	 previous match. */
	const char * p = to_match;
	/* "N_matches" is the maximum number of matches allowed. */
	const int n_matches = 10;
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];
	debug(deep, "Catch the matches from %s", to_match);
	int matches = 0;
	while (1) {
		debug(deep, "Matching %s", p);
		int i = 0;
		int nomatch = regexec(r, p, n_matches, m, 0);
		if (nomatch) {
			if (matches == 0) {
				error(deep, "No matches!");
				return false;
			} else {
				debug(deep, "No more matches");
				return true;
			}
		}
		matches++;
		for (i = 0; i < n_matches; i++) {
			int start;
			int finish;
			if (m[i].rm_so == -1) {
				break;
			}
			start = m[i].rm_so + (p - to_match);
			finish = m[i].rm_eo + (p - to_match);
			if (i == 1) {
				sprintf(filename, "%.*s", (finish - start), to_match + start);
			} else if (i == 2) {
				sprintf(length, "%.*s", (finish - start), to_match + start);
			} else if (i == 3) {
				sprintf(content, "%.*s", (finish - start), to_match + start);
			}
		}
		p += m[0].rm_eo;
	}
	return false;
}

