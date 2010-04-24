/*
 * $Log: qmail-rspawn.c,v $
 * Revision 1.26  2010-04-24 07:40:36+05:30  Cprogrammer
 * set SMTPROUTE / QMTPROUTE depending on route variable
 *
 * Revision 1.25  2010-02-10 08:58:59+05:30  Cprogrammer
 * removed dependency on indimail
 *
 * Revision 1.24  2009-12-10 10:46:57+05:30  Cprogrammer
 * return -2 for MySQL error
 *
 * Revision 1.23  2009-12-09 23:55:57+05:30  Cprogrammer
 * handle MySQL error correctly
 *
 * Revision 1.22  2007-12-20 13:57:15+05:30  Cprogrammer
 * added case 'r' for code clarity
 * removed compilation warning
 *
 * Revision 1.21  2004-10-22 20:29:33+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.20  2004-10-09 00:58:05+05:30  Cprogrammer
 * use MAX_BUFF instead of hardcoded numbers
 *
 * Revision 1.19  2004-07-17 21:21:33+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 * Revision 1.18  2003-12-09 00:15:35+05:30  Cprogrammer
 * execute spawn-filter with argv[0] as qmail-remote
 *
 * Revision 1.17  2003-12-07 13:06:17+05:30  Cprogrammer
 * check for the return value of env_put() and env_unset()
 *
 * Revision 1.16  2003-11-29 23:40:43+05:30  Cprogrammer
 * QMAILREMOTE environment variable to execute different qmail-remote
 *
 * Revision 1.15  2003-11-25 20:41:18+05:30  Cprogrammer
 * removed fork.h
 *
 * Revision 1.14  2003-10-28 20:01:57+05:30  Cprogrammer
 * conditional compilation for INDIMAIL
 *
 * Revision 1.13  2003-10-23 01:25:59+05:30  Cprogrammer
 * replaced strncmp with str_diffn
 * fixed compilation warnings
 *
 * Revision 1.12  2003-10-12 01:13:48+05:30  Cprogrammer
 * added msgsize as argument to spawn()
 *
 * Revision 1.11  2003-09-27 21:11:59+05:30  Cprogrammer
 * change in rcpthosts() function
 *
 * Revision 1.10  2003-09-21 18:54:55+05:30  Cprogrammer
 * added comment
 *
 * Revision 1.9  2002-09-08 23:48:43+05:30  Cprogrammer
 * added fmt.h
 *
 * Revision 1.8  2002-09-07 20:37:07+05:30  Cprogrammer
 * chdir to auto_qmail to make rcpthosts_init() work
 *
 * Revision 1.7  2002-09-04 01:50:06+05:30  Cprogrammer
 * conditional compilation of indimail code
 *
 * Revision 1.6  2002-08-25 03:17:06+05:30  Cprogrammer
 * replaced snprintf with fmt_str and fmt_strn
 *
 * Revision 1.5  2002-03-27 16:22:37+05:30  Cprogrammer
 * added sccsid
 *
 * Revision 1.4  2002-03-27 16:05:11+05:30  Cprogrammer
 * added SMTPROUTE bypass if env ROUTES is defined as static
 *
 */
#include <unistd.h>
#include "fd.h"
#include "str.h"
#include "wait.h"
#include "substdio.h"
#include "exit.h"
#include "error.h"
#include "tcpto.h"
#include "env.h"
#include "rcpthosts.h"
#include "fmt.h"
#include "auto_qmail.h"
#include "hasindimail.h"

void
initialize(argc, argv)
	int             argc;
	char          **argv;
{
	tcpto_clean();
}

int             truncreport = 0;

