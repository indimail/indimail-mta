/* $Id: bogoreader.h 6714 2008-04-20 15:27:06Z relson $ */

/** \file bogoreader.h
 * prototypes and definitions for bogoreader.c
 *
 * \author David Relson <relson@osagesoftware.com>
 */

#ifndef BOGOREADER_H
#define BOGOREADER_H

#include "buff.h"

/* Function Prototypes */

extern void bogoreader_init(int argc, const char * const *argv);
extern void bogoreader_close_ifeof(void);
extern void bogoreader_fini(void);
void bogoreader_name(const char *name);

/* Lexer-Reader Interface */

/** check if the string of \a len bytes starting at \a buf
 * ends with LF or CRLF */
extern	bool is_eol(const char *buf, size_t len);

typedef int   reader_line_t(buff_t *buff);
typedef bool  reader_more_t(void);
typedef const char *reader_file_t(void);

extern reader_line_t *reader_getline;
extern reader_more_t *reader_more;
extern reader_file_t *reader_filename;

#endif
