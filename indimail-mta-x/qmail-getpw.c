/*
 * $Log: qmail-getpw.c,v $
 * Revision 1.7  2020-11-24 13:46:50+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2004-10-22 20:28:20+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:36:51+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.4  2004-07-17 21:20:38+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <substdio.h>
#include <subfd.h>
#include <error.h>
#include <byte.h>
#include <str.h>
#include <case.h>
#include <fmt.h>
#include <env.h>
#include <qgetpwgr.h>
#include "auto_usera.h"
#include "auto_break.h"
#include "qlx.h"

#define GETPW_USERLEN 32

static char    *local, *dash, *extension;
static int      use_pwgr;
static struct passwd  *pw;

int
userext()
{
	char            username[GETPW_USERLEN];
	struct stat     st;

	extension = local + str_len(local);
	for (;;) {
		if (extension - local < sizeof(username)) {
			if (!*extension || (*extension == *auto_break)) {
				byte_copy(username, extension - local, local);
				username[extension - local] = 0;
				case_lowers(username);
				errno = 0;
				pw = (use_pwgr ? qgetpwnam : getpwnam) (username);
				if (errno == error_txtbsy)
					_exit(QLX_SYS);
				if (pw) {
					if (pw->pw_uid) {
						if (stat(pw->pw_dir, &st) == 0) {
							if (st.st_uid == pw->pw_uid) {
								dash = "";
								if (*extension) {
									++extension;
									dash = "-";
								}
								return 1;
							}
						} else
						if (error_temp(errno))
							_exit(QLX_NFS);
					}
				}
			}
		}
		if (extension == local)
			return 0;
		--extension;
	}
}

char            num[FMT_ULONG];

int
main(int argc, char **argv)
{
	if (!(local = argv[1]))
		_exit(100);
	use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
	if (!userext()) {
		extension = local;
		dash = "-";
		pw = (use_pwgr ? qgetpwnam : getpwnam) (auto_usera);
	}
	if (!pw)
		_exit(QLX_NOALIAS);
	substdio_puts(subfdoutsmall, pw->pw_name);
	substdio_put(subfdoutsmall, "", 1);
	substdio_put(subfdoutsmall, num, fmt_ulong(num, (long) pw->pw_uid));
	substdio_put(subfdoutsmall, "", 1);
	substdio_put(subfdoutsmall, num, fmt_ulong(num, (long) pw->pw_gid));
	substdio_put(subfdoutsmall, "", 1);
	substdio_puts(subfdoutsmall, pw->pw_dir);
	substdio_put(subfdoutsmall, "", 1);
	substdio_puts(subfdoutsmall, dash);
	substdio_put(subfdoutsmall, "", 1);
	substdio_puts(subfdoutsmall, extension);
	substdio_put(subfdoutsmall, "", 1);
	substdio_flush(subfdoutsmall);
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_getpw_c()
{
	static char    *x = "$Id: qmail-getpw.c,v 1.7 2020-11-24 13:46:50+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
