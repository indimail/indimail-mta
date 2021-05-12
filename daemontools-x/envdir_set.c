/*
 * $Log: envdir_set.c,v $
 * Revision 1.2  2021-05-12 18:51:24+05:30  Cprogrammer
 * fixed error message
 *
 * Revision 1.1  2010-06-08 19:06:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "error.h"
#include "open.h"
#include "pathexec.h"
#include "openreadclose.h"
#include "byte.h"
#include "strerr.h"
#include "direntry.h"
#include "stralloc.h"

static stralloc sa;

void
envdir_set(char *fn)
{
	DIR            *dir;
	direntry       *d;
	int             i, fdorigdir;

	if ((fdorigdir = open_read(".")) == -1)
		strerr_die1sys(111, "unable to open current directory: ");
	if (chdir(fn) == -1)
		strerr_die3sys(111, "unable to switch to directory ", fn, ": ");
	if (!(dir = opendir(".")))
		strerr_die3sys(111, "unable to read directory ", fn, ": ");
	for (;;) {
		errno = 0;
		if (!(d = readdir(dir))) {
			if (errno)
				strerr_die3sys(111, "unable to read directory ", fn, ": ");
			break;
		}
		if (d->d_name[0] != '.') {
			if (openreadclose(d->d_name, &sa, 256) == -1)
				strerr_die5sys(111, "unable to read ", fn, "/", d->d_name, ": ");
			if (sa.len) {
				sa.len = byte_chr(sa.s, sa.len, '\n');
				while (sa.len) {
					if (sa.s[sa.len - 1] != ' ' && sa.s[sa.len - 1] != '\t')
						break;
					--sa.len;
				}
				for (i = 0; i < sa.len; ++i) {
					if (!sa.s[i])
						sa.s[i] = '\n';
				}
				if (!stralloc_0(&sa))
					strerr_die1x(111, "out of memory");
				if (!pathexec_env(d->d_name, sa.s))
					strerr_die1x(111, "out of memory");
			} else {
				if (!pathexec_env(d->d_name, 0))
					strerr_die1x(111, "out of memory");
			}
		}
	}
	closedir(dir);
	if (fchdir(fdorigdir) == -1)
		strerr_die1sys(111, "unable to switch to starting directory: ");
	close(fdorigdir);
	return;
}

void
getversion_envdir_set_c()
{
	static char    *x = "$Id: envdir_set.c,v 1.2 2021-05-12 18:51:24+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