void
report(ss, wstat, s, len)
	substdio       *ss;
	int             wstat;
	char           *s;
	int             len;
{
	int             j;
	int             k;
	int             result;
	int             orr;

	if (wait_crashed(wstat))
	{
		substdio_puts(ss, "Zqmail-remote crashed.\n");
		return;
	}
	switch (wait_exitcode(wstat))
	{
	case 0:
		break;
	case 111:
		substdio_puts(ss, "ZUnable to run qmail-remote.\n");
		return;
	default:
		substdio_puts(ss, "DUnable to run qmail-remote.\n");
		return;
	}
	if (!len)
	{
		substdio_puts(ss, "Zqmail-remote produced no output.\n");
		return;
	}
	result = -1;
	j = 0;
	for (k = 0; k < len; ++k)
	{
		if (!s[k])
		{
			if (s[j] == 'K')
			{
				result = 1;
				break;
			}
			if (s[j] == 'Z')
			{
				result = 0;
				break;
			}
			if (s[j] == 'D')
				break;
			j = k + 1;
		}
	}
	orr = result;
	switch (s[0])
	{
	case 's': /*- SMTP code >= 400 */
		orr = 0;
		break;
	case 'h': /*- SMTP code >= 500 */
		orr = -1;
	case 'r': /*- SMTP code success */
		break;
	}

	switch (orr)
	{
	case 1:
		substdio_put(ss, "K", 1);
		break;
	case 0:
		substdio_put(ss, "Z", 1);
		break;
	case -1:
		substdio_put(ss, "D", 1);
		break;
	}
	for (k = 1; k < len;)
	{
		if (!s[k++])
		{
			substdio_puts(ss, s + 1);
			if (result <= orr)
				if (k < len)
					switch (s[k])
					{
					case 'Z':
					case 'D':
					case 'K':
						substdio_puts(ss, s + k + 1);
					}
			break;
		}
	}
}

int
spawn(fdmess, fdout, msgsize, s, qqeh, r, at)
	int             fdmess;
	int             fdout;
	unsigned long   msgsize;
	char           *s;
	char           *qqeh;
	char           *r;
	int             at;
{
	int             f;
	char           *ptr, *(args[7]);
	char            size_buf[FMT_ULONG];
#ifdef INDIMAIL
	int             i;
	char           *ip, *real_domain;
	static char     smtproute[MAX_BUFF], CurDir[MAX_BUFF]; 
	static int      rcptflag = 1;
#endif

	size_buf[fmt_ulong(size_buf, msgsize)] = 0;
	args[0] = "qmail-remote";
	args[1] = r + at + 1;		/*- domain */
	args[2] = s;				/*- sender */
	args[3] = qqeh;				/*- qqeh */
	args[4] = size_buf;			/*- size */
	args[5] = r;				/*- recipient */
	args[6] = 0;
#ifdef INDIMAIL
	*smtproute = 0;
	if (!env_unset("SMTPROUTE"))
		return (-1);
	else
	if (!env_unset("QMTPROUTE"))
		return (-1);
	/*
	 * On SIGHUP have to figure out a way to set rcptflag to 0.
	 * A new Distributed domain added will not work as distributed as
	 * long as rcptflag is not reinitialized. Static smtproutes will
	 * be used instead.
	 */
	if ((ptr = env_get("ROUTES")) && (!str_diffn(ptr, "smtp", 4) || !str_diffn(ptr, "qmtp", 4)))
	{
		if (rcptflag)
		{
			if (!getcwd(CurDir, MAX_BUFF - 1))
				return (-1);
			if (chdir(auto_qmail))
				return (-1);
			rcptflag = rcpthosts_init();
			if (chdir(CurDir))
				return (-1);
		}
		if (!rcptflag && (f = rcpthosts(r, str_len(r), 0)) == 1)
		{
			if ((real_domain = vget_real_domain(r + at + 1))
				&& (is_distributed_domain(real_domain) == 1))
			{
				if ((ip = findhost(r, 0)) != (char *) 0)
				{
					i = fmt_str(smtproute, !str_diffn(ptr, "smtp", 4) ? "SMTPROUTE=" : "QMTPROUTE=");
					i += fmt_strn(smtproute + i, ip, MAX_BUFF - 11);
					if (i > MAX_BUFF - 1)
						return (-1);
					smtproute[i] = 0;
					if (!env_put(smtproute))
						return (-1);
				} else
				if (!userNotFound) /*- temp MySQL error */
					return (-2);
			}
		}
	}
#endif
	if (!(f = vfork()))
	{
		if (fd_move(0, fdmess) == -1)
			_exit(111);
		if (fd_move(1, fdout) == -1)
			_exit(111);
		if (fd_copy(2, 1) == -1)
			_exit(111);
		if (!(ptr = env_get("QMAILREMOTE")))
			execvp(*args, args);
		else
			execvp(ptr, args);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	return f;
}

void
getversion_qmail_rspawn_c()
{
	static char    *x = "$Id: qmail-rspawn.c,v 1.26 2010-04-24 07:40:36+05:30 Cprogrammer Exp mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#else
	x++;
#endif
}
