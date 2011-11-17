/*
 * $Id: iwebadmin.h,v 1.1 2010-04-26 12:08:01+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <time.h>

/*
 * Some platforms use '.maildir' instead of 'Maildir' as the mail directory
 * * name.  Furture versions of Vpopmail might define it.  Until then, change
 * * it below.
 */
#ifndef MAILDIR
#define MAILDIR "Maildir"
#endif

/*
 * max # of forwards a user can set on the Modify User screen 
 */
#define MAX_FORWARD_PER_USER 5

#define QMAILADMIN_TEMPLATEDIR "QMAILADMIN_TEMPLATEDIR"

#define SHOW_VERSION_LINK 1

#define MAX_FILE_NAME 100

#ifdef MAX_BUFF
#undef MAX_BUFF
#endif
#define MAX_BUFF 500
#define MAX_BIG_BUFF 5000

#define MAX_LANG_STR 350

#define QMAILADMIN_UNLIMITED -1

#define NO_ADMIN		0

#define DOMAIN_ADMIN	2
#define USER_ADMIN		3

#define NUM_SQL_OPTIONS	  6

#define ACTION_MODIFY 1
#define ACTION_DELETE 2

void            del_id_files(char *);
void            init_globals();
void            quickAction(char *username, int action);

/*
 * copied from maildirquota.c in vpopmail
 * * it really needs to get into vpopmail.h somehow
 */
int             readuserquota(const char *dir, long *sizep, int *cntp);
