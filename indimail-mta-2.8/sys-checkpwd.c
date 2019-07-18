/*
 * $Log: sys-checkpwd.c,v $
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
#include "alloc.h"
#include "error.h"
#include "str.h"
#include "pathexec.h"
/*----*/
#include "subfd.h"
#include "substdio.h"
#include "env.h"
#include "strerr.h"

extern char    *crypt();
#include <pwd.h>
static struct passwd *pw;
#include "hasspnam.h"
#ifdef HASGETSPNAM
#include <shadow.h>
static struct spwd *spw;
#endif
#include "hasuserpw.h"
#ifdef HASUSERPW
#include <userpw.h>
static struct userpw *upw;
#endif

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
logerr(char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
}

void
logerrf(char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
	if (substdio_flush(subfderr) == -1)
		_exit(111);
}

static int      debug;

void
my_error(char *s1, char *s2, int exit_val)
{
	if (!debug)
		_exit(exit_val);
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	if (exit_val > 0)
		logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val > 0 ? exit_val : -exit_val);
}

static char     up[513];
static int      uplen;

int
main(int argc, char **argv)
{
	char           *login, *password, *encrypted, *stored = 0;
	char          **e;
	int             r, i, tmperrno;

	if (argc < 2)
		_exit(2);
	if (env_get("DEBUG"))
		debug = 1;
	uplen = 0;
	for (;;) {
		do
			r = read(3, up + uplen, sizeof (up) - uplen);
		while ((r == -1) && (errno == error_intr));
		if (r == -1)
			my_error("read", 0, 111);
		if (r == 0)
			break;
		uplen += r;
		if (uplen >= sizeof (up))
			my_error("read", "data too big", -1);
	}
	close(3);
	i = 0;
	if (i >= uplen)
		_exit(2);
	login = up + i;
	while (up[i++])
		if (i >= uplen)
			my_error("invalid data", 0, -2);
	password = up + i;
	if (i >= uplen)
		my_error("invalid data", 0, -2);
	while (up[i++])
		if (i >= uplen)
			my_error("invalid data", 0, -2);
	if (!(pw = getpwnam(login))) {
		if (errno == error_txtbsy)
			my_error("getpwnam", 0, 111);
		my_error("getpwnam", 0, 1);
	} else
		stored = pw->pw_passwd;
#ifdef HASUSERPW
	if (!(upw = getuserpw(login))) {
		if (errno == error_txtbsy)
			my_error("getuserpw", 0, 111);
	} else
		stored = upw->upw_passwd;
#endif
#ifdef HASGETSPNAM
	if (!(spw = getspnam(login))) {
		if (errno == error_txtbsy)
			my_error("getspnam", 0, 111);
	} else
		stored = spw->sp_pwdp;
#endif
	if (!stored || !*stored)
		my_error("getuserpw/getspnam", 0, 1);
	encrypted = crypt(password, stored);
#if 0
	if (debug) {
		out("comparing ");
		out(password);
		out(" stored [");
		out(stored);
		out("] encrypted [");
		out(encrypted);
		out("]\n");
		flush();
	}
#endif
	if (str_diff(encrypted, stored)) { /*- try next module */
		if (debug) {
			out("authentication failure\n");
			flush();
		}
		if ((e = pathexec(argv + 1)))
		{
			tmperrno = errno;
			alloc_free((char *) e);
			errno = tmperrno;
		}
		my_error("exec", 0, 111);
	}
	if (debug) {
		out("authentication success\n");
		flush();
	}
	/*- passwords matched */
	for (i = 0; i < sizeof (up); ++i)
		up[i] = 0;
	_exit(0);
}

void
getversion_sys_checkpwd_c()
{
	static char    *x = "$Id: sys-checkpwd.c,v 1.3 2010-06-08 22:00:54+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
