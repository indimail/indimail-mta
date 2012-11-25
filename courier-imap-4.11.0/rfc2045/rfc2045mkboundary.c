/*
** Copyright 1998 - 2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include "rfc2045_config.h"
#endif
#include	"rfc2045.h"
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	"numlib/numlib.h"


#if	HAS_GETHOSTNAME

#else

extern int gethostname(char *, size_t);
#endif

extern void rfc2045_enomem();

char *rfc2045_mk_boundary(struct rfc2045 *s, struct rfc2045src *src)
{
char	hostnamebuf[256];
pid_t	mypid;
char	pidbuf[NUMBUFSIZE];
time_t	mytime;
char	timebuf[NUMBUFSIZE];
static size_t	cnt=0;
char	cntbuf[NUMBUFSIZE];
char	*p;
int	rc;

	hostnamebuf[sizeof(hostnamebuf)-1]=0;
	if (gethostname(hostnamebuf, sizeof(hostnamebuf)-1))
		hostnamebuf[0]=0;
	mypid=getpid();
	time(&mytime);
	libmail_str_pid_t(mypid, pidbuf);
	libmail_str_time_t(mytime, timebuf);
	for (;;)
	{
		char tempbuf[NUMBUFSIZE];

		libmail_str_size_t(++cnt, tempbuf);
		sprintf(cntbuf, "%4s", tempbuf);
		for (p=cntbuf; *p == ' '; *p++ = '0')
			;
		p=malloc(strlen(hostnamebuf)+strlen(pidbuf)
			 +strlen(timebuf)+strlen(cntbuf)+11);
		if (!p)
		{
			rfc2045_enomem();
			return (NULL);
		}

		sprintf(p, "=_%-1.30s-%s-%s-%s", hostnamebuf,
			pidbuf, timebuf, cntbuf);
		if ((rc=rfc2045_try_boundary(s, src, p)) == 0)
			break;
		free(p);
		if (rc < 0)
			return (NULL);
	}
	return (p);
}
