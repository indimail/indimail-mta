/*
** Copyright 2000-2001 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"authsasl.h"
#include	"soxwrap/sconnect.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<fcntl.h>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <sys/un.h>
#include        <sys/time.h>
#include        <unistd.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <errno.h>
#include	"authdaemonrc.h"
#include	"numlib/numlib.h"

#define	TIMEOUT		15

static const char rcsid[]="$Id: authdaemonlib.c,v 1.9 2004/04/18 15:54:38 mrsam Exp $";

static int opensock()
{
int	s=socket(PF_UNIX, SOCK_STREAM, 0);
struct  sockaddr_un skun;

	skun.sun_family=AF_UNIX;
	strcpy(skun.sun_path, AUTHDAEMONSOCK);

	if (s < 0)
	{
		perror("CRIT: authdaemon: socket() failed");
		return (-1);
	}

	if (s_connect(s, (const struct sockaddr *)&skun, sizeof(skun),
		      TIMEOUT))
	{
		perror("CRIT: authdaemon: s_connect() failed");
		if (errno == ETIMEDOUT || errno == ECONNREFUSED)
			fprintf(stderr, "[Hint: perhaps authdaemond is not running?]\n");
		close(s);
		return (-1);
	}
	return (s);
}

static int writeauth(int fd, const char *p, unsigned pl)
{
fd_set  fds;
struct  timeval tv;

	while (pl)
	{
	int     n;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec=TIMEOUT;
		tv.tv_usec=0;
		if (select(fd+1, 0, &fds, 0, &tv) <= 0 || !FD_ISSET(fd, &fds))
			return (-1);
		n=write(fd, p, pl);
		if (n <= 0)     return (-1);
		p += n;
		pl -= n;
	}
	return (0);
}

static void readauth(int fd, char *p, unsigned pl)
{
time_t	end_time, curtime;

	--pl;

	time(&end_time);
	end_time += TIMEOUT;

	while (pl)
	{
	int     n;
	fd_set  fds;
	struct  timeval tv;

		time(&curtime);
		if (curtime >= end_time)
			break;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec=end_time - curtime;
		tv.tv_usec=0;
		if (select(fd+1, &fds, 0, 0, &tv) <= 0 || !FD_ISSET(fd, &fds))
			break;

		n=read(fd, p, pl);
		if (n <= 0)
			break;
		p += n;
		pl -= n;
	}
	*p=0;
}

int authdaemondopasswd(char *buffer, int bufsiz)
{
	int s=opensock();

	if (s < 0)
		return (1);
	if (writeauth(s, buffer, strlen(buffer)))
	{
		close(s);
		return (1);
	}

	readauth(s, buffer, bufsiz);
	close(s);
	return (strcmp(buffer, "OK\n"));
}

int authdaemondo(const char *authreq,
	int (*func)(struct authinfo *, void *), void *arg)
{
int	s=opensock();
char	buf[BUFSIZ];
char	*p, *q, *r;
struct	authinfo a;
uid_t	u;

	if (s < 0)
	{
		return (1);
	}

	if (writeauth(s, authreq, strlen(authreq)))
	{
		close(s);
		return (1);
	}

	readauth(s, buf, sizeof(buf));
	close(s);
	memset(&a, 0, sizeof(a));
	a.homedir="";
	p=buf;
	while (*p)
	{
		for (q=p; *q; q++)
			if (*q == '\n')
			{
				*q++=0;
				break;
			}
		if (strcmp(p, ".") == 0)
		{
			return ( (*func)(&a, arg));
		}
		if (strcmp(p, "FAIL") == 0)
			return (-1);
		r=strchr(p, '=');
		if (!r)
		{
			p=q;
			continue;
		}
		*r++=0;

		if (strcmp(p, "USERNAME") == 0)
			a.sysusername=r;
		else if (strcmp(p, "UID") == 0)
		{
			u=atol(r);
			a.sysuserid= &u;
		}
		else if (strcmp(p, "GID") == 0)
		{
			a.sysgroupid=atol(r);
		}
		else if (strcmp(p, "HOME") == 0)
		{
			a.homedir=r;
		}
		else if (strcmp(p, "ADDRESS") == 0)
		{
			a.address=r;
		}
		else if (strcmp(p, "NAME") == 0)
		{
			a.fullname=r;
		}
		else if (strcmp(p, "MAILDIR") == 0)
		{
			a.maildir=r;
		}
		else if (strcmp(p, "QUOTA") == 0)
		{
			a.quota=r;
		}
		else if (strcmp(p, "PASSWD") == 0)
		{
			a.passwd=r;
		}
		else if (strcmp(p, "PASSWD2") == 0)
		{
			a.clearpasswd=r;
		}
		else if (strcmp(p, "OPTIONS") == 0)
		{
			a.options=r;
		}
		p=q;
	}
	return (1);
}

void auth_daemon_cleanup()
{
}

struct enum_getch {
	char buffer[BUFSIZ];
	char *buf_ptr;
	size_t buf_left;
};

#define getauthc(fd,eg) ((eg)->buf_left-- ? \
			(unsigned char)*((eg)->buf_ptr)++:\
			fillgetauthc((fd),(eg)))

static int fillgetauthc(int fd, struct enum_getch *eg)
{
	time_t	end_time, curtime;

	time(&end_time);
	end_time += 60;

	for (;;)
	{
		int     n;
		fd_set  fds;
		struct  timeval tv;

		time(&curtime);
		if (curtime >= end_time)
			break;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec=end_time - curtime;
		tv.tv_usec=0;
		if (select(fd+1, &fds, 0, 0, &tv) <= 0 || !FD_ISSET(fd, &fds))
			break;

		n=read(fd, eg->buffer, sizeof(eg->buffer));
		if (n <= 0)
			break;

		eg->buf_ptr=eg->buffer;
		eg->buf_left=n;

		--eg->buf_left;
		return (unsigned char)*(eg->buf_ptr)++;
	}
	return EOF;
}

static int readline(int fd, struct enum_getch *eg,
		    char *buf,
		    size_t bufsize)
{
	if (bufsize == 0)
		return EOF;

	while (--bufsize)
	{
		int ch=getauthc(fd, eg);

		if (ch == EOF)
			return -1;
		if (ch == '\n')
			break;

		*buf++=ch;
	}
	*buf=0;
	return 0;
}

void auth_daemon_enumerate( void(*cb_func)(const char *name,
					   uid_t uid,
					   gid_t gid,
					   const char *homedir,
					   const char *maildir,
					   void *void_arg),
			    void *void_arg)
{
	static char cmd[]="ENUMERATE\n";
	struct enum_getch eg;
	char linebuf[BUFSIZ];

	int s=opensock();

	if (s < 0)
		return;

	if (writeauth(s, cmd, sizeof(cmd)-1))
	{
		close(s);
		return;
	}

	eg.buf_left=0;

	while (readline(s, &eg, linebuf, sizeof(linebuf)) == 0)
	{
		char *p;
		const char *name;
		uid_t uid;
		gid_t gid;
		const char *homedir;
		const char *maildir;

		if (strcmp(linebuf, ".") == 0)
		{
			(*cb_func)(NULL, 0, 0, NULL, NULL, void_arg);
			break;
		}

		p=strchr(linebuf, '#');
		if (p) *p=0;

		p=strchr(linebuf, '\t');

		if (p)
		{
			name=linebuf;
			*p++=0;

			uid=libmail_atouid_t(p);
			p=strchr(p, '\t');
			if (uid && p)
			{
				*p++=0;
				gid=libmail_atogid_t(p);
				p=strchr(p, '\t');
				if (gid && p)
				{
					*p++=0;
					homedir=p;
					p=strchr(p, '\t');
					maildir=NULL;

					if (p)
					{
						*p++=0;
						maildir=p;
						p=strchr(p, '\t');
						if (p) *p=0;
					}


					(*cb_func)(name, uid, gid, homedir,
						   maildir, void_arg);
				}
			}
		}
	}
	close(s);
}
