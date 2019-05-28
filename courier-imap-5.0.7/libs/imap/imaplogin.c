/*
** Copyright 1998 - 2016 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<time.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif

#include	"imaptoken.h"
#include	"imapwrite.h"

#include	"authlib/auth.h"
#include	"authlib/authmod.h"
#include	"tcpd/spipe.h"
#include	"authlib/debug.h"

static const char rcsid[]="$Id: imaplogin.c,v 1.24 2004/09/12 23:25:56 mrsam Exp $";

FILE *debugfile=0;
extern void mainloop();
extern void imapcapability();
extern void initcapability();
extern int have_starttls();
extern int tlsrequired();
extern int authenticate(const char *);
unsigned long header_count=0, body_count=0;	/* Dummies */
int enabled_utf8=0;

extern unsigned long bytes_received_count; /* counter for received bytes (imaptoken.c) */
extern unsigned long bytes_sent_count; /* counter for sent bytes (imapwrite.c) */

int main_argc;
char **main_argv;
char tcpremoteip[28];
extern time_t start_time;

void rfc2045_error(const char *p)
{
	if (write(2, p, strlen(p)) < 0)
		_exit(1);
	_exit(0);
}

extern void cmdfail(const char *, const char *);
extern void cmdsuccess(const char *, const char *);

static int	starttls(const char *tag)
{
int	pipefd[2];
pid_t	p;
int	waitstat;

	if (libmail_streampipe(pipefd))
	{
		cmdfail(tag, "libmail_streampipe() failed.\r\n");
		return (-1);
	}

	p=fork();
	if (p == -1)
	{
		cmdfail(tag, "fork() failed.\r\n");
		return (-1);
	}

	if (p == 0)
	{
	char	buf1[100];
	char	dummy;

		/*
		** Fork once more, and let the parent exit,
		** so that courieresmtpd doesn't have this
		** child process.
		*/

		p=fork();
		if (p == -1)
		{
			perror("fork");
			exit(1);
		}
		if (p)	exit(0);

		close(pipefd[0]);
		sprintf(buf1, "-localfd=%d", (int)pipefd[1]);
		if (read(pipefd[1], &dummy, 1) != 1)
			exit(0);

		/* couriertls will have the socket on fd 0,
		** and dup stderr on fd 1 */

		close(1);
		dup(2);
		execl( getenv("COURIERTLS"), "couriertls",
			buf1, "-tcpd", "-server", (char *)0);
	}

	cmdsuccess(tag, "Begin SSL/TLS negotiation now.\r\n");
	writeflush();
	close(pipefd[1]);
	close(0);
	close(1);
	if (dup(pipefd[0]) != 0 || dup(pipefd[0]) != 1)
	{
		perror("dup");
		exit(1);
	}
	close(pipefd[0]);
	write(1, "", 1);	/* child - exec OK now */
	while (wait(&waitstat) != p)
		;

	/* We use select() with a timeout, so use non-blocking filedescs */

	if (fcntl(0, F_SETFL, O_NONBLOCK) ||
	    fcntl(1, F_SETFL, O_NONBLOCK))
	{
		perror("fcntl");
		exit(1);
	}
	return (0);
}

