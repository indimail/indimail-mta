/*
 * $Log: exp.c,v $
 * Revision 2.2  2011-04-08 17:26:08+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.1  2004-09-05 00:55:52+05:30  Cprogrammer
 * routines for Operator Shell
 *
 *
 * This is (osh) exp.c  Version 1.0
 * 
 * Description:
 * These are the routines which handle wildcard expansion.
 * 
 * ----
 * Copyright (c) 1993 The Regents of the University of California
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Los Alamos National Laboratory and its contributors.'' Neither the name of
 * the University nore the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

static char    *rcsid = "@(#) $Id: exp.c,v 2.2 2011-04-08 17:26:08+05:30 Cprogrammer Stab mbhangui $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#if STDC_HEADERS
#include <string.h>
#else
char           *strchr(), *strrchr();
#endif
#ifdef HAVE_MALLOC_H
# include <malloc.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include "osh.h"

int
instring(target, key)
	char           *target, *key;

{
	char            c;
#ifdef STDC_HEADERS
	size_t          i;
#else
	int             i;
#endif

	for (i = 0; i < strlen(key); i++)
	{
		c = (*(key + i));
		if (strchr(target, c) != 0)
			return 1;
	}
	return 0;
}

int
reg_match(target, pattern)
	char           *target;
	char           *pattern;

{
	char           *tcopy = (char *) malloc(strlen(target) + 1);
	char           *pcopy = (char *) malloc(strlen(pattern) + 1);
	char            c;
	int             ret = 1;
	int             advance = 0;
	int             found;

	strcpy(tcopy, target);
	strcpy(pcopy, pattern);

	while (*pcopy != '\0')
		switch (c = (*pcopy++))
		{
		case '*':
			advance = 1;
			break;
		case '?':
			if (*tcopy != '\0')
				tcopy++;
			else
				ret = 0;		/*- ? must be matched by SOMETHING */
			break;
		case '[':
			found = 0;
			while ((c != '\0') && (c != ']'))
			{
				if (*tcopy == c)
					found = 1;
				c = (*(pcopy++));
			}
			if ((!found) || (c == '\0'))
				ret = 0;
			break;
		default:
			if (advance)
			{
				advance = 0;
				while ((*tcopy != '\0') && (*tcopy != c))
					tcopy++;
				if (*tcopy == c)
					tcopy++;
			} else
			{
				if (*tcopy != c)
					ret = 0;
				else
					tcopy++;
			}
			break;
		}
	if ((*pcopy == '\0') && (*tcopy != '\0') && (!advance))
		ret = 0;
	return ret;
}

void
insert_alias(alias, argc, argv)
	char           *alias;
	int            *argc;
	char           *argv[MAXARG + 1];

{
	char           *x;
	char           *alias2 = (char *) malloc(strlen(alias) + 1);
	int             done = 0;

	strcpy(alias2, alias);
	while (!done)
	{
		if ((x = (char *) strchr(alias2, ' ')) == NULL)
		{
			argv[*argc] = (char *) malloc(strlen(alias2) + 1);
			strcpy(argv[(*argc)++], alias2);
			done = 1;
		} else
		{
			*x = '\0';
			argv[*argc] = (char *) malloc(strlen(alias2) + 1);
			strcpy(argv[(*argc)++], alias2);
			x++;
			alias2 = x;
		}
	}
}	/*- *Of routine */

char           *
expand(argv, argc, word)
	char           *argv[MAXARG + 1];
	int            *argc;
	char           *word;

{
	int             oarg, i;
	DIR            *dirp;
	struct dirent  *dp;
	char            path[MAXPATHLEN], target[80];
	char           *x;
	int             aliased = 0;
	struct stat     buf;

	oarg = (*argc);
	if (*argc == 0)
		for (i = 0; i < AliasCounter; i++)
			if (!strcmp(AliasList[i].cmd, word))
			{
				insert_alias(AliasList[i].alias, argc, argv);
				aliased = 1;
				break;
			}
	if (aliased)
		return (NULL);
	if (!instring(word, "*?[]{}"))
	{
		/*- Not a regexp */
		argv[*argc] = (char *) malloc(strlen(word) + 1);
		strcpy(argv[*argc], word);
		(*argc)++;
		return (NULL);
	}
	if ((x = (char *) strrchr(word, '/')) == NULL)
	{
		/*- there is slash */
		strcpy(path, ".");
		strcpy(target, word);
	} else
	{
		*x = '\0';
		strcpy(path, word);
		strcpy(target, x + 1);
	}
	/*
	 * Now we have separated the path and target from the incoming string 
	 */
	if ((dirp = opendir(path)) == NULL)
		return ("No such file or directory");
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	{
		stat(dp->d_name, &buf);
		/*
		 * if (!(buf.st_mode & S_IFDIR)) {   Allow directories 
		 */
		if (reg_match(dp->d_name, target))
		{
			argv[*argc] = (char *) malloc(strlen(path) + strlen(dp->d_name) + 2);
			if (strcmp(path, "."))
			{
				strcpy(argv[*argc], path);
				strcat(argv[*argc], "/");
				strcat(argv[*argc], dp->d_name);
			} else
				strcpy(argv[*argc], dp->d_name);
			(*argc)++;
		}
		/*- } */
	}
	if (oarg == (*argc))
		return ("No Match");
	return (NULL);
}

void
getversion_exp_c()
{
	printf("%s\n", rcsid);
}
