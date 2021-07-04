/*
 * $Log: get_uid.c,v $
 * Revision 1.18  2021-07-04 14:38:41+05:30  Cprogrammer
 * use qgetpwent, qgetgrent functions from qgetpwgr.c of libqmail
 *
 * Revision 1.17  2021-06-27 10:36:34+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.16  2019-07-18 10:49:03+05:30  Cprogrammer
 * use strerr_die?x macro instead of strerr_die() function
 *
 * Revision 1.15  2019-07-13 11:05:18+05:30  Cprogrammer
 * fixed indentation style
 *
 * Revision 1.14  2019-07-13 10:21:18+05:30  Cprogrammer
 * return actual user/group from passwd/group file
 *
 * Revision 1.13  2009-12-09 23:56:24+05:30  Cprogrammer
 * close passwd, group database if passed additional flag - closeflag
 *
 * Revision 1.12  2009-02-05 15:34:30+05:30  Cprogrammer
 * removed rootgroup
 *
 * Revision 1.11  2009-02-01 00:06:35+05:30  Cprogrammer
 * added get_user, get_group and root gid
 *
 * Revision 1.10  2008-06-10 16:00:03+05:30  Cprogrammer
 * made all uid, gid configurable from conf-users, conf-groups
 *
 * Revision 1.9  2008-05-26 22:21:27+05:30  Cprogrammer
 * added documentation for uid_array, gid_array
 *
 * Revision 1.8  2005-04-26 23:04:56+05:30  Cprogrammer
 * optimized reading of passwd and group files
 *
 * Revision 1.7  2004-10-22 20:25:39+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-09-26 00:03:07+05:30  Cprogrammer
 * removed sserr
 *
 * Revision 1.5  2004-09-26 00:00:30+05:30  Cprogrammer
 * removed stdio
 *
 * Revision 1.4  2004-09-23 22:55:05+05:30  Cprogrammer
 * use auto_uids.h
 *
 * Revision 1.3  2003-07-30 19:06:58+05:30  Cprogrammer
 * changed default user to indimail
 *
 * Revision 1.2  2002-11-29 19:43:22+05:30  Cprogrammer
 * added debugging statements
 *
 * Revision 1.1  2002-09-28 00:11:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <qgetpwgr.h>
#include "auto_uids.h"

static char     strnum[FMT_ULONG];

/*-
 * When you add an uid, increase length of uid_array[]
 * change Makefile targets auto_uids.h and auto_uids.c
 */
