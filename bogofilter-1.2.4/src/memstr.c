/* $Id: memstr.c 6774 2009-02-01 02:29:27Z relson $ */

/** \file memstr.c
 * find a C string in memory
 * \author Matthias Andree
 * \date 2004
 * GNU General Public License v2
 */

#include "memstr.h"

/** find the C string \a needle in the \a n bytes starting with \a hay,
 * \return 0 if no match found, the pointer to the first byte otherwise.
 */
void *memstr(void *hay, size_t n, const char *needle)
{
    unsigned char *haystack = hay;
    size_t l = strlen(needle);

    while (n >= l) {
	if (0 == memcmp(haystack, needle, l))
	    return (void *)haystack;
	haystack++;
	n--;
    }
    return (void *)0;
}
