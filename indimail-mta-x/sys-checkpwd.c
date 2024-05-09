/*
 * $Id: sys-checkpwd.c,v 1.21 2024-04-30 08:28:37+05:30 Cprogrammer Exp mbhangui $
 *
 * Test method
 * printf "login\0pass\0\0\x01\0" >/tmp/input
 * as root run
 * env DEBUG=1 DEBUG_LOGIN=1 sys-checkpwd /bin/false < /tmp/input 3<&0
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
#include <wait.h>
#include <error.h>
#include <env.h>
#include <subfd.h>
#include <in_crypt.h>
#include <pw_comp.h>
#include <makeargs.h>
#include <qprintf.h>
#include <qgetpwgr.h>

#define FATAL "sys-checkpwd: fatal: "
#define WARN  "sys-checkpwd: warn: "

static int      debug;

void
print_error(const char *str)
{
	subprintf(subfdout, "454-%s: %s (#4.3.0)\r\n", str, error_str(errno));
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
}

static int
runcmmd(const char *cmmd)
{
	char          **argv;
	int             status, i, werr, retval;
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
		if (!(i = waitpid(pid, &status, WUNTRACED|WCONTINUED)))
			break;
		else
		if (i == -1) {
#ifdef ERESTART
			if (errno == error_intr || errno == error_restart)
#else
			if (errno == error_intr)
#endif
				continue;
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_warn3(FATAL, strnum1, ": waitpid error: ", &strerr_sys);
			break;
		}
		if (i != pid) {
			strerr_warn3(FATAL, strnum1, ": waitpid surprise: ", 0);
			continue;
		}
		if (!(i = wait_handler(status, &werr)) && werr) {
			if (debug) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strnum2[fmt_int(strnum2, werr)] = 0;
				strerr_warn4(WARN, strnum1, wait_stopped(status) ? ": stopped by signal " : ": started by signal ", strnum2, 0);
			}
			continue;
		} else
		if (werr == -1) {
			if (debug) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strerr_warn3(FATAL, strnum1, ": internal wait handler error", 0);
			}
		} else
		if (werr) {
			if (debug) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strnum2[fmt_uint(strnum2, WTERMSIG(status))] = 0;
				strerr_warn4(FATAL, strnum1, ": killed by signal ", strnum2, 0);
			}
		} else {
			if (debug) {
				strnum1[fmt_ulong(strnum1, pid)] = 0;
				strnum2[fmt_int(strnum2, i)] = 0;
				strerr_warn4(WARN, strnum1, ": normal exit return status ", strnum2, 0);
			}
			retval = i;
		}
		break;
	} /*- for (retval = -1;;) */
	return (retval);
}

