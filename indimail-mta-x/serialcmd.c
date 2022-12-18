/*
 * $Id: serialcmd.c,v 1.7 2022-12-18 12:29:59+05:30 Cprogrammer Exp mbhangui $
 *
 * serialcmd -- apply a command to a mail message.
 * Copyright 1999,  Len Budney
 */
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <strerr.h>
#include <getln.h>
#include <subfd.h>
#include <substdio.h>
#include <now.h>
#include <open.h>
#include <stralloc.h>
#include <sig.h>
#include <str.h>
#include <byte.h>
#include <env.h>
#include <wait.h>
#include <error.h>
#include <fmt.h>
#include <qgetpwgr.h>
#include <noreturn.h>
#include "quote.h"

#define FATAL "serialcmd: fatal: "
#define WARN  "serialcmd: warning: "

static int      use_pwgr;
static stralloc line = { 0 };
static stralloc quosender = { 0 };
static stralloc recipient = { 0 };
static stralloc sender = { 0 };

no_return void
die_usage()
{
	strerr_die1x(100, "serialcmd: usage: serialcmd prefix command");
}

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
die_nohomedir()
{
	strerr_die2x(111, FATAL, "no home directory");
}

no_return void
die_nousername()
{
	strerr_die2x(111, FATAL, "no user name");
}

no_return void
die_badexit()
{
	strerr_die2x(111, FATAL, "child exited abnormally");
}

no_return void
die_output()
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

no_return void
die_readmess()
{
	strerr_die2sys(111, FATAL, "unable to read file: ");
}

no_return void
die_openmess()
{
	strerr_die2sys(111, FATAL, "unable to open file: ");
}

no_return void
die_statmess()
{
	strerr_die2sys(111, FATAL, "unable to stat file: ");
}

no_return void
die_readstdin()
{
	strerr_die2sys(111, FATAL, "unable to read stdin: ");
}

void
result(stralloc *fnam, int code, char *statmsg)
{

	if (!stralloc_copyb(&line, fnam->s, fnam->len))
		die_nomem();
	switch (code)
	{
	case (0):
	case (99):
		if (!stralloc_catb(&line, "K", 1))
			die_nomem();
		break;
	case (100):
	case (64):
	case (65):
	case (70):
	case (76):
	case (77):
	case (78):
	case (112):
		if (!stralloc_catb(&line, "D", 1))
			die_nomem();
		break;
	case (111):
	default:
		if (!stralloc_catb(&line, "Z", 1))
			die_nomem();
		break;
	}
	if (statmsg != NULL)
		if (!stralloc_catb(&line, statmsg, str_len(statmsg)))
			die_nomem();
	if (!stralloc_catb(&line, "\n", 1))
		die_nomem();

	if (substdio_put(subfdoutsmall, line.s, line.len) == -1)
		die_output();
	if (substdio_flush(subfdoutsmall) == -1)
		die_output();
}

