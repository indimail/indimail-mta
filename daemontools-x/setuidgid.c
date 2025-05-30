/*
 * $Id: setuidgid.c,v 1.12 2025-01-21 23:35:24+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <unistd.h>
#ifdef FREEBSD
#include <sys/param.h>
#endif
#include <pwd.h>
#include <grp.h>
#include <setuserid.h>
#include <strerr.h>
#include <sgetopt.h>
#include <prot.h>
#include <isnum.h>
#include <pathexec.h>
#include <alloc.h>
#include <scan.h>

#define FATAL "setuidgid: fatal: "

int
main(int argc, char **argv)
{
	struct passwd  *pw;
	struct group   *gr;
	gid_t          *gidset = (gid_t *) NULL, g;
	char           *ptr, *cptr, *account, *groups = NULL;
	const char     *usage = "usage: setuidgid [-s] [-g gid_list] account child";
	char          **child;
	int             i, ngroups = 0, opt, old, do_env = 0;

	while ((opt = getopt(argc, argv, "esg:")) != opteof) {
		switch (opt)
		{
		case 'e':
			do_env = 1;
			break;
		case 's':
			ngroups = 1;
			break;
		case 'g':
			groups = optarg;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (argc < optind + 2)
		strerr_die1x(100, usage);
	account = argv[optind++];
	child = argv + optind++;
	if (!(pw = getpwnam(account)))
		strerr_die3x(111, FATAL, "unknown account ", account);
	if (prot_gid(pw->pw_gid) == -1)
		strerr_die2sys(111, FATAL, "unable to set group id: ");
	if (ngroups) {
		if (!(gidset = grpscan(account, &ngroups)))
			strerr_die2sys(111, FATAL, "unable to get groups: ");
	}
	if (groups) {
		old = ngroups;
		for (ptr = groups; *ptr; ptr++) {
			if (*ptr == ',')
				ngroups++;
		}
		ngroups++;
		if (!alloc_re((void **) &gidset, old * sizeof(gid_t), ngroups * sizeof(gid_t)))
			return -1;
		for (i = old, ptr = cptr = groups; *ptr; ptr++) {
			if (*ptr == ',') {
				*ptr = 0;
				if (!isnum(cptr)) {
					if (!(gr = getgrnam(cptr)))
						strerr_die3x(111, FATAL, "unknown group ", cptr);
					g = gr->gr_gid;
				} else
					scan_uint(cptr, &g);
				gidset[i++] = g;
				*ptr = ',';
				cptr = ptr + 1;
			}
		}
		if (!isnum(cptr)) {
			if (!(gr = getgrnam(cptr)))
				strerr_die3x(111, FATAL, "unknown group ", cptr);
			g = gr->gr_gid;
		} else
			scan_uint(cptr, &g);
		gidset[i++] = g;
	}
	if (ngroups) {
		if (setgroups(ngroups, gidset))
			strerr_die2sys(111, FATAL, "unable to add addition group ids: ");
		alloc_free((char *) gidset);
	}
	if (prot_uid(pw->pw_uid) == -1)
		strerr_die2sys(111, FATAL, "unable to set user id: ");
	if (do_env && (!pathexec_env("HOME", pw->pw_dir) || !pathexec_env("USER", account) ||
			!pathexec_env("LOGNAME", account)))
		strerr_die2x(111, FATAL, "out of memory");
	pathexec(child);
	strerr_die4sys(111, FATAL, "unable to run ", *child, ": ");
}

void
getversion_setuidgid_c()
{
	const char     *x = "$Id: setuidgid.c,v 1.12 2025-01-21 23:35:24+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: setuidgid.c,v $
 * Revision 1.12  2025-01-21 23:35:24+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.11  2024-12-29 09:07:32+05:30  Cprogrammer
 * added -e option for setting USER, LOGNAME and HOME env variables
 *
 * Revision 1.10  2024-12-27 01:02:09+05:30  Cprogrammer
 * Set HOME, USER, LOGNAME env variable
 *
 * Revision 1.9  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.8  2023-02-21 01:06:12+05:30  Cprogrammer
 * re-allocate gidset to actual size
 *
 * Revision 1.7  2023-02-21 00:19:31+05:30  Cprogrammer
 * moved set_additional_groups function to libqmail
 *
 * Revision 1.6  2021-09-11 10:20:02+05:30  Cprogrammer
 * set additional groups using -g option
 *
 * Revision 1.5  2020-09-16 19:06:56+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.4  2020-06-16 23:57:33+05:30  Cprogrammer
 * added option -s to set supplementary groups
 *
 * Revision 1.3  2020-06-16 22:36:48+05:30  Cprogrammer
 * set supplementary group ids
 *
 * Revision 1.2  2004-10-22 20:30:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:37:58+05:30  Cprogrammer
 * Initial revision
 *
 */
