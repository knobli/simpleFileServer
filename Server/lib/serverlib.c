/* (C) IT Sky Consulting GmbH 2014
 * http://www.it-sky-consulting.com/
 * Author: Karl Brodowsky
 * Date: 2014-02-27
 * License: GPL v2 (See https://de.wikipedia.org/wiki/GNU_General_Public_License )
 */

/*
 * library functions commonly used for the system programming examples
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <serverlib.h>

#define ERROR_SIZE 4096

void exit_by_type(enum exit_type et) {
  switch (et) {
  case PROCESS_EXIT: 
    exit(1);
    break;
  case THREAD_EXIT:
    pthread_exit(NULL);
    break;
  case NO_EXIT:
    printf("continuing\n");
    break;
  default:
    printf("unknown exit_type=%d\n", et);
    exit(2);
    break;
  }
}


/* helper function for dealing with errors */
void handle_error_myerrno(long return_code, int myerrno, const char *msg, enum exit_type et) {
  if (return_code < 0) {
    char extra_msg[ERROR_SIZE];
    char error_msg[ERROR_SIZE];
    const char *error_str = strerror(myerrno);
    if (msg != NULL) {
      sprintf(extra_msg, "%s\n", msg);
    } else {
      extra_msg[0] = '\000';
    }
    sprintf(error_msg, "return_code=%ld | message=%s\nerror=%s\n", return_code, error_str, extra_msg);
    write(STDOUT_FILENO, error_msg, strlen(error_msg));
    exit_by_type(et);
  }
}

void handle_thread_error(int retcode, const char *msg, enum exit_type et) {
  if (retcode != 0) {
    handle_error_myerrno(-abs(retcode), retcode, msg, et);
  }
}

/* helper function for dealing with errors */
void handle_error(long return_code, const char *msg, enum exit_type et) {
  int myerrno = errno;
  handle_error_myerrno(return_code, myerrno, msg, et);
}


void handle_ptr_error(void *ptr, const char *msg, enum exit_type et) {
  if (ptr == NULL) {
    handle_error(-1L, msg, et);
  }
}


void die_with_error(char *error_message) {
    perror(error_message);
    exit(1);
}

int open_retry_mode(char *file, int flags, mode_t mode, enum exit_type et) {

  int fd = -1;
  while (fd == -1) {
    fd = open(file, flags, mode);
    if (fd < 0 && errno == EMFILE) {
      sleep(1);
      continue;
    }
    if (fd < 0) {
      char msg[1024];
      sprintf(msg, "error while opening file=%s", file);
      handle_error(fd, msg, et);
    }
  }
  return fd;
}

int open_retry(char *file, int flags, enum exit_type et) {

  int fd = -1;
  while (fd == -1) {
    fd = open(file, flags);
    if (fd < 0 && errno == EMFILE) {
      sleep(1);
      continue;
    }
    if (fd < 0) {
      char msg[1024];
      sprintf(msg, "error while opening file=%s", file);
      handle_error(fd, msg, et);
    }
  }
  return fd;
}


/* check if file exits and what type it is
 * exit with error if errors occur during stat
 * return NOT_EXISTENT, DIRECTORY, REGULAR_FILE or OTHER
 */
enum file_type check_file(const char *file_or_dir_name) {
  int r;
  struct stat stat_buf;
  r = lstat(file_or_dir_name, &stat_buf);
  if (r < 0) {
    int myerrno = errno;
    if (myerrno == ENOENT) {
      /* not existing should be handled as legitimate result, not as error */
      return NOT_EXISTENT;
    } else {
      const char *error_str = strerror(myerrno);
      printf("errno=%d\nmessage=%s\n", myerrno, error_str);
      exit(1);
    }
  }
  mode_t st_mode = stat_buf.st_mode;
  if (S_ISDIR(st_mode)) {
    return DIRECTORY;
  } else if (S_ISREG(st_mode)) {
    return REGULAR_FILE;
  } else {
    return OTHER;
  }
}

/* create a given file if it is not there */
int create_if_missing(const char *pathname, mode_t mode) {
  int fd = creat(pathname, mode);
  if (fd < 0) {
    char s[1024];
    sprintf(s, "could not create file=\"%s\".", pathname);
    handle_error(fd, s, NO_EXIT);
    return fd;
  }
  int retcode = close(fd);
  handle_error(fd, "close", NO_EXIT);
  return retcode;
}

/* check if --help or similar is indicated */
int is_help_requested(int argc, char *argv[]) {
  return (argc >= 2 
          && (strcmp(argv[1], "-h") == 0 
              || strcmp(argv[1], "-H") == 0
              || strcmp(argv[1], "-help") == 0
              || strcmp(argv[1], "--help") == 0));
}

/* get a timestamp that is s seconds and n nanoseconds into the future from now */
struct timespec get_future(time_t sec, long nsec) {
  int retcode;

  struct timespec ts;
  retcode = clock_gettime(CLOCK_REALTIME, &ts);
  handle_error(retcode, "clock_gettime", PROCESS_EXIT);
  // printf("timout(%ld sec %ld nsec)\n", (long) ts.tv_sec, (long) ts.tv_nsec);
  ts.tv_sec  += sec;
  ts.tv_nsec += nsec;
  // printf("timout(%ld sec %ld nsec)\n", (long) ts.tv_sec, (long) ts.tv_nsec);
  return ts;
}

int is_string_char(char c) {
  return ('A' <= c && c <= 'Z' || '0' <= c && c <= '9' || 'a' <= c && c <= 'z' || '\240' <= c && c <= '\377');
}