static void
pipe_exec(char **argv, char *tmpbuf, int len, int restore)
{
	int             pipe_fd[2];
	char            strnum[FMT_ULONG];

	if (!geteuid() && setuid(getuid()))
		strerr_die4sys(111, FATAL, "setuid: uid(", strnum, "):");
	if (debug)
		subprintf(subfderr, "sys-checkpwd: pid=%d, executing authmodule=%s\n", getpid(), argv[1]);
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
					auth_method, enable_cram;
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
	enable_cram = env_get("ENABLE_CRAM") ? 1 : 0;
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
	challenge = tmpbuf + count; /*- challenge for CRAM methods (or plain text password for LOGIN/PLAIN) */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	response = tmpbuf + count; /*- response (CRAM methods, etc) */
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
	if (enable_cram)
		stored = pw->pw_passwd;
	else
		stored = (auth_method >= AUTH_CRAM_MD5 && auth_method <= AUTH_DIGEST_MD5) ? NULL : pw->pw_passwd;
#ifdef HASUSERPW
	if (!(upw = getuserpw(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset, save);
		else {
			print_error("getuserpw");
			strerr_die2sys(111, FATAL, "getuserpw: ");
		}
	}
	if (enable_cram)
		stored = upw->upw_passwd
	else /*- fail all CRAM auth methods if enable cram isn't set */
		stored = (auth_method >= AUTH_CRAM_MD5 && auth_method <= AUTH_DIGEST_MD5) ? NULL : upw->upw_passwd;
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
	if (enable_cram)
		stored = spw->sp_pwdp;
	else /*- fail all CRAM auth methods if enable cram isn't set */
		stored = (auth_method >= AUTH_CRAM_MD5 && auth_method <= AUTH_DIGEST_MD5) ? NULL : spw->sp_pwdp;
#endif
	if (setuid(getuid()))
		strerr_die4sys(111, FATAL, "setuid: uid(", strnum, "):");
	if (env_get("DEBUG_LOGIN")) {
		i = str_rchr(argv[0], '/');
		ptr = (char *) get_authmethod(auth_method);
		subprintf(subfderr,
				"%s: uid=%u, login=%s, challenge=%s, response=%s, encrypted=%s, CRAM=%s AUTH=%s",
				argv[0][i] ? argv[0] + i + 1 : argv[0],
				getuid(), login, challenge, response, stored ? stored : "null",
				enable_cram ? "Yes" : "No", ptr);
#ifdef HASUSERPW
		subprintf(subfderr, ", USERPW=YES");
#else
		subprintf(subfderr, ", USERPW=NO");
#endif
#ifdef HASGETSPNAM
		subprintf(subfderr, ", SHADOW=YES");
#else
		subprintf(subfderr, ", SHADOW=NO");
#endif
		subprintf(subfderr, "\n");
		substdio_flush(subfderr);
	} else
	if (debug) {
		i = str_rchr(argv[0], '/');
		ptr = (char *) get_authmethod(auth_method);
		subprintf(subfderr, "%s: uid=%u, login=%s, CRAM=%s AUTH=%s",
				argv[0][i] ? argv[0] + i + 1 : argv[0],
				getuid(), login, enable_cram ? "Yes" : "No", ptr);
#ifdef HASUSERPW
		subprintf(subfderr, ", USERPW=YES");
#else
		subprintf(subfderr, ", USERPW=NO");
#endif
#ifdef HASGETSPNAM
		subprintf(subfderr, ", SHADOW=YES");
#else
		subprintf(subfderr, ", SHADOW=NO");
#endif
		subprintf(subfderr, "\n");
		substdio_flush(subfderr);
	}
	if (!stored)
		pipe_exec(argv, tmpbuf, offset, save);
	if (pw_comp((unsigned char *) login, (unsigned char *) stored,
			(unsigned char *) (*response ? challenge : 0),
			(unsigned char *) (*response ? response : challenge), auth_method))
		pipe_exec(argv, tmpbuf, offset, save); /*- never returns */
	status = 0;
	if ((ptr = (char *) env_get("POSTAUTH"))) {
		if (!access(ptr, X_OK)) {
			if (!stralloc_copys(&buf, ptr) || !stralloc_append(&buf, " ")
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
	const char     *x = "$Id: sys-checkpwd.c,v 1.21 2024-04-30 08:28:37+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidmakeargsh;
	x++;
}
#endif

/*
 * $Log: sys-checkpwd.c,v $
 * Revision 1.21  2024-04-30 08:28:37+05:30  Cprogrammer
 * display in logs if userpw, shadow is enabled for encrypted password lookup
 *
 * Revision 1.20  2023-07-13 02:43:57+05:30  Cprogrammer
 * replaced out() with subprintf()
 *
 * Revision 1.19  2023-03-26 08:23:19+05:30  Cprogrammer
 * fixed code for wait_handler
 *
 * Revision 1.18  2023-02-22 00:01:10+05:30  Cprogrammer
 * sys-checkpwd.c: replaced strerr_warn with subprintf
 *
 * Revision 1.17  2022-12-18 12:30:10+05:30  Cprogrammer
 * handle wait status with details
 *
 * Revision 1.16  2022-11-05 22:41:26+05:30  Cprogrammer
 * Use ENABLE_CRAM env variable to enable CRAM authentication using encrypted password
 *
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