static uid_t    uid_array[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static char    *user_uid_list[] = {
	ALIASU,
	QMAILD,
	QMAILL,
	ROOTUSER,
	QMAILP,
	QMAILQ,
	QMAILR,
	QMAILS,
	INDIUSER,
	QSCANDU,
	""
};

/*-
 * When you add a gid, increase length of gid_array[]
 * change Makefile targets auto_uids.h and auto_uids.c
 */
static gid_t    gid_array[5] = {-1, -1, -1, -1, -1};
static char    *user_gid_list[] = {
	QMAILG,
	NOFILESG,
	INDIGROUP,
	QSCANDG,
	""
};

static int
get_uid(char *user, int all)
{
	struct passwd  *pw;
	int             i, len, len1, len2;
	static int      first;

	if (!first) {
		for (;;) {
			if (!(pw = qgetpwent()))
				break;
			for (i = 0; *user_uid_list[i]; i++) {
				len = str_len(user_uid_list[i]);
				if (!str_diffn(user_uid_list[i], pw->pw_name, len))
					uid_array[i] = pw->pw_uid;
			}
		}
		if (all) {
			for (i = 0; *user_uid_list[i]; i++) {
				if (uid_array[i] == -1)
					strerr_die3sys(111, "qgetpwent: ", user_uid_list[i], ": No such user: ");
			}
		}
		first++;
	}
	for (i = 0; *user_uid_list[i]; i++) {
		len1 = str_len(user_uid_list[i]);
		len2 = str_len(user);
		len = len2 > len1 ? len2 : len1;
		if (!str_diffn(user_uid_list[i], user, len))
			return (uid_array[i]);
	}
	return (-2); /*- not found */
}

static int
get_gid(char *group, int all)
{
	struct group   *gr;
	int             i, len, len1, len2;
	static int      first;

	if (!first) {
		for (;;) {
			if (!(gr = qgetgrent()))
				break;
			for (i = 0; *user_gid_list[i]; i++) {
				len = str_len(user_gid_list[i]);
				if (!str_diffn(user_gid_list[i], gr->gr_name, len))
					gid_array[i] = gr->gr_gid;
			}
		}
		if (all) {
			for (i = 0; *user_gid_list[i]; i++) {
				if (gid_array[i] == -1)
					strerr_die3sys(111, "qgetgrent: ", user_gid_list[i], ": No such group: ");
			}
		}
		first++;
	}
	for (i = 0; *user_gid_list[i]; i++) {
		len1 = str_len(user_gid_list[i]);
		len2 = str_len(group);
		len = len2 > len1 ? len2 : len1;
		if (!str_diffn(user_gid_list[i], group, len))
			return (gid_array[i]);
	}
	return (-2);
}

int
uidinit(int closeflag, int all)
{
	static int      first;
	int             err = 0, i;
	uid_t           u;
	gid_t           g;

	if (first)
		return(0);
	auto_uida = ((u = get_uid(ALIASU, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidd = ((u = get_uid(QMAILD, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidl = ((u = get_uid(QMAILL, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uido = ((u = get_uid(ROOTUSER, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidp = ((u = get_uid(QMAILP, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidq = ((u = get_uid(QMAILQ, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidr = ((u = get_uid(QMAILR, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uids = ((u = get_uid(QMAILS, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidv = ((u = get_uid(INDIUSER, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_uidc = ((u = get_uid(QSCANDU, all)) == -2) ? -1 : u;
	if (u == -2)
		err = 1;
	auto_gidq = ((g = get_gid(QMAILG, all)) == -2) ? -1 : g;
	if (g == -2)
		err = 1;
	auto_gidn = ((g = get_gid(NOFILESG, all)) == -2) ? -1 : g;
	if (g == -2)
		err = 1;
	auto_gidv = ((g = get_gid(INDIGROUP, all)) == -2) ? -1 : g;
	if (g == -2)
		err = 1;
	auto_gidc = ((g = get_gid(QSCANDG, all)) == -2) ? -1 : g;
	if (g == -2)
		err = 1;
	if (closeflag) {
		endpwent();
		endgrent();
	}
	if (err)
		return -1;
	if (all) {
		for (i = 0; *user_uid_list[i]; i++) {
			if (uid_array[i] == -1)
				strerr_die3sys(111, "qgetpwent: ", user_uid_list[i], ": No such user: ");
		}
		for (i = 0; *user_gid_list[i]; i++) {
			if (gid_array[i] == -1)
				strerr_die3sys(111, "qgetgrent: ", user_gid_list[i], ": No such group: ");
		}
	}
	first++;
	return (0);
}

char *
get_user(uid_t uid)
{
	struct passwd  *pw;

	if (uidinit(0, 0) == -1)
		return ((char *) 0);
	else
	if (uid == auto_uida)
		return("alias");
	else
	if (uid == auto_uidd)
		return("qmaild");
	else
	if (uid == auto_uidl)
		return("qmaill");
	else
	if (uid == auto_uido)
		return("root");
	else
	if (uid == auto_uidp)
		return("qmailp");
	else
	if (uid == auto_uidq)
		return("qmailq");
	else
	if (uid == auto_uidr)
		return("qmailr");
	else
	if (uid == auto_uids)
		return("qmails");
	else
	if (uid == auto_uidv)
		return("indimail");
	else
	if (uid == auto_uidc)
		return("qscand");
	else {
		if (!(pw = qgetpwuid(uid))) {
			strnum[fmt_ulong(strnum, uid)] = 0;
			strerr_die3sys(111, "get_user: unable to get uid for uid ", strnum, ": ");
		}
		return(pw->pw_name);
	}
}

char *
get_group(gid_t gid)
{
	struct group   *gr;

	if (uidinit(0, 0) == -1)
		return ((char *) 0);
	if (gid == auto_gidq)
		return("qmail");
	else
	if (gid == auto_gidn)
		return("nofiles");
	else
	if (gid == auto_gidv)
		return("indimail");
	else
	if (gid == auto_gidc)
		return("qscand");
	else
	if (gid == 0)
		return("root");
	else {
		if (!(gr = qgetgrgid(gid))) {
			strnum[fmt_ulong(strnum, gid)] = 0;
			strerr_die3sys(111, "get_user: unable to get gid for gid ", strnum, ": ");
		}
		return (gr->gr_name);
	}
}

void
getversion_get_uid_c()
{
	static char    *x = "$Id: get_uid.c,v 1.18 2021-07-04 14:38:41+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
