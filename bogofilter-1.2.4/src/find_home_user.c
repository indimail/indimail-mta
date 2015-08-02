/** \file find_home_user.c figure out the home dir of given user */

/* (C) 2002,2003,2006 by Matthias Andree <matthias.andree@gmx.de>
 * (C) 2003 by David Relson <relson@osagesoftware.com>
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

/* $Id: find_home_user.c 6541 2006-07-08 14:56:40Z m-a $ */

#include "system.h"

#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

#include "find_home.h"

/** This function will try to figure out the home directory of the user
 * whose name is given as argument.
 *
 * This function returns NULL in case of failure.
 */
const char *find_home_user(const char *username) {
    struct passwd *pw;

    pw = getpwnam(username);
    if (pw != NULL) {
	return pw -> pw_dir;
    }
    return NULL;
}
