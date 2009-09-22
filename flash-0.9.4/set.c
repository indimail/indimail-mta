/*
 * $Log: set.c,v $
 * Revision 1.3  2008-06-09 15:32:51+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.2  2002-12-21 19:09:21+05:30  Manny
 * corrected compilation warnings
 *
 * Revision 1.1  2002-12-16 01:55:19+05:30  Manny
 * Initial revision
 *
 *
 * Functions to set/search/unset variables.
 *
 * Copyright (C) Stephen Fegan 1995, 1996
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 * 
 * please send patches or advice on flash to: `flash@netsoc.ucd.ie'
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>
#include<pwd.h>

#include"set.h"
#include"tree.h"
#include"misc.h"

struct dict    *SetVariables;

static int
Set_name_compare(void *a, void *b)
{
	return strcmp(((struct set_node *) a)->variable, ((struct set_node *) b)->variable);
}

int
find_variable(char *key, char **data)
{
	struct set_node k, *x;
	k.variable = key;
	k.value = NULL;

	if (data != NULL)
		*data = NULL;

	if (SetVariables == NULL)
		return 0;

	x = (struct set_node *) find_node(SetVariables, &k);

	if ((x == NULL) || ((x->flags & SF_SET) == 0))
		return 0;
	else
	{
		if (data != NULL)
			*data = x->value;
		return 1;
	}
}

void
set_variable(char *key, char *data, int resettable)
{
	struct set_node *k, *x;

	k = (struct set_node *) xmalloc(sizeof(*k));
	k->variable = xmalloc(strlen(key) + 1);
	strcpy(k->variable, key);

	if (data != NULL)
	{
		k->value = xmalloc(strlen(data) + 1);
		strcpy(k->value, data);
	} else
		k->value = NULL;

	k->flags = SF_SET;
	if (!resettable)
		k->flags |= SF_NORESET;

	if (SetVariables == NULL)
		SetVariables = new_dict(Set_name_compare);

	if (!add_unique_node(SetVariables, (void *) k))
	{
		x = find_node(SetVariables, (void *) k);
		if ((x->flags & SF_NORESET) == 0)
			x = change_node(SetVariables, (void *) k);
		else
			x = k;
		free(x);
	}
}

void
unset_variable(char *key)
{
	struct set_node k, *x;

	k.variable = key;
	k.value = NULL;

	if (SetVariables == NULL)
		return;

	x = find_node(SetVariables, &k);
	if ((x != NULL) && ((x->flags & SF_NORESET) == 0))
		x->flags &= !(SF_SET | SF_NORESET);

	return;
}

void
global_variables(void)
{
	struct passwd  *pw;
	char            temp[40];

	pw = getpwuid(getuid());

	sprintf(temp, "%d", (int) pw->pw_uid);
	set_variable("uid", temp, 0);
	set_variable("user", pw->pw_name, 0);
	set_variable("home", pw->pw_dir, 0);

	set_variable("tty", ttyname(0), 0);
}
