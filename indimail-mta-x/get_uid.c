/*
 * $Log: get_uid.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-02-14 07:49:06+05:30  Cprogrammer
 * added qcerts group ID for certificate group permission
 * renamed auto_uidc, auto_gidc to auto_uidv, auto_gidv
 * renamed auto_uidv, auto_gidv to auto_uidi, auto_gidi
 * added auto_gidc for qcerts group ID
 *
 * Revision 1.4  2022-03-09 13:03:12+05:30  Cprogrammer
 * use qendpwent(), qendgrent() when using functions from qgetpwgr.c
 *
 * Revision 1.3  2021-07-10 00:28:46+05:30  Cprogrammer
 * fixed wrong uid/gid allocation
 *
 * Revision 1.2  2021-07-05 19:10:20+05:30  Cprogrammer
 * complete rewrite
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
#include <env.h>
#include <qgetpwgr.h>
#include "auto_uids.h"

#define DO_UID(v, w, x, y, z) v = ((w = get_uid(x, y)) == -2) ? -1 : w; \
		if (w == -2) z = 1;
#define DO_GID(v, w, x, y, z) v = ((w = get_gid(x, y)) == -2) ? -1 : w; \
		if (w == -2) z = 1;
#define GET_USER(x, y, z) if (x == y) return z;
#define GET_GROUP(x, y, z) if (x == y) return z;

static char     strnum[FMT_ULONG];
static int      use_pwgr;

/*-
 * When you add an uid, gid edit Makefile for
 * targets auto_uids.h, auto_uids.c
 */
typedef struct uidarray
{
	const char     *user;
	uid_t           uid;
	int             len;
} UIDARRAY;

typedef struct gidarray
{
	const char     *group;
	gid_t           gid;
	int             len;
} GIDARRAY;

UIDARRAY        uid_a[] = {{ALIASU,-1,-1}, {QMAILD,-1,-1}, {QMAILL,-1,-1},
					{ROOTUSER,0,-1}, {QMAILP,-1,-1}, {QMAILQ,-1,-1},
					{QMAILR,-1,-1}, {QMAILS,-1,-1}, {INDIUSER,-1,-1},
					{QSCANDU,-1,-1}, {0}};
GIDARRAY        gid_a[] = {{QMAILG,-1,-1}, {NOFILESG,-1,-1},
					{INDIGROUP,-1,-1}, {QSCANDG,-1,-1}, {QCERTSG, -1, -1}, {0}};

static int
get_uid(const char *user, int exit_on_error)
{
	struct passwd  *pw;
	int             i, len;
	uid_t           found = -1;
	static int      first;

	if (!first) {
		use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
		for (;;) {
			if (!(pw = (use_pwgr ? qgetpwent : getpwent) ()))
				break;
			for (i = 0; uid_a[i].user; i++) {
				len = str_len(uid_a[i].user);
				if (!str_diffn(uid_a[i].user, pw->pw_name, len)) {
					uid_a[i].uid = pw->pw_uid;
					uid_a[i].len = len;
				}
				if (!str_diffn(uid_a[i].user, user, len))
					found = uid_a[i].uid;
			}
		}
		if (exit_on_error) {
			for (i = 0; uid_a[i].user; i++) {
				if (uid_a[i].uid == -1)
					strerr_die3sys(111, "qgetpwent: ", uid_a[i].user, ": No such user: ");
			}
		}
		first++;
		return (found == -1 ? -2 : found);
	}
	for (i = 0; uid_a[i].user; i++) {
		len = str_len(user);
		len = len > uid_a[i].len ? len : uid_a[i].len;
		if (!str_diffn(uid_a[i].user, user, len))
			return (uid_a[i].uid);
	}
	return (-2); /*- not found */
}

static int
get_gid(const char *group, int exit_on_error)
{
	struct group   *gr;
	int             i, len;
	gid_t           found = -1;
	static int      first;

	if (!first) {
		use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
		for (;;) {
			if (!(gr = (use_pwgr ? qgetgrent : getgrent) ()))
				break;
			for (i = 0; gid_a[i].group; i++) {
				len = str_len(gid_a[i].group);
				if (!str_diffn(gid_a[i].group, gr->gr_name, len)) {
					gid_a[i].gid = gr->gr_gid;
					gid_a[i].len = len;
				}
				if (!str_diffn(gid_a[i].group, group, len))
					found = gid_a[i].gid;
			}
		}
		if (exit_on_error) {
			for (i = 0; gid_a[i].group; i++) {
				if (gid_a[i].gid == -1)
					strerr_die3sys(111, "qgetgrent: ", gid_a[i].group, ": No such group: ");
			}
		}
		first++;
		return (found == -1 ? -2 : found);
	}
	for (i = 0; gid_a[i].group; i++) {
		len = str_len(group);
		len = len > gid_a[i].len ? len : gid_a[i].len;
		if (!str_diffn(gid_a[i].group, group, len))
			return (gid_a[i].gid);
	}
	return (-2);
}

