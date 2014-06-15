/*
 * message-creator.h
 *
 *  Created on: 15.06.2014
 *      Author: knobli
 */

#ifndef MESSAGE_CREATOR_H_
#define MESSAGE_CREATOR_H_

const char *list_msg = "LIST\n";

char *create_create_message(const char *filename, const char *content);

char *create_update_message(const char *filename, const char *content);

char *create_read_message(const char *filename);

char *create_delete_message(const char *filename);

char *create_numbered_filename(const char *filename_base, size_t num);

char *create_create_message_numbered(const char *filename_base, size_t num, const char *file_content);

char *create_update_message_numbered(const char *filename_base, size_t num, const char *file_content);

char *create_read_message_numbered(const char *filename_base, size_t num);

char *create_delete_message_numbered(const char *filename_base, size_t num);


#endif /* MESSAGE_CREATOR_H_ */
