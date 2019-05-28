/*
** Copyright 1998 - 2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#if TIME_WITH_SYS_TIME
#include        <sys/time.h>
#include        <time.h>
#else
#if HAVE_SYS_TIME_H
#include        <sys/time.h>
#else
#include        <time.h>
#endif
#endif

#include	"imapwrite.h"

static const char rcsid[]="$Id: imapwrite.c,v 1.8 2008/07/20 16:58:32 mrsam Exp $";

static char outbuf[BUFSIZ];	/* Ye olde output buffer */
static size_t outbuf_cnt=0;	/* How much stuff's in ye olde output buffer */

extern FILE *debugfile;

extern unsigned long bytes_sent_count; /* counter for sent bytes (imapwrite.c) */

extern void disconnected();

void writeflush()
{
const char *p=outbuf, *cp;
unsigned s=outbuf_cnt;
time_t	t, tend;
fd_set	fds;
struct	timeval	tv;
int	n;

	if (s == 0)	return;
	time(&t);
	if (!(cp = getenv("SOCKET_TIMEOUT")))
		tend=t+SOCKET_TIMEOUT;
	else
		tend=t+atoi(cp);
	if (debugfile)
	{
		fprintf(debugfile, "WRITE: ");
		if (fwrite(p, 1, s, debugfile) == 1)
			fprintf(debugfile, "\n");
		fflush(debugfile);
	}
	while (t < tend)
	{
		FD_ZERO(&fds);
		FD_SET(1, &fds);
		tv.tv_sec=tend-t;
		tv.tv_usec=0;
		/* BUG: if client closes connection BEFORE we flush it, select "stucks"
		 * until timeout. To workaround this, we should "write" first, and then
		 * if we get EPIPE connection is already closed. Othervise, try select
		 */
		if ((n=write(1, p, s)) <= 0)
		{
			if (errno == EPIPE ||
				select(2, 0, &fds, 0, &tv) <= 0 ||
				!FD_ISSET(1, &fds) ||
				(n=write(1, p, s)) <= 0)
			{
				disconnected();
				return;
			}
		}
		bytes_sent_count += n;
		p += n;
		s -= n;
		if (s == 0)	break;
		time(&t);
	}
	if (s)	disconnected();
	outbuf_cnt=0;
}

void writemem(const char *s, size_t l)
{
size_t	n;

	while (l)
	{
		n=sizeof(outbuf) - outbuf_cnt;

		if (n >= l)
		{
			memcpy(outbuf+outbuf_cnt, s, l);
			outbuf_cnt += l;
			break;
		}
		if (n == 0)
		{
			writeflush();
			continue;
		}
		if (n > l)	n=l;
		memcpy(outbuf+outbuf_cnt, s, n);
		outbuf_cnt += n;
		l -= n;
		s += n;
	}
}

void writes(const char *s)
{
	writemem(s, strlen(s));
}

void writen(unsigned long n)
{
char	buf[40];

	sprintf(buf, "%lu", n);
	writemem(buf, strlen(buf));
}

void writeqs(const char *s)
{
size_t	i=strlen(s), j;

	while (i)
	{
		for (j=0; j<i; j++)
		{
			if ( s[j] == '"' || s[j] == '\\')
			{
				writemem(s, j);
				writemem("\\", 1);
				writemem(s+j, 1);
				++j;
				s += j;
				i -= j;
				j=0;
				break;
			}
#if 0
			if (s[j] == '&')
			{
				writemem(s, j);
				writemem("&-", 2);
				++j;
				s += j;
				i -= j;
				j=0;
				break;
			}

			if (s[j] < ' ' || s[j] >= 0x7F)
			{
			char	*q;

				writemem(s, j);
				++j;
				s += j;
				i -= j;
				for (j=0; j<i; j++)
					if (s[j] >= ' ' && s[j] < 0x7F)
						break;
				q=imap_utf7_encode(s, j);
				if (!q)	write_error_exit(0);
				writemem("&", 1);
				writes(q);
				writemem("-", 1);
				s += j;
				i -= j;
				j=0;
				break;
			}
#endif
		}
		writemem(s, j);
		s += j;
		i -= j;
	}
}

void write_error_exit(const char *funcname)
{
	outbuf_cnt=0;
	writes("* BYE [ALERT] Fatal error: ");
	if (funcname && *funcname)
	{
		writes(funcname);
		writes(": ");
	}

	if (errno)
	{
#if	HAVE_STRERROR
	const	char *p;

		p=strerror(errno);
#else
	char	p[40];

		sprintf(p, "Error %d", (int)errno);
#endif


		writes(p);
	}
	writes("\r\n");
	writeflush();

	if (funcname && *funcname)
	{
		fprintf(stderr, "ERR: %s: %s\n", getenv("AUTHENTICATED"),
			funcname);
		fflush(stderr);
	}
	_exit(1);
}