int
uidinit(int closeflag, int exit_on_error)
{
	static int      first;
	int             not_found = 0, i;
	uid_t           u;
	gid_t           g;

	if (first)
		return(0);
	use_pwgr = env_get("USE_QPWGR") ? 1 : 0;
	DO_UID(auto_uida, u, ALIASU, exit_on_error, not_found);
	DO_UID(auto_uidd, u, QMAILD, exit_on_error, not_found);
	DO_UID(auto_uidl, u, QMAILL, exit_on_error, not_found);
	DO_UID(auto_uido, u, ROOTUSER, exit_on_error, not_found);
	DO_UID(auto_uidp, u, QMAILP, exit_on_error, not_found);
	DO_UID(auto_uidq, u, QMAILQ, exit_on_error, not_found);
	DO_UID(auto_uidr, u, QMAILR, exit_on_error, not_found);
	DO_UID(auto_uids, u, QMAILS, exit_on_error, not_found);
	DO_UID(auto_uidi, u, INDIUSER, exit_on_error, not_found);
	DO_UID(auto_uidv, u, QSCANDU, exit_on_error, not_found);
	DO_GID(auto_gidq, g, QMAILG, exit_on_error, not_found);
	DO_GID(auto_gidn, g, NOFILESG, exit_on_error, not_found);
	DO_GID(auto_gidi, g, INDIGROUP, exit_on_error, not_found);
	DO_GID(auto_gidv, g, QSCANDG, exit_on_error, not_found);
	DO_GID(auto_gidc, g, QCERTSG, exit_on_error, not_found);
	if (closeflag) {
		use_pwgr ? qendpwent() : endpwent();
		use_pwgr ? qendgrent() : endgrent();
	}
	if (not_found)
		return -1;
	if (exit_on_error) {
		for (i = 0; uid_a[i].user; i++) {
			if (uid_a[i].uid == -1)
				strerr_die3sys(111, "qgetpwent: ", uid_a[i].user, ": No such user: ");
		}
		for (i = 0; gid_a[i].group; i++) {
			if (gid_a[i].gid == -1)
				strerr_die3sys(111, "qgetgrent: ", gid_a[i].group, ": No such group: ");
		}
	}
	first++;
	return (0);
}

const char *
get_user(uid_t uid)
{
	struct passwd  *pw;

	if (uidinit(0, 0) == -1)
		return ((char *) 0);
	GET_USER(uid, auto_uida, ALIASU);
	GET_USER(uid, auto_uidd, QMAILD);
	GET_USER(uid, auto_uidl, QMAILL);
	GET_USER(uid, auto_uido, ROOTUSER);
	GET_USER(uid, auto_uidp, QMAILP);
	GET_USER(uid, auto_uidq, QMAILQ);
	GET_USER(uid, auto_uidr, QMAILR);
	GET_USER(uid, auto_uids, QMAILS);
	GET_USER(uid, auto_uidi, INDIUSER);
	GET_USER(uid, auto_uidv, QSCANDU);
	if (!(pw = (use_pwgr ? qgetpwuid : getpwuid) (uid))) {
		strnum[fmt_ulong(strnum, uid)] = 0;
		strerr_die3sys(111, "get_user: unable to get uid for uid ", strnum, ": ");
	}
	return(pw->pw_name);
}

const char *
get_group(gid_t gid)
{
	struct group   *gr;

	if (uidinit(0, 0) == -1)
		return ((char *) 0);
	GET_GROUP(gid, auto_gidq, QMAILG);
	GET_GROUP(gid, auto_gidn, NOFILESG);
	GET_GROUP(gid, auto_gidi, INDIGROUP);
	GET_GROUP(gid, auto_gidv, QSCANDG);
	GET_GROUP(gid, auto_gidc, QCERTSG);
	GET_GROUP(gid, 0, "root");
	if (!(gr = (use_pwgr ? qgetgrgid : getgrgid) (gid))) {
		strnum[fmt_ulong(strnum, gid)] = 0;
		strerr_die3sys(111, "get_user: unable to get gid for gid ", strnum, ": ");
	}
	return (gr->gr_name);
}

void
getversion_get_uid_c()
{
	const char     *x = "$Id: get_uid.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
