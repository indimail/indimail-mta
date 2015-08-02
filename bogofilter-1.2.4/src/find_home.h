/* $Id: find_home.h 5752 2005-03-29 03:04:39Z relson $ */

/* find_home.h -- library function to figure out the home dir of current user */

#ifndef FIND_HOME_H
#define FIND_HOME_H

#include "system.h"

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

/*@null@*/ /*@observer@*/
const char *
find_home(int read_env) /*@globals errno@*/;

/*@null@*/ /*@observer@*/
const char *
find_home_user(const char *username) /*@globals errno@*/;

/*@only@*/
char *
tildeexpand(const char *filename) /*@globals errno@*/;

#endif
