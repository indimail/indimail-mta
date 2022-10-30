/*
 * $Log: sys-checkpwd.c,v $
 * Revision 1.15  2022-10-30 10:03:39+05:30  Cprogrammer
 * display auth method in logs if DEBUG is set
 *
 * Revision 1.14  2022-01-30 09:44:47+05:30  Cprogrammer
 * replaced execvp with execv
 *
 * Revision 1.13  2021-09-12 12:55:30+05:30  Cprogrammer
 * relinquish setuid in pipe_exec()
 *
 * Revision 1.12  2021-07-05 21:24:53+05:30  Cprogrammer
 * use qgetpw interface from libqmail if USE_QPWGR is set
 *
 * Revision 1.11  2021-06-15 12:22:24+05:30  Cprogrammer
 * moved makeargs.h to libqmail
 *
 * Revision 1.10  2021-05-26 10:47:37+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.9  2020-09-28 13:11:38+05:30  Cprogrammer
 * added pid in debug log for authmodule
 *
 * Revision 1.8  2020-09-28 13:08:19+05:30  Cprogrammer
 * display authmodule executed in debug logs
 *
 * Revision 1.7  2020-09-28 00:06:20+05:30  Cprogrammer
 * skip stripping of domain if STRIP_DOMAIN env variable is not set
 *
 * Revision 1.6  2020-09-27 23:44:25+05:30  Cprogrammer
 * restore '@' sign in username
 *
 * Revision 1.5  2020-04-01 19:00:49+05:30  Cprogrammer
 * display uid in debug login mode
 *
 * Revision 1.4  2020-04-01 16:15:33+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.3  2010-06-08 22:00:54+05:30  Cprogrammer
 * pathexec() now returns allocated environment variable which should be freed
 *
 * Revision 1.2  2010-05-05 11:48:44+05:30  Cprogrammer
 * fixed calls to my_error
 *
 * Revision 1.1  2010-04-21 20:01:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "hasspnam.h"
#ifdef HASGETSPNAM
#include <shadow.h>
#endif
#include "hasuserpw.h"
#ifdef HASUSERPW
#include <userpw.h>
#endif
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <authmethods.h>
#include <fmt.h>
#include <error.h>
#include <env.h>
#include <subfd.h>
#include <in_crypt.h>
#include <pw_comp.h>
#include <makeargs.h>
#include <qgetpwgr.h>

#define FATAL "sys-checkpwd: fatal: "
#define WARN  "sys-checkpwd: warn: "

static int      debug;

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
print_error(char *str)
{
	out("454-");
	out(str);
	out(": ");
	out(error_str(errno));
	out(" (#4.3.0)\r\n");
	flush();
}

static int
runcmmd(char *cmmd)
{
	char          **argv;
	int             status, i, retval;
	pid_t           pid;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	switch ((pid = fork()))
	{
	case -1:
		_exit(111);
	case 0:
		if (!(argv = makeargs(cmmd)))
			_exit(111);
		execv(*argv, argv);
		strerr_die2sys(111, *argv, ": ");
	default:
		break;
	}
	for (retval = -1;;) {
		i = wait(&status);
#ifdef ERESTART
		if (i == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (i == -1 && errno == EINTR)
#endif
			continue;
		else
		if (i == -1)
			break;
		if (i != pid)
			continue;
		if (WIFSTOPPED(status) || WIFSIGNALED(status)) {
			if (debug) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_uint(strnum2, WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status))] = 0;
				strerr_warn3(strnum1, ": killed by signal ", strnum2, 0);
			}
			retval = -1;
		} else
		if (WIFEXITED(status)) {
			retval = WEXITSTATUS(status);
			if (debug) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_uint(strnum2, retval < 0 ? 0 - retval : retval)] = 0;
				strerr_warn4(strnum1, ": normal exit return status", retval < 0 ? " -" : " ", strnum2, 0);
			}
		}
		break;
	}
	return (retval);
}

static void
pipe_exec(char **argv, char *tmpbuf, int len, int restore)
{
	int             pipe_fd[2];
	char            strnum[FMT_ULONG];

	if (!geteuid() && setuid(getuid()))
		strerr_die4sys(111, FATAL, "setuid: uid(", strnum, "):");
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (debug)
		strerr_warn5("sys-checkpwd: pid [", strnum, "] executing authmodule [", argv[1], "]", 0);
	if (pipe(pipe_fd) == -1)
		strerr_die2sys(111, FATAL, "pipe: ");
	if (dup2(pipe_fd[0], 3) == -1 || dup2(pipe_fd[1], 4) == -1)
		strerr_die2sys(111, FATAL, "dup: ");
	if (pipe_fd[0] != 3 && pipe_fd[0] != 4)
		close(pipe_fd[0]);
	if (pipe_fd[1] != 3 && pipe_fd[1] != 4)
		close(pipe_fd[1]);
	if (restore > 0)
		tmpbuf[restore] = '@';
	if (write(4, tmpbuf, len) != len)
		strerr_die2sys(111, FATAL, "write: ");
	close(4);
	execv(argv[1], argv + 1);
	strerr_die3sys(111, FATAL, "exec: ", argv[1]);
}


int
main(int argc, char **argv)
{
	char           *ptr, *tmpbuf, *login, *response, *challenge, *stored;
	char            strnum[FMT_ULONG];
	static stralloc buf = {0};
	int             i, count, offset, status, save = -1, use_pwgr, authlen = 512,
					auth_method;
	struct passwd  *pw;
#ifdef HASUSERPW
	struct userpw  *upw;
#endif
#ifdef HASGETSPNAM
	struct spwd    *spw;
#endif

	if (argc < 2)
		_exit(2);
	use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
	debug = env_get("DEBUG") ? 1 : 0;
	if (!(tmpbuf = alloc((authlen + 1) * sizeof(char)))) {
		print_error("out of memory");
		strnum[fmt_uint(strnum, (unsigned int) authlen + 1)] = 0;
		strerr_warn3("alloc-", strnum, ": ", &strerr_sys);
		_exit(111);
	}
	for (offset = 0;;) {
		do {
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while(count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while(count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			print_error("read error");
			strerr_die2sys(111, FATAL, "read: ");
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = tmpbuf + count; /*- username */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	challenge = tmpbuf + count; /*- challenge */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	response = tmpbuf + count; /*- response */
	for (; tmpbuf[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		auth_method = 0;
	else
		auth_method = tmpbuf[count + 1];

	if (env_get("STRIP_DOMAIN")) { /*- set this for roundcubemail */
		i = str_chr(login, '@');
		if (login[i]) {
			login[i] = 0;
			save = i;
		}
	}
	if (!(pw = (use_pwgr ? qgetpwnam : getpwnam) (login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset, save);
		else {
			print_error("getpwnam");
			strerr_die2sys(111, FATAL, "getpwnam: ");
		}
		_exit (111);
	}
	stored = pw->pw_passwd;
#ifdef HASUSERPW
	if (!(upw = getuserpw(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset, save);
		else {
			print_error("getuserpw");
			strerr_die2sys(111, FATAL, "getuserpw: ");
		}
	}
	stored = upw->upw_passwd;
#endif
#ifdef HASGETSPNAM
	if (!(spw = getspnam(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset, save);
		else {
			print_error("getspnam");
			strerr_die2sys(111, FATAL, "getspnam: ");
		}
	}
	stored = spw->sp_pwdp;
#endif
	if (setuid(getuid()))
		strerr_die4sys(111, FATAL, "setuid: uid(", strnum, "):");
	strnum[fmt_ulong(strnum, getuid())] = 0;
	if (env_get("DEBUG_LOGIN")) {
		i = str_rchr(argv[0], '/');
		ptr = get_authmethod(auth_method);
		strerr_warn14(argv[0][i] ? argv[0] + i + 1 : argv[0], ": uid(",
				strnum, ") login [", login, "] challenge [", challenge,
				"] response [", response, "] pw_passwd [", stored,
				"] auth_method [", ptr, "]", 0);
	} else
	if (debug) {
		i = str_rchr(argv[0], '/');
		ptr = get_authmethod(auth_method);
		strerr_warn8(argv[0][i] ? argv[0] + i + 1 : argv[0], ": uid(",
				strnum, ") login [", login, "] auth_method [", ptr, "]", 0);
	}
	if (pw_comp((unsigned char *) login, (unsigned char *) stored,
			(unsigned char *) (*response ? challenge : 0),
			(unsigned char *) (*response ? response : challenge), 0))
		pipe_exec(argv, tmpbuf, offset, save); /*- never returns */
	status = 0;
	if ((ptr = (char *) env_get("POSTAUTH"))) {
		if (!access(ptr, X_OK)) {
			if (stralloc_copys(&buf, ptr) || !stralloc_append(&buf, " ")
					|| !stralloc_cats(&buf, login) || !stralloc_0(&buf))
				strerr_die2x(111, WARN, "out of memory");
			status = runcmmd(buf.s);
		} else
		if (errno != error_noent)
			strerr_die4sys(111, FATAL, "unable to access ", ptr, ": ");
	}
	_exit(status);
	/*- Not reached */
	return (0);
}

#ifndef lint
void
getversion_sys_checkpwd_c()
{
	static char    *x = "$Id: sys-checkpwd.c,v 1.15 2022-10-30 10:03:39+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmakeargsh;
	x++;
}
#endif
