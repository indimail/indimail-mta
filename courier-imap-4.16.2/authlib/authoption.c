/*
** Copyright 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<pwd.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#include	"auth.h"

static const char rcsid[]="$Id: authoption.c,v 1.1 2004/01/19 19:37:37 mrsam Exp $";


char *authgetoptionenv(const char *keyword)
{
	return authgetoption(getenv("OPTIONS"), keyword);
}

char *authgetoption(const char *options, const char *keyword)
{
	size_t keyword_l=strlen(keyword);
	char *p;

	while (options)
	{
		if (strncmp(options, keyword, keyword_l) == 0)
		{
			if (options[keyword_l] == 0 ||
			    options[keyword_l] == ',')
				return strdup("");

			if (options[keyword_l] == '=')
			{
				options += keyword_l;
				++options;

				for (keyword_l=0;
				     options[keyword_l] &&
					     options[keyword_l] != ',';
				     ++keyword_l)
					;

				if (!(p=malloc(keyword_l+1)))
					return NULL;
				memcpy(p, options, keyword_l);
				p[keyword_l]=0;
				return p;
			}
		}

		options=strchr(options, ',');
		if (options)
			++options;
	}
	errno=ENOENT;
	return NULL;
}
