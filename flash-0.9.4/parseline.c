/*
 * $Log: parseline.c,v $
 * Revision 1.3  2008-07-17 21:38:48+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.2  2008-06-09 15:32:38+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:16+05:30  Manny
 * Initial revision
 *
 *
 * General line parsing routine
 * General line parsing routine
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pwd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#include"set.h"
#include"parseline.h"
#include"misc.h"
#include"variables.h"

int
parseline(char *buffer, int *argc, char **argv, int maxargc)
{
	enum parse_state state = S_LWSP, savedstate = 0;
	int             remaining;
	char           *here = buffer;
	char           *storage, *addhere = NULL, *rememberme = NULL, *charpointer = NULL;
	char            lastquote = '\0';
	struct passwd  *pwd;

	*argc = 0;

	storage = xmalloc((strlen(buffer) + 265) * sizeof(char));
	*storage = '0';

	remaining = strlen(buffer);
	addhere = storage;

	while ((remaining) && (*argc < maxargc))
	{
		switch (state)
		{
		case S_LWSP:
			switch (*here)
			{
			case ' ':
			case '\t':
			case '\n':
				here++, remaining--;
				break;
			case '~':
				/*
				 * savedstate=state; 
				 */
				rememberme = addhere;
				state = S_TILDE;
				*(addhere++) = *(here++);
				remaining--;
				break;
			default:
				state = S_WORD;
				break;
			}
			break;
		case S_WORD:
			switch (*here)
			{
			case ' ':
			case '\t':
			case '\n':
				state = S_LWSP;
				*addhere = '\0';
				*(argv + *argc) = xmalloc((strlen(storage) + 1) * sizeof(char));
				strcpy(*(argv + *argc), storage);
				addhere = storage;
				(*argc)++;
				here++, remaining--;
				break;
			case COMMENT:
				here++, remaining = 0;
				break;
			case ESCAPE:
				savedstate = state;
				state = S_ESCAPED;
				here++, remaining--;
				break;
			case '"':
			case '\'':
				state = S_QUOTED;
				lastquote = *(here++);
				remaining--;
				break;
			case '$':
				/*
				 * savedstate=state; 
				 */
				rememberme = addhere;
				state = S_DOLLAR;
				*(addhere++) = *(here++);
				remaining--;
				break;
			default:
				*(addhere++) = *(here++);
				remaining--;
				break;
			}
			break;
		case S_QUOTED:
			if (*here == ESCAPE)
			{
				savedstate = state;
				state = S_ESCAPED;
				here++, remaining--;
			} else
			if (*here == lastquote)
			{
				state = S_WORD;
				here++, remaining--;
			} else
			{
				*(addhere++) = *(here++);
				remaining--;
			}
			break;
		case S_ESCAPED:
			*(addhere++) = *(here++);
			remaining--;
			state = savedstate;
			break;
		case S_TILDE:
			switch (*here)
			{
			case ESCAPE:
				savedstate = state;
				state = S_ESCAPED;
				here++, remaining--;
				break;
			case '/':
			case ' ':
			case '\t':
			case '\n':
				state = S_WORD;
				*addhere = '\0';
				if (addhere == rememberme + 1)
					pwd = getpwuid(getuid());
				else
					pwd = getpwnam(rememberme + 1);
				if (pwd != NULL)
				{
					strcpy(rememberme, pwd->pw_dir);
					addhere = rememberme + strlen(rememberme);
				}
				break;
			case COMMENT:
				here++, remaining = 0;
				break;
			default:
				*(addhere++) = *(here++);
				remaining--;
				break;
			}
			break;
		case S_DOLLAR:
			switch (*here)
			{
			case ESCAPE:
				savedstate = state;
				state = S_ESCAPED;
				here++, remaining--;
				break;
			case COMMENT:
				here++, remaining = 0;
				break;
			case '\'':
			case '\"':
			case ' ':
			case '\t':
			case '\n':
				state = S_WORD;
				*addhere = '\0';
				if (addhere == rememberme + 1)
					charpointer = NULL;
				else
					charpointer = getenv(rememberme + 1);
				if (charpointer != NULL)
				{
					strcpy(rememberme, charpointer);
					addhere = rememberme + strlen(rememberme);
				}
				break;
			default:
				*(addhere++) = *(here++);
				remaining--;
				break;
			}
			break;
		default:
			fprintf(stderr, "Weirdness Factor 12\n");
			return -1;
			break;
		}
	}

	if (state == S_TILDE)
	{
		*addhere = '\0';
		if (addhere == (rememberme + 1))
			pwd = getpwuid(getuid());
		else
			pwd = getpwnam(rememberme + 1);
		if (pwd != NULL)
		{
			strcpy(rememberme, pwd->pw_dir);
			addhere = rememberme + strlen(rememberme);
		}
	} else
	if (state == S_DOLLAR)
	{
		*addhere = '\0';
		if (addhere == rememberme + 1)
			charpointer = NULL;
		else
			charpointer = getenv(rememberme + 1);
		if (charpointer != NULL)
		{
			strcpy(rememberme, charpointer);
			addhere = rememberme + strlen(rememberme);
		}
	}

	*addhere = '\0';

	if ((*argc < maxargc) && (strlen(storage) != 0))
	{
		*(argv + *argc) = xmalloc((strlen(storage) + 1) * sizeof(char));
		strcpy(*(argv + *argc), storage);
		addhere = storage;
		(*argc)++;
	}

	free((void *) storage);
	return *argc;
}
