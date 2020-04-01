/*
 * $Log: sys-checkpwd.c,v $
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
#include "alloc.h"
#include "stralloc.h"
#include "strerr.h"
#include "str.h"
#include "fmt.h"
#include "error.h"
#include "env.h"
#include "subfd.h"
#include "in_crypt.h"
#include "pw_comp.h"
#include "MakeArgs.h"

#define FATAL "sys-checkpwd: fatal: "

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
	int             status, i, retval, debug;
	pid_t           pid;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];

	debug = env_get("DEBUG") ? 1 : 0;
	switch ((pid = fork()))
	{
	case -1:
		_exit(111);
	case 0:
		if (!(argv = MakeArgs(cmmd)))
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
pipe_exec(char **argv, char *tmpbuf, int len)
{
	int             pipe_fd[2];

	if (pipe(pipe_fd) == -1)
		strerr_die2sys(111, FATAL, "pipe: ");
	if (dup2(pipe_fd[0], 3) == -1 || dup2(pipe_fd[1], 4) == -1)
		strerr_die2sys(111, FATAL, "dup: ");
	if (pipe_fd[0] != 3 && pipe_fd[0] != 4)
		close(pipe_fd[0]);
	if (pipe_fd[1] != 3 && pipe_fd[1] != 4)
		close(pipe_fd[1]);
	if (write(4, tmpbuf, len) != len)
		strerr_die2sys(111, FATAL, "write: ");
	close(4);
	execvp(argv[1], argv + 1);
	strerr_die3sys(111, FATAL, "exec: ", argv[1]);
}

int             authlen = 512;

int
main(int argc, char **argv)
{
	char           *ptr, *tmpbuf, *login, *response, *challenge, *stored;
	char            strnum[FMT_ULONG];
	static stralloc buf = {0};
	int             i, count, offset, status;
	struct passwd  *pw;
#ifdef HASUSERPW
	struct userpw  *upw;
#endif
#ifdef HASGETSPNAM
	struct spwd    *spw;
#endif

	if (argc < 2)
		_exit(2);
	if (!(tmpbuf = alloc((authlen + 1) * sizeof(char)))) {
		print_error("out of memory");
		strnum[fmt_uint(strnum, (unsigned int) authlen + 1)] = 0;
		strerr_warn3("alloc-", strnum, ": ", &strerr_sys);
		_exit(111);
	}
	for (offset = 0;;) {
		do
		{
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while(count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while(count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			print_error("read error");
			strerr_warn1("syspass: read: ", &strerr_sys);
			_exit(111);
		}
		else
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
	response = tmpbuf + count + 1; /*- response */
	i = str_chr(login, '@');
	if (login[i])
		login[i] = 0;
	if (!(pw = getpwnam(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			strerr_warn1("syspass: getpwnam: ", &strerr_sys);
		print_error("getpwnam");
		_exit (111);
	}
	stored = pw->pw_passwd;
#ifdef HASUSERPW
	if (!(upw = getuserpw(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			strerr_warn1("syspass: getuserpw: ", &strerr_sys);
		print_error("getuserpw");
		_exit (111);
	}
	stored = upw->upw_passwd;
#endif
#ifdef HASGETSPNAM
	if (!(spw = getspnam(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			strerr_warn1("syspass: getspnam: ", &strerr_sys);
		print_error("getspnam");
		_exit (111);
	}
	stored = spw->sp_pwdp;
#endif
	if (env_get("DEBUG_LOGIN")) {
		out(argv[0]);
		out(": ");
		out("login [");
		out(login);
		out("] challenge [");
		out(challenge);
		out("] response [");
		out(response);
		out("] pw_passwd [");
		out(stored);
		out("]\n");
		flush();
	}
	if (pw_comp((unsigned char *) login, (unsigned char *) stored,
		(unsigned char *) (*response ? challenge : 0),
		(unsigned char *) (*response ? response : challenge), 0))
	{
		pipe_exec(argv, tmpbuf, offset);
		print_error("exec");
		_exit (111);
	}
	status = 0;
	if ((ptr = (char *) env_get("POSTAUTH")) && !access(ptr, X_OK)) {
		if (stralloc_copys(&buf, ptr) ||
				!stralloc_append(&buf, " ") ||
				!stralloc_cats(&buf, login) ||
				!stralloc_0(&buf)) {
			strerr_warn1("syspass: out of memory: ", &strerr_sys);
			_exit (111);
		}
		status = runcmmd(buf.s);
	}
	_exit(status);
	/*- Not reached */
	return (0);
}

void
getversion_sys_checkpwd_c()
{
	static char    *x = "$Id: sys-checkpwd.c,v 1.4 2020-04-01 16:15:33+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
