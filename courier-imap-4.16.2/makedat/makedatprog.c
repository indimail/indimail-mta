/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	"dbobj.h"


static int addgdbm(char *p, struct dbobj *o)
{
char	*key, *val;

	if (!*p || *p == '#')
		return (0);

	key=p;
	if ( (val=strchr(p, '\t')) == 0)	val="";
	else	*val++=0;

	if (*key)
	{
		if (!*val)	val="1";
		if (dbobj_store(o, key, strlen(key), val, strlen(val), "I"))
		{
			fprintf(stderr, "Cannot store record for %s - duplicate or out of disk space.\n", key);
			return (-1);
		}
	}
	return (0);
}

static int buildgdbm(FILE *i, struct dbobj *o)
{
char	*buf=0;
size_t	bufsize, buflen;

	bufsize=0;
	buflen=0;

	for (;;)
	{
	int	c;

		buflen=0;
		for (;;)
		{
			if (buflen >= bufsize)
			{
				bufsize += 256;
				buf= buf ? realloc(buf, bufsize):
					malloc(bufsize);
				if (!buf)
				{
					perror("malloc");
					return (-1);
				}
			}
			c=getc(i);
			if (c == '\n' || c == EOF)	break;
			buf[buflen++]=c;
		}
		buf[buflen]=0;
		if (c == EOF)	return (-1);

		if (strcmp(buf, ".") == 0)	return (0);
		if (addgdbm(buf, o))	return (-1);
	}
}

int main(int argc, char **argv)
{
FILE	*i;
struct	dbobj	obj;

	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s textfile tmpfile gdbmfile\n",
			argv[0]);
		exit(1);
	}
	if (strcmp(argv[1], "-") == 0)
		i= stdin;
	else
	{
		if ((i=fopen(argv[1], "r")) == 0)
		{
			perror(argv[1]);
			exit(1);
		}
	}

	dbobj_init(&obj);

	if (dbobj_open(&obj, argv[2], "N"))
	{
		fprintf(stderr, "Cannot create %s\n", argv[2]);
		exit (1);
	}

	if (buildgdbm(i, &obj))
	{
		dbobj_close(&obj);
		unlink(argv[2]);
		exit (1);
	}

	dbobj_close(&obj);

	if (rename(argv[2], argv[3]))
	{
		perror("rename");
		unlink(argv[2]);
		exit(1);
	}
	exit(0);
	return (0);
}
