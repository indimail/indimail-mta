#include	"argparse.h"
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>

/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/



int argparse(int argc, char **argv, struct args *s)
{
int	argn=1;
int	i;

	while (argn < argc)
	{
	const char *p;
	int	l=0;

		if ( argv[argn][0] != '-')	break;
		if ( argv[argn][1] == 0)
		{
			++argn;
			break;
		}
		for (i=0; s[i].name; i++)
		{
			l=strlen(s[i].name);
			if (strncmp(s[i].name, argv[argn]+1, l) == 0 &&
				(argv[argn][l+1] == 0 ||
				argv[argn][l+1] == '='))	break;
		}
		if (s[i].name == 0)
		{
			fprintf(stderr, "%s: Invalid option: %s\n",
				argv[0], argv[argn]);
			exit(1);
		}
		p=argv[argn]+1+l;
		if (*p)	++p;
		if (s[i].valuep)
			*s[i].valuep=p;
		else
			(*s[i].funcp)(p);
		++argn;
	}
	return (argn);
}
