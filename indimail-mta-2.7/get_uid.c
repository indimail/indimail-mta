/*
 * $Log: get_uid.c,v $
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
#include "auto_uids.h"
#include "strerr.h"
#include "str.h"
#include "fmt.h"

static int      get_uid(char *);
static int      get_gid(char *);

static char     strnum[FMT_ULONG];

/*-
 * When you add an uid, increase length of uid_array[]
 * change Makefile targets auto_uids.h and auto_uids.c
 */
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
static char    *user_gid_list[] = {
	QMAILG,
	NOFILESG,
	INDIGROUP,
	QSCANDG,
	""
};

static int
get_uid(char *user)
{
	struct passwd  *pw;
	int             i, len, len1, len2;
	static int      first;
	static uid_t    uid_array[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	if (!first) {
		for (;;) {
			if (!(pw = getpwent()))
				break;
			for (i = 0; *user_uid_list[i]; i++) {
				len = str_len(user_uid_list[i]);
				if (!str_diffn(user_uid_list[i], pw->pw_name, len))
					uid_array[i] = pw->pw_uid;
			}
		}
		for (i = 0; *user_uid_list[i]; i++) {
			if (uid_array[i] == -1)
				strerr_die3sys(111, "getpwent: ", user_uid_list[i], ": No such user: ");
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
	return (-1);
}

static int
get_gid(char *group)
{
	struct group   *gr;
	int             i, len, len1, len2;
	static int      first;
	static uid_t    gid_array[5] = {-1, -1, -1, -1, -1};

	if (!first) {
		for (;;) {
			if (!(gr = getgrent()))
				break;
			for (i = 0; *user_gid_list[i]; i++) {
				len = str_len(user_gid_list[i]);
				if (!str_diffn(user_gid_list[i], gr->gr_name, len))
					gid_array[i] = gr->gr_gid;
			}
		}
		for (i = 0; *user_gid_list[i]; i++) {
			if (gid_array[i] == -1)
				strerr_die3sys(111, "getgrent: ", user_gid_list[i], ": No such group: ");
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
	return (-1);
}

int
uidinit(int closeflag)
{
	static int      first;

	if (first)
		return(0);
	if ((auto_uida = get_uid(ALIASU)) == -1)
		return (-1);
	else
	if ((auto_uidd = get_uid(QMAILD)) == -1)
		return (-1);
	else
	if ((auto_uidl = get_uid(QMAILL)) == -1)
		return (-1);
	else
	if ((auto_uido = get_uid(ROOTUSER)) == -1)
		return (-1);
	else
	if ((auto_uidp = get_uid(QMAILP)) == -1)
		return (-1);
	else
	if ((auto_uidq = get_uid(QMAILQ)) == -1)
		return (-1);
	else
	if ((auto_uidr = get_uid(QMAILR)) == -1)
		return (-1);
	else
	if ((auto_uids = get_uid(QMAILS)) == -1)
		return (-1);
	else
	if ((auto_uidv = get_uid(INDIUSER)) == -1)
		return (-1);
	else
	if ((auto_uidc = get_uid(QSCANDU)) == -1)
		return (-1);
	else
	if ((auto_gidq = get_gid(QMAILG)) == -1)
		return (-1);
	else
	if ((auto_gidn = get_gid(NOFILESG)) == -1)
		return (-1);
	else
	if ((auto_gidv = get_gid(INDIGROUP)) == -1)
		return (-1);
	else
	if ((auto_gidc = get_gid(QSCANDG)) == -1)
		return (-1);
	else {
		first++;
		if (closeflag) {
			endpwent();
			endgrent();
		}
		return (0);
	}
}

char *
get_user(uid_t uid)
{
	struct passwd  *pw;

	if (uidinit(0) == -1)
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
		if (!(pw = getpwuid(uid))) {
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

	if (uidinit(0) == -1)
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
		if (!(gr = getgrgid(gid))) {
			strnum[fmt_ulong(strnum, gid)] = 0;
			strerr_die3sys(111, "get_user: unable to get gid for gid ", strnum, ": ");
		}
		return("nobody");
	}
}

void
getversion_get_uid_c()
{
	static char    *x = "$Id: get_uid.c,v 1.16 2019-07-18 10:49:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
