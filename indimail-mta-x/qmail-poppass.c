/*
 * $Log: qmail-poppass.c,v $
 * Revision 1.5  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.4  2021-06-15 12:14:05+05:30  Cprogrammer
 * use makeargs from libqmail
 *
 * Revision 1.3  2009-08-06 22:53:21+05:30  Cprogrammer
 * remove '\r' from input stream
 *
 * Revision 1.2  2009-08-06 09:33:22+05:30  Cprogrammer
 * log password change to descriptor 2
 *
 * Revision 1.1  2009-08-05 14:34:21+05:30  Cprogrammer
 * Initial revision
 *
 *
 * See http://ietfreport.isoc.org/all-ids/draft-gellens-password-00.txt

 * Steve Dorner's description of the simple protocol:
 *
 * The server's responses should be like an FTP server's responses; 
 * 1xx for in progress, 2xx for success, 3xx for more information
 * needed, 4xx for temporary failure, and 5xx for permanent failure.  
 * Putting it all together, here's a sample conversation:
 *
 *   S: 200 machine_name popassd v1.4 hello, who are you?\r\n
 *   E: user yourloginname\r\n
 *   S: 300 your password please.
 *   E: pass yourcurrentpassword\r\n
 *   S: 200 your new password please.\r\n
 *   E: newpass yournewpassword\r\n
 *   200 Password changed, thank-you.\r\n
 *   E: quit\r\n
 *   S: 200 Bye-bye\r\n
 *   S: <closes connection>
 *   E: <closes connection>
 */
#include <unistd.h>
#include <substdio.h>
#include <str.h>
#include <env.h>
#include <fmt.h>
#include <wait.h>
#include <byte.h>
#include <sig.h>
#include <stralloc.h>
#include <getln.h>
#include <error.h>
#include <makeargs.h>
#include <noreturn.h>
#include "auto_uids.h"

static char     ssinbuf[1024];
static char     ssoutbuf[512];
static char     sserrbuf[512];
static char     upbuf[128];
static char   **authargs, **passargs;
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static stralloc user = { 0 };
static stralloc old_pass = { 0 };
static stralloc new_pass = { 0 };
static substdio ssup;

void
errlog(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(111);
}

void
errlogf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(111);
	if (substdio_flush(&sserr) == -1)
		_exit(111);
}

void
out(char *s)
{
	if (substdio_puts(&ssout, s) == -1)
		_exit(111);
}

void
flush()
{
	if (substdio_flush(&ssout) == -1)
		_exit(111);
}

no_return void
my_error(char *s1, int exit_val)
{
	if (substdio_puts(&sserr, s1))
		_exit(111);
	if (substdio_puts(&sserr, ": "))
		_exit(111);
	if (substdio_puts(&sserr, error_str(errno)))
		_exit(111);
	if (substdio_puts(&sserr, "\n"))
		_exit(111);
	if (substdio_flush(&sserr) == -1)
		_exit(111);
	out(exit_val ? "500 " : " 400");
	out(s1);
	out(": ");
	out("\r\n");
	flush();
	_exit(exit_val ? 100 : 111);
}