void
makeenv(void)
{
	int             index = 0;
	int             i = 0;
	stralloc        foo = { 0 };
	stralloc        user = { 0 };
	struct passwd  *pw = 0;
	char           *host = 0;
	char           *ext = 0;

	/*
	 * Parse the recipient address. 
	 */
	index = byte_rchr(recipient.s, recipient.len, '@');
	*(recipient.s + index) = '\0';
	host = recipient.s + index + 1;
	env_put2("LOCAL", recipient.s);
	env_put2("HOST", host);

	/*
	 * Parse the pieces of the recipient domain. 
	 */
	i = str_len(host);
	i = byte_rchr(host, i, '.');
	if (!stralloc_copyb(&foo, host, i))
		die_nomem();
	if (!stralloc_0(&foo))
		die_nomem();
	env_put2("HOST2", foo.s);
	i = byte_rchr(host, i, '.');
	if (!stralloc_copyb(&foo, host, i))
		die_nomem();
	if (!stralloc_0(&foo))
		die_nomem();
	env_put2("HOST3", foo.s);
	i = byte_rchr(host, i, '.');
	if (!stralloc_copyb(&foo, host, i))
		die_nomem();
	if (!stralloc_0(&foo))
		die_nomem();
	env_put2("HOST4", foo.s);

	/*
	 * Get the username associated with the effective UID. 
	 */
	pw = (use_pwgr ? qgetpwuid : getpwuid) (getuid());
	if (!stralloc_copys(&user, pw->pw_name))
		die_nomem();
	if (!stralloc_0(&user))
		die_nomem();
	if (pw == NULL) {
		if (errno == ENOMEM)
			die_nomem();
		die_nousername();
	}
	env_put2("USER", user.s);

	/*
	 * Compare the username with the local part of the recipient. 
	 */
	ext = recipient.s;
	if (str_len(user.s) <= str_len(ext) && !str_diffn(user.s, ext, str_len(user.s)))
		ext += str_len(user.s);
	ext += str_chr(ext, '-');
	if (*ext)
		++ext;
	env_put2("EXT", ext);
	ext += str_chr(ext, '-');
	if (*ext)
		++ext;
	env_put2("EXT2", ext);
	ext += str_chr(ext, '-');
	if (*ext)
		++ext;
	env_put2("EXT3", ext);
	ext += str_chr(ext, '-');
	if (*ext)
		++ext;
	env_put2("EXT4", ext);

	/*
	 * Determine the home directory of user 
	 */
	if (!(pw = (use_pwgr ? qgetpwnam : getpwnam) (user.s))) {
		if (errno == ENOMEM)
			die_nomem();
		die_nohomedir();
	}
	env_put2("HOME", pw->pw_dir);
}

void
runcmd(char **cmd, int fd, stralloc *fnam)
{
	pid_t           pid;
	int             i, r, werr, status, msglen, pipedesc[2];
	char            c, msg[80], strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	if (pipe(pipedesc) == -1)
		strerr_die2sys(111, FATAL, "unable to open pipe: ");
	lseek(fd, (off_t) 0, SEEK_SET);
	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	else
	if (!pid) {
		if (dup2(fd, 0) == -1)
			strerr_die2sys(111, FATAL, "unable to read message: ");
		if (dup2(pipedesc[1], 1) == -1)
			strerr_die2sys(111, FATAL, "unable to write to pipe: ");
		close(pipedesc[1]);
		close(pipedesc[0]);
		execvp(*cmd, cmd);
		strerr_die4sys(111, FATAL, "execvp: ", *cmd, ": ");
	}
	/*- Read any output from the child.  */
	close(pipedesc[1]);
	msglen = read(pipedesc[0], msg, 79);
	if (msglen == -1)
		strerr_die2sys(111, FATAL, "unable to read process output: ");
	msg[msglen] = '\n';
	msglen = str_chr(msg, '\n');
	if (*(msg + msglen))
		msg[msglen] = '\0';

	/*- Wait for the child to finish.  */
	while (1) {
		if (read(pipedesc[0], &c, 1) == 0)
			break;
	}
	for (r = -1;;) {
		if (!(i = waitpid(pid, &status, 0)))
			break;
		if (i == -1) {
#ifdef ERESTART
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
#endif
				continue;
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_die3sys(111, FATAL, strnum1, ": waitpid error: ");
		}
		if (i != pid) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_die3x(111, FATAL, strnum1, ": waitpid surprise");
		}
		if (!(i = wait_handler(status, &werr))) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strnum2[fmt_int(strnum2, werr)] = 0;
			strerr_warn4(WARN, strnum1, wait_stopped(status) ? ": stopped by signal " : ": started by signal ", strnum2, 0);
			continue;
		} else
		if (werr == -1) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_die3x(111, FATAL, strnum1, ": internal wait handler error");
		} else
		if (werr) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strnum2[fmt_uint(strnum2, WTERMSIG(status))] = 0;
			strerr_die4x(111, FATAL, strnum1, ": killed by signal ", strnum2);
		} else {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strnum2[fmt_int(strnum2, i)] = 0;
			strerr_warn4(WARN, strnum1, ": normal exit return status ", strnum2, 0);
			r = i;
		}
	}
	result(fnam, r, msg);
}

