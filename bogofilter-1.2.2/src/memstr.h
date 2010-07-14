/* $Id: memstr.h 6774 2009-02-01 02:29:27Z relson $ */

/** \file memstr.h declarations for memstr.c
 * \author Matthias Andree
 * \date 2004
 * GNU General Public License v2 */

#ifndef MEMSTR_H
#define MEMSTR_H

#include <string.h>

/** find needle in haystack (which is treated as unsigned char *). */
void *memstr(void *haystack, size_t n, const char *needle);

#endif
