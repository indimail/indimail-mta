/*
 * $Log: user.c,v $
 * Revision 1.2  2014-04-17 11:47:03+05:30  Cprogrammer
 * set supplementary group ids in set_user()
 *
 * Revision 1.1  2013-05-15 00:35:16+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "user.h"
#include "log.h"

uid_t
get_user_id(char *username)
{

	struct passwd  *pwd;

	pwd = getpwnam(username);
	if (!pwd) {
		perror(username);
		debug("Sorry, could not resolve username %s", username);
		exit(1);
	}
	return pwd->pw_uid;
}

gid_t
get_guser_id(char *username)
{

	struct passwd  *pwd;

	pwd = getpwnam(username);
	if (!pwd) {
		perror(username);
		debug("Sorry, could not resolve username %s", username);
		exit(1);
	}
	return pwd->pw_gid;
}

/*-
 * scan the group file for all supplementary groups.
 * Return NULL if unsucessfull
 * The returned value should be freed after use
 */
static gid_t   *
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


void
set_user(uid_t uid, gid_t gid, char *user)
{

	uid_t           cur_uid;
	gid_t          *gidset;
	int             ngroups;

	cur_uid = getuid();
	if ((cur_uid != 0) && (cur_uid != uid)) {
		debug("sorry, only root can change uid to another user");
		exit(1);
	}
	if (!(gidset = grpscan(user, &ngroups))) {
		exit(1);
	}
	if (setgroups(ngroups, gidset)) {
		perror("setgroups");
		debug("setgroups %s", user);
		free(gidset);
		exit(1);
	}
	if ((setgid(gid) != 0) || (setuid(uid) != 0) || (seteuid(uid) != 0) || (setegid(gid) != 0)) {
		perror("uid");
		debug("cannot setuid to %d:%d", uid, gid);
		free(gidset);
		exit(1);
	}

	return;
}