void
doit(int fd, char **argv, stralloc *fnam)
{
	char            messbuf[4096];
	substdio        ssmess;
	int             match;
	datetime_sec    age;
	struct stat     st;

	/*
	 * Stat the file, and read its age. 
	 */
	if (fstat(fd, &st) == -1)
		die_statmess();
	age = now() - st.st_mtime;
	if (!stralloc_catulong0(&line, age, 7))
		die_nomem();
	env_put2("AGE", line.s);

	/*
	 * Initialize a substdio buffer. 
	 */
	substdio_fdbuf(&ssmess, read, fd, messbuf, sizeof messbuf);

	/*
	 * Look up the envelope sender. 
	 */
	if (getln(&ssmess, &line, &match, '\n') == -1)
		die_readmess();
	if (!match)
		return;
	if (!stralloc_starts(&line, "Return-Path: <"))
		return;
	if (line.s[line.len - 2] != '>')
		return;
	if (line.s[line.len - 1] != '\n')
		return;
	if (!stralloc_copyb(&sender, line.s + 14, line.len - 16))
		die_nomem();
	if (!stralloc_catb(&line, "\0", 1))
		die_nomem();
	env_put2("RPLINE", line.s);

	/*
	 * Look up the envelope recipient. 
	 */
	if (getln(&ssmess, &line, &match, '\n') == -1)
		die_readmess();
	if (!match)
		return;
	if (!stralloc_starts(&line, "Delivered-To: "))
		return;
	if (line.s[line.len - 1] != '\n')
		return;
	if (!stralloc_copyb(&recipient, line.s + 14, line.len - 15))
		die_nomem();
	if (!stralloc_catb(&line, "\0", 1))
		die_nomem();
	env_put2("DTLINE", line.s);

	/*
	 * Do some quoting on the sender and recipient. 
	 */
	if (!stralloc_0(&sender))
		die_nomem();
	if (!quote2(&quosender, sender.s))
		die_nomem();
	if (!stralloc_0(&recipient))
		die_nomem();

	/*
	 * Export some environment variables. 
	 */
	env_put2("SENDER", sender.s);
	env_put2("RECIPIENT", recipient.s);

	/*
	 * Derive the rest of the environment variables. 
	 */
	makeenv();

	/*
	 * Run the command now. 
	 */
	argv[0] = "/bin/sh";
	argv[1] = "-c";
	runcmd(argv, fd, fnam);
}

int
main(int argc, char *argv[])
{
	int             match;
	stralloc        fname = { 0 };
	char            fnamebuf[4096];
	substdio        ssfname;
	int             fd;

	use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
	substdio_fdbuf(&ssfname, read, 0, fnamebuf, sizeof fnamebuf);
	if (getln(&ssfname, &fname, &match, '\0') == -1)
		die_readstdin();
	if (!match)
		die_readstdin();
	if ((fd = open_read(fname.s)) == -1)
		die_openmess();

	doit(fd, argv, &fname);
	/*- Not reached */
	return(0);
}

/*
 * $Log: serialcmd.c,v $
 * Revision 1.7  2022-12-18 12:29:59+05:30  Cprogrammer
 * handle wait status with details
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2021-07-05 21:24:39+05:30  Cprogrammer
 * use qgetpw interface from libqmail if USE_QPWGR is set
 *
 * Revision 1.4  2004-10-22 20:30:13+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:38:57+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.2  2004-09-26 00:00:51+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.1  2004-05-14 00:45:09+05:30  Cprogrammer
 * Initial revision
 *
 */

void
getversion_serialcmd_c()
{
	static char    *x = "$Id: serialcmd.c,v 1.7 2022-12-18 12:29:59+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
