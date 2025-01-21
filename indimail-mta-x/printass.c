/*
 * $Id: printass.c,v 1.3 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sgetopt.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <qprintf.h>
#include <env.h>
#include <open.h>
#include <str.h>
#include <ctype.h>
#include <getln.h>
#include "read_assign.h"
#include "auto_assign.h"

#define FATAL "readass: fatal: "

stralloc        dir, line;

no_return void
die_readass()
{
	strerr_die2sys(111, FATAL, "unable to read assign file: ");
}

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memeroy");
}

void
printall(int fd, char p_dir, char p_uid, char p_gid)
{
	substdio        ssass;
	char            assbuf[512];
	char           *ptr, *user, *dirp, *uidp, *gidp;
	int             i, match;

	substdio_fdbuf(&ssass, (ssize_t (*)(int,  char *, size_t)) read, fd, assbuf, sizeof assbuf);
	for (;;) {
		if (getln(&ssass, &line, &match, '\n') == -1)
			die_readass();
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (*ptr != '+' && *ptr != '=') /*- ignore lines that do not start with '+' or '=' */
			continue;
		i = str_chr(ptr, ':');
		if (!ptr[i])
			continue;
		user = ptr + i + 1;
		if (!*user)
			continue;
		i = str_chr(user, ':');
		if (user[i])
			user[i] = 0;
		else
			continue;
		uidp = user + i + 1;
		i = str_chr(uidp, ':');
		if (uidp[i])
			uidp[i] = 0;
		else
			continue;
		gidp = uidp + i + 1;
		i = str_chr(gidp, ':');
		if (gidp[i])
			gidp[i] = 0;
		else
			continue;
		dirp = gidp + i + 1;
		i = str_chr(dirp, ':');
		if (dirp[i]) {
			dirp[i] = 0;
		} else
			continue;

		subprintf(subfdoutsmall, "%s ", user);
		if (p_dir)
			subprintf(subfdoutsmall, "%s", dirp);

		if (p_uid) {
			if (p_dir)
				substdio_put(subfdoutsmall, " ", 1);
			subprintf(subfdoutsmall, "%s", uidp);
		}

		if (p_gid) {
			if (p_dir || p_uid)
				substdio_put(subfdoutsmall, " ", 1);
			subprintf(subfdoutsmall, "%s", gidp);
		}
		substdio_put(subfdoutsmall, "\n", 1);
	} /* for (;;) */
	substdio_flush(subfdoutsmall);
}

int
main(int argc, char **argv)
{
	int             fd, opt;
	char            p_dir, p_uid, p_gid;
	char           *cdbdir;
	uid_t           uid;
	gid_t           gid;

	p_dir = p_uid = p_gid = 0;
	while ((opt = getopt(argc, argv, "dug")) != opteof) {
		switch (opt)
		{
		case 'd':
			p_dir = 1;
			break;
		case 'u':
			p_uid = 1;
			break;
		case 'g':
			p_gid = 1;
			break;
		}
	}
	if (!p_dir && !p_uid && !p_gid)
		p_dir = p_uid = p_gid = 1;
	if (!(cdbdir = env_get("ASSIGNDIR")))
		cdbdir = auto_assign;
	if (optind == argc ) { /*- no args */
		if (chdir(cdbdir) == -1)
			strerr_die4sys(111, FATAL, "chdir: ", cdbdir, ": ");
		if ((fd = open_read("assign")) == -1)
			strerr_die3sys(111, FATAL, cdbdir, "/assign:");
		printall(fd, p_dir, p_uid, p_gid);
		return 0;
	}

	for (;optind < argc; optind++) {
		if (!read_assign(argv[optind], &dir, &uid, &gid))
			strerr_die5x(111, "unable to get entry for ", argv[optind], " in ", cdbdir, "/cdb");

		subprintf(subfdoutsmall, "%s ", argv[optind]);
		if (p_dir)
			subprintf(subfdoutsmall, "%s", dir.s);

		if (p_uid) {
			if (p_dir)
				substdio_put(subfdoutsmall, " ", 1);
			subprintf(subfdoutsmall, "%u", uid);
		}

		if (p_gid) {
			if (p_dir || p_uid)
				substdio_put(subfdoutsmall, " ", 1);
			subprintf(subfdoutsmall, "%u", gid);
		}
		substdio_put(subfdoutsmall, "\n", 1);
	}
	substdio_flush(subfdoutsmall);
	return 0;
}

#ifndef lint
void
getversion_printass_c()
{
	const char     *x = "$Id: printass.c,v 1.3 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif

/*
 * $Log: printass.c,v $
 * Revision 1.3  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2023-12-10 10:18:52+05:30  Cprogrammer
 * Initial revision
 *
 */
