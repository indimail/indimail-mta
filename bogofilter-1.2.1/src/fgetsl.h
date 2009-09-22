/**
 * \file fgetsl.h Header file for NUL-proof fgets() replacement.
 * \author Matthias Andree
 * \date 2002 - 2003
 */

#ifndef FGETSL_H
#define FGETSL_H

#include <stdio.h>

/** This function reads up to \a siz-1 characters from \a stream into \a buf
 * and adds a terminating NUL character. When the buffer cannot hold at
 * least one character of payload, the program is aborted.
 * \return
 * - negative: EOF or error condition (check ferror(3))
 * - zero or positive: number of characters read (not counting the
 *   trailing NUL)
 */
extern int fgetsl(/*@out@*/ char *buf /** output buffer */,
	int siz /** capacity of buffer */,
	FILE *stream /** input stream */);

/** This function reads up to \p siz or \p siz-1 (depending on \p
 * no_NUL_terminate) characters from \p stream into \p buf and
 * optionally adds a terminating NUL character. When the buffer
 * cannot hold at least one character of payload, the program is
 * aborted.
 * \return
 * - negative: EOF or error condition (check ferror(3))
 * - zero or positive: number of characters read (not counting the
 *   trailing NUL)
 */

extern int xfgetsl(/*@out@*/ char *buf /** output buffer */,
	int siz /** capacity of buffer */,
	/*@null@*/ FILE *stream /** input stream */,
	bool no_NUL_terminate /** \li if false, the maximum amount of bytes read is size-1 and the buffer is NUL terminated.
				  \li if true, the maximum amount of bytes read is size and the buffer WILL NOT BE NUL terminated. */);

#endif