int do_imap_command(const char *tag, int *flushflag)
{
	struct	imaptoken *curtoken=nexttoken();
	char authservice[40];

#if SMAP
	if (strcmp(tag, "\\SMAP1") == 0)
	{
		const char *p=getenv("SMAP_CAPABILITY");

		if (p && *p)
			putenv("PROTOCOL=SMAP1");
		else
			return -1;
	}
#endif

	auth_debug_login( 1, "command=%s", curtoken->tokenbuf );

	if (strcmp(curtoken->tokenbuf, "LOGOUT") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)   return (-1);
		writes("* BYE IMAP4rev1 server shutting down\r\n");
		cmdsuccess(tag, "LOGOUT completed\r\n");
		writeflush();
		fprintf(stderr, "INFO: LOGOUT, ip=[%s], port=%s, rcvd=%lu, sent=%lu\n",
			getenv("TCPREMOTEIP"), getenv("TCPREMOTEPORT"), bytes_received_count, bytes_sent_count);
		exit(0);
	}
	if (strcmp(curtoken->tokenbuf, "NOOP") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)	return (-1);
		cmdsuccess(tag, "NOOP completed\r\n");
		return (0);
	}
	if (strcmp(curtoken->tokenbuf, "CAPABILITY") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)	return (-1);

		writes("* CAPABILITY ");
		imapcapability();
		writes("\r\n");
		cmdsuccess(tag, "CAPABILITY completed\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "STARTTLS") == 0)
	{
		if (!have_starttls())	return (-1);
		if (starttls(tag))		return (-2);
		putenv("IMAP_STARTTLS=NO");
		putenv("IMAP_TLS_REQUIRED=0");
		putenv("IMAP_TLS=1");
		*flushflag=1;
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "LOGIN") == 0)
	{
	struct imaptoken *tok=nexttoken_nouc();
	char	*userid, *ptr;
	char	*tagenv;
	char	*passwd;
	const char *p;

		if (have_starttls() && tlsrequired())	/* Not yet */
		{
			cmdfail(tag, "STARTTLS required\r\n");
			return (0);
		}

		switch (tok->tokentype)	{
		case IT_ATOM:
		case IT_NUMBER:
		case IT_QUOTED_STRING:
			break;
		default:
			return (-1);
		}

		userid=strdup(tok->tokenbuf);
		if (!userid)
			write_error_exit(0);
		if((ptr = strchr(userid, ':')) != (char *) 0)
		{
			*ptr = 0;
			snprintf(tcpremoteip, 28, "TCPREMOTEIP=%s", ptr + 1);
			putenv(tcpremoteip);
		}
		tok=nexttoken_nouc_okbracket();
		switch (tok->tokentype)	{
		case IT_ATOM:
		case IT_NUMBER:
		case IT_QUOTED_STRING:
			break;
		default:
			free(userid);
			return (-1);
		}

		tagenv=malloc(sizeof("IMAPLOGINTAG=")+strlen(tag));
		if (!tagenv)	write_error_exit(0);
		strcat(strcpy(tagenv, "IMAPLOGINTAG="), tag);
		passwd=my_strdup(tok->tokenbuf);

		if (nexttoken()->tokentype != IT_EOL)
		{
			free(tagenv);
			free(userid);
			free(passwd);
			return (-1);
		}
		putenv(tagenv);

		strcat(strcpy(authservice, "AUTHSERVICE"),
		       getenv("TCPLOCALPORT"));

		p=getenv(authservice);

		if (!p || !*p)
			p="imap";

		authmod_login(main_argc-1, main_argv+1, p,
                         userid, passwd);
	}

	if (strcmp(curtoken->tokenbuf, "AUTHENTICATE") == 0)
	{
		if (have_starttls() && tlsrequired())	/* Not yet */
		{
			cmdfail(tag, "STARTTLS required\r\n");
			return (0);
		}
		authenticate(tag);
		cmdfail(tag, "Authentication failed.\r\n");
		writeflush();
		return (-2);
	}

	return (-1);
}

extern void ignorepunct();

int main(int argc, char **argv)
{
const char	*tag=getenv("IMAPLOGINTAG");
const char	*ip, *port;

#ifdef HAVE_SETVBUF_IOLBF
	setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
#endif
	ip=getenv("TCPREMOTEIP");
	if (!ip)	exit(9);
	if(!(port = getenv("TCPREMOTEPORT")))
		exit(9);
	time(&start_time);
	initcapability();
#if	IMAP_CLIENT_BUGS

	ignorepunct();

#endif

	auth_debug_login_init();

	/* We use select() with a timeout, so use non-blocking filedescs */

	if (fcntl(0, F_SETFL, O_NONBLOCK) ||
	    fcntl(1, F_SETFL, O_NONBLOCK))
	{
		perror("fcntl");
		exit(1);
	}

	if (authmoduser(argc, argv, 60, 5))
	{
		writes("* OK IMAP4rev1 Server Ready.\r\n");
		fprintf(stderr, "INFO: Connection, ip=[%s], port=%s\n", ip, port);
	}
	else
	{
		cmdfail(tag ? tag:"", "Login failed.\r\n");
		fprintf(stderr, "ERR: LOGIN FAILED, ip=[%s], port=%s\n", ip, port);
	}
	writeflush();
	main_argc=argc;
	main_argv=argv;
	putenv("PROTOCOL=IMAP");
	mainloop();
	return (0);
}

void bye()
{
	exit(0);
}