int
authenticate()
{
	int             child;
	int             wstat;
	int             pi[2];

	if (pipe(pi) == -1)
		my_error("Requested action aborted: unable to open pipe and I can't auth (#4.3.0)", 0);
	switch (child = fork())
	{
	case -1:
		my_error("Requested action aborted: child won't start and I can't auth (#4.3.0)", 0);
	case 0:
		close(pi[1]);
		if (pi[0] != 3)
		{
			dup2(pi[0], 3);
			close(pi[0]);
		}
		sig_pipedefault();
		execv(*authargs, authargs);
		_exit(1);
	}
	close(pi[0]);
	substdio_fdbuf(&ssup, write, pi[1], upbuf, sizeof upbuf);
	if (substdio_put(&ssup, user.s, user.len) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_put(&ssup, old_pass.s, old_pass.len) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_put(&ssup, "\0\0", 2) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_flush(&ssup) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	/* byte_zero(old_pass.s, old_pass.len); */
	byte_zero(upbuf, sizeof upbuf);
	close(pi[1]);
	if (wait_pid(&wstat, child) == -1)
		my_error("Requested action aborted: problem with child and I can't auth (#4.3.0)", 0);
	if (wait_crashed(wstat))
		my_error("Requested action aborted: problem with child and I can't auth (#4.3.0)", 0);
	return (wait_exitcode(wstat));
}

int
change_pass()
{
	int             child;
	int             wstat;
	int             pi[2];

	if (pipe(pi) == -1)
		my_error("Requested action aborted: unable to open pipe and I can't auth (#4.3.0)", 0);
	switch (child = fork())
	{
	case -1:
		my_error("Requested action aborted: child won't start and I can't auth (#4.3.0)", 0);
	case 0:
		close(pi[1]);
		if (pi[0] != 3)
		{
			dup2(pi[0], 3);
			close(pi[0]);
		}
		sig_pipedefault();
		execv(*passargs, passargs);
		_exit(1);
	}
	close(pi[0]);
	substdio_fdbuf(&ssup, write, pi[1], upbuf, sizeof upbuf);
	if (substdio_put(&ssup, user.s, user.len) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_put(&ssup, old_pass.s, old_pass.len) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_put(&ssup, "\0", 1) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_put(&ssup, new_pass.s, new_pass.len) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	if (substdio_flush(&ssup) == -1)
		my_error("Requested action aborted: unable to write pipe and I can't auth (#4.3.0)", 0);
	byte_zero(old_pass.s, old_pass.len);
	byte_zero(new_pass.s, new_pass.len);
	byte_zero(upbuf, sizeof upbuf);
	close(pi[1]);
	if (wait_pid(&wstat, child) == -1)
		my_error("Requested action aborted: problem with child and I can't auth (#4.3.0)", 0);
	if (wait_crashed(wstat))
		my_error("Requested action aborted: problem with child and I can't auth (#4.3.0)", 0);
	return (wait_exitcode(wstat));
}

int
main(int argc, char **argv)
{
	stralloc        line = { 0 };
	int             match, item;
	char           *host, *ptr;
	uid_t           uid;
	char            strnum[FMT_ULONG];

	if (argc < 2)
	{
		out("500 usage: ");
		out(argv[0]);
		out(" hostname checkprogram [subprogram subprogram]\r\n");
		flush();
		sleep(1);
		return (1);
	}
	if (!(ptr = env_get("PASSWORD_COMMAND")))
	{
		out("500 PASSWORD_COMMAND not set\r\n");
		flush();
		sleep(1);
		return (1);
	}
	if (!(passargs = makeargs(ptr)))
		my_error("out of memory", 0);
	host = argv[1];
	authargs = argv + 2;
	uid = getuid();
	if (uid != auto_uidv && uid != 0)
	{
		out("500 you should run this program with uid 0 or uid ");
		strnum[fmt_ulong(strnum, auto_uidv)] = 0;
		out(strnum);
		out("\r\n");
		flush();
		sleep(1);
		return (1);
	}
	for (item = 1;;)
	{
		switch (item)
		{
		case 1:
			if (!stralloc_copys(&user, ""))
				my_error("out of memory", 0);
			if (!stralloc_copys(&old_pass, ""))
				my_error("out of memory", 0);
			if (!stralloc_copys(&new_pass, ""))
				my_error("out of memory", 0);

			out("200 ");
			out(host);
			out(" poppasswd hello, who are you?\r\n");
			break;
		case 2:
			out("300 your password please.\r\n");
			break;
		case 3:
			out("300 your new password please.\r\n");
			break;
		}
		flush();
		if (getln(&ssin, &line, &match, '\n') == -1)
			my_error("read", 0);
		if (!match && line.len == 0)
			break;
		--line.len;
		if (line.len && (line.s[line.len - 1] == '\r'))
			--line.len;
		if (!str_diffn(line.s, "quit", 4))
		{
 			out("200 Bye-bye\r\n");
			flush();
			_exit (0);
		}
		switch (item)
		{
		case 1:
			if (str_diffn(line.s, "user ", 5))
			{
				out("500 Username required.\r\n");
				continue;
			}
			if (!stralloc_catb(&user, line.s + 5, line.len - 5))
				my_error("out of memory", 0);
			if (!stralloc_0(&user))
				my_error("out of memory", 0);
			break;
		case 2:
			if (str_diffn(line.s, "pass ", 5))
			{
				out("500 Password required.\r\n");
				item = 1;
				continue;
			}
			if (!stralloc_catb(&old_pass, line.s + 5, line.len - 5))
				my_error("out of memory", 0);
			if (!stralloc_0(&old_pass))
				my_error("out of memory", 0);
			if (authenticate())
			{
				out("500 Old password is incorrect or user not found.\r\n");
				item = 1;
				sleep (5);
				continue;
			}
			break;
		case 3:
			if (str_diffn(line.s, "newpass ", 8))
			{
				out("500 New Password required.\r\n");
				item = 1;
				continue;
			}
			if (!stralloc_catb(&new_pass, line.s + 8, line.len - 8))
				my_error("out of memory", 0);
			if (!stralloc_0(&new_pass))
				my_error("out of memory", 0);
			if (change_pass())
			{
				errlog("Password change for ");
				errlog(user.s);
				errlogf(" failed\n");
				out("500 Password change failed.\r\n");
				item = 1;
				continue;
			} else
			{
				errlog("Password change for ");
				errlog(user.s);
				errlogf(" succeeded\n");
 				out("200 Password changed for ");
				out(user.s);
				out(", thank-you.\r\n");
				item = 0;
			}
			break;
		} /*- switch (item) */
		item++;
	} /*- for (item = 1;;) */
	return(0);
}

#ifndef lint
void
getversion_qmail_poppass_c()
{
	static char    *x = "$Id: qmail-poppass.c,v 1.5 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmakeargsh;
	x++;
}
#endif
