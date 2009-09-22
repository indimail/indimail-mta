/*
** Copyright 2000-2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>

static const char rcsid[]="$Id: rfc2045header.c,v 1.5 2003/05/27 15:55:11 mrsam Exp $";

struct rfc2045headerinfo {
	int fd;
	char *headerbuf;
	size_t headerbufsize;

	char readbuf[1024];

	char *readptr;
	size_t readleft;

	size_t headerleft;
} ;

struct rfc2045headerinfo *rfc2045header_start(int fd, struct rfc2045 *rfcp)
{
	off_t start_pos, dummy, start_body;
	struct rfc2045headerinfo *p;

	if (rfcp)
	{
		rfc2045_mimepos(rfcp, &start_pos, &dummy, &start_body, &dummy,
				&dummy);
	}
	else
	{
		struct stat stat_buf;

		if (fstat(fd, &stat_buf) < 0)
			return (NULL);

		start_pos=0;
		start_body=stat_buf.st_size;
	}

	if (lseek(fd, start_pos, SEEK_SET) == -1)
		return (NULL);

	p=(struct rfc2045headerinfo *)calloc(sizeof(struct rfc2045headerinfo),
					     1);

	if (!p)
		return (NULL);
	p->fd=fd;
	p->headerleft=start_body - start_pos;
	return (p);
}

void rfc2045header_end(struct rfc2045headerinfo *p)
{
	if (p->headerbuf)
		free(p->headerbuf);
	free(p);
}

static int fill(struct rfc2045headerinfo *p)
{
	int n;

	if (p->headerleft == 0)
		return (-1);

	n=sizeof(p->readbuf);
	if (n > p->headerleft)
		n=p->headerleft;

	n=read(p->fd, p->readbuf, n);

	if (n <= 0)
	{
		p->headerleft=0;
		p->readleft=0;
		return (-1);
	}
	p->readptr=p->readbuf;
	p->readleft = n;
	p->headerleft -= n;
	return ((int)(unsigned char)p->readbuf[0]);
}

#define PEEK(p) ((p)->readleft ? (int)(unsigned char)*p->readptr:fill(p))


int rfc2045header_get(struct rfc2045headerinfo *p, char **header,
		      char **value,
		      int flags)
{
	int c=PEEK(p);
	int isnl=0;
	size_t n=0;
	char *s, *t;
	int eatspace=0;

	if (c == -1 || c == '\r' || c == '\n')
	{
		*header=*value=NULL;
		return (0);
	}

	for (;;)
	{
		if (n >= p->headerbufsize)
		{
			size_t n=p->headerbufsize += 256;
			char *s= p->headerbuf ?
				realloc(p->headerbuf, n):
				malloc(n);

			if (!s)
				return (-1);
			p->headerbuf=s;
			p->headerbufsize=n;
		}

		c=PEEK(p);
		if (c < 0)
			break;

		if (c == '\r')
		{
			--p->readleft;
			++p->readptr;
			continue;
		}

		if (isnl) /* Last char was newline */
		{
			if (!isspace((int)(unsigned char)c) || c == '\n')
				break;

			isnl=0;

			if ((flags & RFC2045H_KEEPNL) == 0)
				eatspace=1; /* Fold headers */
		}

		if (c == '\n')
			isnl=1;

		if (eatspace)
		{
			if (c != '\n' && isspace((int)(unsigned char)c))
			{
				--p->readleft;
				++p->readptr;
				continue;
			}
			eatspace=0;
		}

		if (c == '\n' && (flags & RFC2045H_KEEPNL) == 0)
			c=' ';

		p->headerbuf[n++]=c;
		--p->readleft;
		++p->readptr;
	}

	while (n > 0 && p->headerbuf[n-1] == ' ')
		--n;

	p->headerbuf[n]=0;

	*header= *value= p->headerbuf;

	while (**value)
	{
		if (**value == ':')
		{
			**value=0;
			++*value;

			while (**value && isspace((int)(unsigned char)**value))
				++*value;
			break;
		}

		if (!(flags & RFC2045H_NOLC))
		{
			if (**value >= 'A' && **value <= 'Z')
				**value += 'a' - 'A';
		}
		++*value;
	}

	s=strrchr( *value, '\n');

	if (s && *s && s[1] == 0)
		*s=0;

	s=strrchr( *value, '\r');

	if (s && *s && s[1] == 0)
		*s=0;

	for (s=t=*value; *s; )
		if (!isspace((int)(unsigned char)*s++))
			t=s;
	*t=0;
	return (0);
}
