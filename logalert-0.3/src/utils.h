#ifndef UTILS_H
#define UTILS_H

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "log.h"
#include "defs.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#define MALLOC_UNIT 2

#define COMMENT '#'
#define MAXARGSIZE 50
#define DELIMITERS " \t\n\r"


void * xmalloc (size_t size);
void * xrealloc (void *ptr, size_t size);
int open_file(char *file, int flags,mode_t *mode);
int open_file_ro(char *file);
char * getword(char *line);
int getwords(char *line, char **vec);
int mygetline(int fd, char **buff);
char ** dupArray(char **src);
void mywrite (int fd, char const *buffer, size_t n_bytes);


#endif
