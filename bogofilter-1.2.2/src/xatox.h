/* $Id: xatox.h 1807 2003-03-04 04:10:38Z m-a $ */

/** \file xatox.h
 * Header file for xato* string-to-number conversion
 * functions with simplified error checking.
 *
 * \author Matthias Andree
 * \date 2003
 */

#ifndef XATOX_H
#define XATOX_H

/** atoi variant with error checking. The function tries to parse the
 * integer string \a in into \a i using the \b strtol(3) ANSI C
 * function. \a i is only changed on success and retains its original
 * value otherwise.
 *
 * \return
 * - 0 - failure, out of range or illegal characters, see errno for
 *   details; if errno == 0, then the string contained junk at the end.
 * - 1 - success
 */
int xatoi(/*@out@*/ int *i /** the result is stored here */,
	  const char *in /** input string */);

/** strtod variant with simplified error checking. The function tries to parse
 * the floating point string \a in into \a d using the \b strtod(3) ANSI
 * C function. \a d is only changed on success and retains its original
 * value otherwise.
 *
 *  \return
 * - 0 - failure, out of range or illegal characters, see errno for
 *   details; if errno == 0, then the string contained junk at the end.
 * - 1 - success
 */
int xatof(/*@out@*/ double *d /** the result is stored here */,
	  const	char *in /** input string */);

#endif
