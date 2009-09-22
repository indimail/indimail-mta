/* find_home_tildeexpand.c -- library function to do sh-like ~ expansion */

/* (C) 2002 by Matthias Andree <matthias.andree@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details, it is in the file named
 * COPYING.
 */

/* $Id: find_home_tildeexpand.c 5753 2005-03-29 03:06:11Z relson $ */

#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "find_home.h"
#include "xmalloc.h"
#include "xstrdup.h"

#ifndef __riscos__
static bool tilde_expand =  true;
#else
static bool tilde_expand =  false;
#endif

/*
 * tildeexpand tries to expand the first tilde argument, similar, but
 * not identical, to the way a POSIX sh does. It does not support
 * multiple tildes, and does not search for a slash, but for the longest
 * count of characters in the POSIX portable file name character set.
 */
/*@only@*/
char *tildeexpand(const char *name) 
{
    char *tmp;
    const char *home;
    size_t l, tl;

    if (!tilde_expand)
	return xstrdup(name);

    if (name[0] != '~')
	return xstrdup(name);

    /* figure length of user name */
    l = strspn(&name[1], 
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "0123456789._-"); /* Portable Filename Character Set */
    if (l > 0) {
	/* got a parameter to the tilde */
	tmp = xmalloc(l + 1);
	memcpy(tmp, &name[1], l);
	/* we want exactly the first l characters but as C string,
	 * so stuff the NUL byte */
	tmp[l] = '\0';

	home = find_home_user(tmp);

	xfree(tmp);
    } else {
	/* plain tilde */
	home = find_home(false);
    }

    if (home == NULL) {
	return xstrdup(name);
    }

    tl = strlen(name) + strlen(home) - l + 1;
    tmp = xmalloc(tl);
    (void)strlcpy(tmp, home, tl);

    /* no need to insert a slash here, name[l] contains one */
    if (strlcat(tmp, name + l + 1, tl) >= tl) 
	internal_error;

    return tmp;
}
