/*
 * $Log: postdel.c,v $
 * Revision 2.8  2017-03-13 14:05:07+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.7  2011-11-09 19:45:30+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.6  2008-07-13 19:50:52+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.5  2008-06-05 16:20:34+05:30  Cprogrammer
 * moved vdelivermail vfilter to sbin
 *
 * Revision 2.4  2002-12-13 19:11:15+05:30  Cprogrammer
 * added environment variable MAKE_SEEKABLE to turn on/off seekable stdin
 *
 * Revision 2.3  2002-12-11 10:28:26+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.2  2002-12-05 14:15:51+05:30  Cprogrammer
 * set environment variable SENDER and MTA
 *
 * Revision 2.1  2002-11-22 14:17:05+05:30  Cprogrammer
 * postfix wrapper for vfilter/vdelivermail
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: postdel.c,v 2.8 2017-03-13 14:05:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * From postfix source distribution - sys_exits.h
 */
#define EX_USAGE		64		/*- command line usage error */
#define EX_DATAERR		65		/*- data format error */
#define EX_NOINPUT		66		/*- cannot open input */
#define EX_NOUSER		67		/*- addressee unknown */
#define EX_NOHOST		68		/*- host name unknown */
#define EX_UNAVAILABLE	69		/*- service unavailable */
#define EX_SOFTWARE		70		/*- internal software error */
#define EX_OSERR		71		/*- system error (e.g., can't fork) */
#define EX_OSFILE		72		/*- critical OS file missing */
#define EX_CANTCREAT	73		/*- can't create (user) output file */
#define EX_IOERR		74		/*- input/output error */
#define EX_TEMPFAIL		75		/*- temporary failure */
#define EX_PROTOCOL		76		/*- remote error in protocol */
#define EX_NOPERM		77		/*- permission denied */
#define EX_CONFIG		78		/*- configuration error */

char           *(vdelargs[]) = { PREFIX"/sbin/vdelivermail", "''", BOUNCE_ALL, 0};
char           *(filtargs[]) = { PREFIX"/sbin/vfilter", "''", BOUNCE_ALL, 0};

static int      get_options(int, char **, char *, char *, char *, char *, int *);

int
main(int argc, char **argv)
{
	char            User[MAX_BUFF], Host[MAX_BUFF], ReturnPathEnv[MAX_BUFF], Sender[MAX_BUFF];
	int             use_filter, wait_status, tmp_stat;
	pid_t           pid;

	if(get_options(argc, argv, User, Host, ReturnPathEnv, Sender, &use_filter))
		_exit(EX_USAGE);
	if (putenv(User) == -1)
	{
		fprintf(stderr, "postdel: putenv: %s\n", strerror(ENOMEM));
		_exit(EX_OSERR);
	} else
	if (putenv(Host) == -1)
	{
		fprintf(stderr, "postdel: putenv: %s\n", strerror(ENOMEM));
		_exit(EX_OSERR);
	} else
	if (putenv(ReturnPathEnv) == -1)
	{
		fprintf(stderr, "postdel: putenv: %s\n", strerror(ENOMEM));
		_exit(EX_OSERR);
	} else
	if (putenv(Sender) == -1)
	{
		fprintf(stderr, "postdel: putenv: %s\n", strerror(ENOMEM));
		_exit(EX_OSERR);
	} else
	if (putenv("MTA=Postfix") == -1)
	{
		fprintf(stderr, "postdel: putenv: %s\n", strerror(ENOMEM));
		_exit(EX_OSERR);
	}
	switch (pid = vfork())
	{
		case -1:
			_exit(EX_OSERR);
		case 0:
#ifdef MAKE_SEEKABLE
			putenv("MAKE_SEEKABLE=1");
#endif
			if(use_filter)
				execv(*filtargs, filtargs);
			else
				execv(*vdelargs, vdelargs);
			fprintf(stderr, "postdel: execv: %s\n", *vdelargs);
			_exit(111);
		default:
			for(;;)
			{
				pid = wait(&wait_status);
#ifdef ERESTART
				if(pid == -1 && (errno == EINTR || errno == ERESTART))
#else
				if(pid == -1 && errno == EINTR)
#endif
					continue;
				else
				if(pid == -1)
				{
					fprintf(stderr, "postdel: %s. indimail bug\n", *vdelargs);
					_exit(EX_SOFTWARE);
				}
				break;
			}
			if(WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status))
			{
				fprintf(stderr, "postdel: %s crashed.\n", *vdelargs);
				_exit(EX_TEMPFAIL);
			} else
			if(WIFEXITED(wait_status))
			{
				switch ((tmp_stat = WEXITSTATUS(wait_status)))
				{
				case 0:
					_exit(0);
				case 100:
					_exit(EX_NOUSER);
				default:
					_exit(EX_TEMPFAIL);
				}
			}
			break;
	}
	_exit(EX_TEMPFAIL); return(0); /*- for stupid solaris */
}

static int
get_options(int argc, char **argv, char *user, char *domain, char *rpline, char *sender, int *use_filter)
{
	int             c;
	char           *tmpstr;

	*use_filter = 0;
	*user = *domain = *rpline = 0;
	while ((c = getopt(argc, argv, "vfu:d:r:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'f':
			*use_filter = 1;
			break;
		case 'u':
			snprintf(user, MAX_BUFF, "EXT=%s", optarg);
			break;
		case 'd':
			if ((tmpstr = strchr(optarg, '@')))
				snprintf(domain, MAX_BUFF, "HOST=%s", tmpstr + 1);
			else
				snprintf(domain, MAX_BUFF, "HOST=%s", optarg);
			break;
		case 'r':
			snprintf(rpline, MAX_BUFF, "RPLINE=Return-Path: <%s>\n", optarg);
			snprintf(sender, MAX_BUFF, "SENDER=%s", optarg);
			break;
		default:
			fprintf(stderr, "USAGE: postdel [-f] -u user -d domain -r sender\n");
			return(1);
		}
	}
	if(!*user || !*domain || !*rpline)
	{
		fprintf(stderr, "USAGE: postdel [-f] -u user -d domain -r sender\n");
		return(1);
	}
	return(0);
}

void
getversion_postdel_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
