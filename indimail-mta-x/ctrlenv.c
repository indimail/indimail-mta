/*
 * $Log: ctrlenv.c,v $
 * Revision 1.1  2020-04-08 16:02:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "sgetopt.h"
#include "control.h"
#include "strerr.h"
#include "case.h"
#include "str.h"
#include "stralloc.h"
#include "alloc.h"
#include "pathexec.h"

#include "uint32.h"
#include "alloc.h"
#include "cdb.h"
#include "error.h"
#include "open.h"
#include "env.h"
#include "variables.h"
#include "auto_control.h"

#define FATAL "ctrlenv: fatal: "

stralloc        ctrl = {0};

int
cdb_match(char *fn, char *addr, int len, char **result)
{
	static stralloc controlfile = {0};
	static stralloc temp = { 0 };
	uint32          dlen;
	char           *tmpbuf;
	int             fd_cdb, cntrl_ok, i;

	if (!len || !*addr || !fn)
		return (0);
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&controlfile, controldir) || !stralloc_cats(&controlfile, "/")
			|| !stralloc_cats(&controlfile, fn) || !stralloc_0(&controlfile))
		strerr_die2sys(111, FATAL, "out of memory");
	if ((fd_cdb = open_read(controlfile.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, FATAL, controlfile.s, ": ");
		/*- cdb missing or entry missing */
		*result = (char *) 0;
		return (0);
	}
	if (!stralloc_copyb(&temp, "!", 1)) {
		close(fd_cdb);
		strerr_die2sys(111, FATAL, "out of memory");
	}
	if (!stralloc_catb(&temp, addr, len) || !stralloc_0(&temp)) {
		close(fd_cdb);
		strerr_die2sys(111, FATAL, "out of memory");
	}
	if ((cntrl_ok = cdb_seek(fd_cdb, temp.s, len + 1, &dlen)) == -1) {
		close(fd_cdb);
		strerr_die2sys(111, FATAL, "lseek: ");
	} else
	if (cntrl_ok == 1) {
		if (!(tmpbuf = (char *) alloc(dlen + 1))) {
			close(fd_cdb);
			strerr_die2sys(111, FATAL, "out of memory");
		}
		if ((i = read(fd_cdb, tmpbuf, dlen)) == -1)
			strerr_die4sys(111, FATAL, "read: ", controlfile.s, ": ");
		else
		if (!i)
			strerr_die4x(111, FATAL, "read: ", controlfile.s, ": EOF");
		tmpbuf[dlen] = 0;
		*result = tmpbuf;
	}
	close(fd_cdb);
	return (cntrl_ok ? 1 : 0);
}

int
main(int argc, char **argv)
{
	char           *fn = (char *) 0, *ptr, *env_name = (char *) 0, *addr = (char *) 0, *result;
	int             i, j, opt, token_len, len;

	while ((opt = getopt(argc, argv, "f:e:a:")) != opteof) {
		switch (opt) {
		case 'f':
			fn = optarg;
			break;
		case 'e':
			env_name = optarg;
			break;
		case 'a':
			addr = optarg;
			break;
		}
	}
	if (optind == argc)
		strerr_die1x(100, "usage: cntrlenv -f filename -e env -a address child");
	if (!fn || !env_name || !addr)
		strerr_die1x(100, "usage: cntrlenv -f filename -e env -a address child");
	i = str_end(fn, ".cdb");
	if (fn[i]) {
		if (cdb_match(fn, addr, str_len(addr), &result)) {
			if (!pathexec_env(env_name, result))
				strerr_die2sys(111, FATAL, "out of memory");
			pathexec(argv + optind);
		} else
			pathexec(argv + optind);
		strerr_die4sys(111, FATAL, "unable to run ", argv[optind], ": ");
	}
	if ((opt = control_readfile(&ctrl, fn, 0)) == -1)
		strerr_die3sys(111, FATAL, fn, ": ");
	for (len = 0, ptr = ctrl.s; len < ctrl.len; ) {
		i = str_chr(ptr, ':');
		if (ptr[i]) {
			if (!case_diffb(addr, i - 1, ptr + 1)) {
				j = str_chr(ptr + i + 1, ':');
				ptr[i + 1 + j] = 0;
				if (!pathexec_env(env_name, ptr + i + 1))
					strerr_die2sys(111, FATAL, "out of memory");
				pathexec(argv + optind);
				strerr_die4sys(111, FATAL, "unable to run ", argv[optind], ": ");
			}
		}
		len += ((token_len = str_len(ptr)) + 1);
		ptr += token_len + 1;
	}
	pathexec(argv + optind);
}

void
getversion_ctrlenv_c()
{
	static char    *x = "$Id: ctrlenv.c,v 1.1 2020-04-08 16:02:19+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
