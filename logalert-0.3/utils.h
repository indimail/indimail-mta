/*
 * $Log: utils.h,v $
 * Revision 1.1  2013-05-15 00:15:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef UTILS_H
#define UTILS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "log.h"
#include "defs.h"

#define MALLOC_UNIT 2

#define COMMENT '#'
#define MAXARGSIZE 50
#define DELIMITERS " \t\n\r"


void           *xmalloc(size_t size);
void           *xrealloc(void *ptr, size_t size);
int             open_file(char *file, int flags, mode_t * mode);
int             open_file_ro(char *file);
char           *getword(char *line);
int             getwords(char *line, char **vec);
char          **dupArray(char **src);
void            mywrite(int, char const *, size_t);

#endif
