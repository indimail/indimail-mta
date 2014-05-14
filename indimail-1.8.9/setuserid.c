/*
 * $Log: setuserid.c,v $
 * Revision 2.3  2014-05-14 17:34:43+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 2.2  2014-04-17 11:41:18+05:30  Cprogrammer
 * added setuser_privileges() for setting uid, gid and supplementary group ids
 *
 * Revision 2.1  2014-01-30 13:23:55+05:30  Cprogrammer
 * setuid, setgid and supplementary groups
 *
 */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: setuserid.c,v 2.3 2014-05-14 17:34:43+05:30 Cprogrammer Exp mbhangui $";
#endif
/*-
 * scan the group file for all supplementary groups.
 * Return NULL if unsucessfull
 * The returned value should be freed after use
 */
gid_t   *
grpscan(char *user, int *ngroups)
{
	struct passwd  *pwd;
	struct group   *grp;
	long            maxgroups, idx;
	gid_t          *gidsetlen;
	char          **ptr;

	if (!user || !*user)
		return ((gid_t *) 0);
	if ((maxgroups = sysconf(_SC_NGROUPS_MAX)) == -1)
		return ((gid_t *) 0);
	else
	if (!(gidsetlen = (gid_t *) calloc(1, maxgroups * sizeof(gid_t))))
		return ((gid_t *) 0);
	else
	if (!(pwd = getpwnam(user)))
		return ((gid_t *) 0);
	idx = 0;
	gidsetlen[idx++] = pwd->pw_gid;	/* the base gid */
	endpwent();
	for (;;) {
		if (!(grp = getgrent()))
			break;
		for (ptr = grp->gr_mem; *ptr; ptr++) {
			if (!strcmp(user, *ptr) && grp->gr_gid != gidsetlen[0])
				gidsetlen[idx++] = grp->gr_gid;	/* supplementary group ids */
		}
	}
	endgrent();
	*ngroups = idx;
	return (gidsetlen);
}


int
setuserid(user)
	char           *user;
{
	struct passwd  *pwdent;
	gid_t          *gidset;
	int             ngroups;
	uid_t           uid;
	gid_t           gid;

	if (!(pwdent = getpwnam(user)))
		return (-1);
	uid = pwdent->pw_uid;
	gid = pwdent->pw_gid;
	endpwent();
	if (!(gidset = grpscan(user, &ngroups)))
		return (-1);
	if (setgroups(ngroups, gidset)) {
		free(gidset);
		return (-1);
	} else
	if (setgid(gid)) {
		free(gidset);
		return (-1);
	} else
	if (setuid(uid)) {
		free(gidset);
		return (-1);
	}
	free(gidset);
	return (0);
}

int
setuser_privileges(uid, gid, user)
	uid_t           uid;
	gid_t           gid;
	char           *user;
{
	gid_t          *gidset;
	int             ngroups;

	if (!(gidset = grpscan(user, &ngroups)))
		return (-1);
	if (setgroups(ngroups, gidset)) {
		free(gidset);
		return (-1);
	} else
	if (setgid(gid)) {
		free(gidset);
		return (-1);
	} else
	if (setuid(uid)) {
		free(gidset);
		return (-1);
	}
	free(gidset);
	return (0);
}

#include <stdio.h>
void
getversion_setuserid_c()
{
	char *x = sccsid;
	printf("%s\n", x++);
}
