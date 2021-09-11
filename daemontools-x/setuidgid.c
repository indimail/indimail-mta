/*
 * $Log: setuidgid.c,v $
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
#include <pathexec.h>
#include <alloc.h>
#include <scan.h>

#define FATAL "setuidgid: fatal: "

void
set_additional_groups(char *groups)
{
	struct group   *gr;
	char           *ptr, *cptr;
	int             ngroups, i = 0;
	gid_t          *gidset;

	for (ptr = groups, ngroups = 0; *ptr; ptr++) {
		if (*ptr == ',')
			ngroups++;
	}
	ngroups++;
	if (!(gidset = (gid_t *) alloc(ngroups * sizeof(gid_t))))
		strerr_die2x(111, FATAL, "out of memory");
	for (ptr = cptr = groups; *ptr; ptr++) {
		if (*ptr == ',') {
			*ptr = 0;
			if (!(gr = getgrnam(cptr)))
				strerr_die3x(111, FATAL, "unknown group: ", cptr);
			gidset[i++] = gr->gr_gid;
			*ptr = ',';
			cptr = ptr + 1;
		}
	}
	if (!(gr = getgrnam(cptr)))
		strerr_die3x(111, FATAL, "unknown group: ", cptr);
	gidset[i++] = gr->gr_gid;
	if (setgroups(ngroups, gidset))
		strerr_die2sys(111, FATAL, "unable to setgroups: ");
	return;
}

int
main(int argc, char **argv, char **envp)
{
	struct passwd  *pw;
	gid_t          *gidset;
	char           *account, *groups = 0, *usage = "usage: setuidgid [-s] account child";
	char          **child;
	int             ngroups = 0, opt;

	while ((opt = getopt(argc, argv, "sg:")) != opteof) {
		switch (opt)
		{
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
		strerr_die2sys(111, FATAL, "unable to setgid: ");
	if (ngroups) {
		if (!(gidset = grpscan(account, &ngroups)))
			strerr_die2sys(111, FATAL, "unable to do groupscan: ");
		if (setgroups(ngroups, gidset))
			strerr_die2sys(111, FATAL, "unable to setgroups: ");
	}
	if (groups)
		set_additional_groups(groups);
	if (prot_uid(pw->pw_uid) == -1)
		strerr_die2sys(111, FATAL, "unable to setuid: ");
	pathexec_run(*child, child, envp);
	strerr_die4sys(111, FATAL, "unable to run ", *child, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_setuidgid_c()
{
	static char    *x = "$Id: setuidgid.c,v 1.6 2021-09-11 10:20:02+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
