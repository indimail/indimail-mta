/*
 * $Log: qmail-start.c,v $
 * Revision 1.20  2021-04-05 07:19:44+05:30  Cprogrammer
 * added qmail-todo.h
 *
 * Revision 1.19  2020-11-24 13:47:32+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.18  2020-06-16 22:33:09+05:30  Cprogrammer
 * use prot_gid() to lose existing qmail group privileges (qmailr)
 *
 * Revision 1.17  2020-06-16 21:47:30+05:30  Cprogrammer
 * set supplementary group ids if USE_SETGROUPS env variable is set
 *
 * Revision 1.16  2009-12-09 23:57:36+05:30  Cprogrammer
 * additional closeflag argument to uidinit()
 *
 * Revision 1.15  2009-04-30 16:15:06+05:30  Cprogrammer
 * removed hasindimail.h
 *
 * Revision 1.14  2008-06-30 16:10:44+05:30  Cprogrammer
 * removed license code
 *
 * Revision 1.13  2008-06-04 14:00:24+05:30  Cprogrammer
 * compilation failure for non-indimail installation
 *
 * Revision 1.12  2007-12-21 16:03:36+05:30  Cprogrammer
 * conditional compilation of license code
 *
 * Revision 1.11  2004-10-22 20:29:39+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.10  2004-06-03 23:01:47+05:30  Cprogrammer
 * fixed compilation problem without indimail
 *
 * Revision 1.9  2004-05-13 22:53:58+05:30  Cprogrammer
 * removed debug statement left by mistake
 *
 * Revision 1.8  2004-05-12 08:59:35+05:30  Cprogrammer
 * change in checklicense()
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <grp.h>
#include <str.h>
#include "fd.h"
#include "env.h"
#include "alloc.h"
#include "prot.h"
#include "auto_uids.h"
#include "setuserid.h"

char           *(qsargs[]) = { "qmail-send", 0};
char           *(qcargs[]) = { "qmail-clean", 0};
char           *(qlargs[]) = { "qmail-lspawn", "./Mailbox", 0};
char           *(qrargs[]) = { "qmail-rspawn", 0};
#ifdef EXTERNAL_TODO
char           *(qtargs[]) = { "qmail-todo", 0};
#endif

void
die()
{
	_exit(111);
}

/*
 * return 0 if userlist (USE_SETGROUPS env vairable is not set
 * return 1 if userlist is set and == 1
 * else userlist is a list of users seperated by colon ':'
 *   return 1 if user is present in the list
 *   return 0 if user is not present in the list
 */
int
check_user(char *userlist, char *user)
{
	char           *ptr1, *ptr2;

	if (!userlist)
		return 0;
	if (userlist && *userlist == '1')
		return 1;
	for (ptr1 = ptr2 = userlist; *ptr1; ptr1++) {
		if (*ptr1 == ':') {
			*ptr1 = 0;
			if (!str_diff(user, ptr2)) {
				*ptr1 = ':';
				return 1;
			}
			*ptr1 = ':';
			ptr2 = ptr1 + 1;
		}
	}
	if (!str_diff(user, ptr2))
		return 1;
	return 0;
}

int             pi0[2];
int             pi1[2];
int             pi2[2];
int             pi3[2];
int             pi4[2];
int             pi5[2];
int             pi6[2];
#ifdef EXTERNAL_TODO
int             pi7[2];
int             pi8[2];
int             pi9[2];
int             pi10[2];
#endif

void
close23456()
{ 
	close(2);
	close(3);
	close(4);
	close(5);
	close(6); 
#ifdef EXTERNAL_TODO
	close(7);
	close(8);
#endif
}

void
closepipes()
{
	close(pi1[0]);
	close(pi1[1]);
	close(pi2[0]);
	close(pi2[1]);
	close(pi3[0]);
	close(pi3[1]);
	close(pi4[0]);
	close(pi4[1]);
	close(pi5[0]);
	close(pi5[1]);
	close(pi6[0]);
	close(pi6[1]);
#ifdef EXTERNAL_TODO
	close(pi7[0]);
	close(pi7[1]);
	close(pi8[0]);
	close(pi8[1]);
	close(pi9[0]);
	close(pi9[1]);
	close(pi10[0]);
	close(pi10[1]);
#endif
}

int             uidinit(int);

int             verbose;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *set_supplementary_groups;
	gid_t          *gidset;
	int             ngroups;

	set_supplementary_groups = env_get("USE_SETGROUPS");
	if (chdir("/") == -1)
		die();
	if (uidinit(1) == -1)
		die();
	umask(077);
	if (prot_gid(auto_gidq) == -1) /*- qmail unix group */
		die();
	if (fd_copy(2, 0) == -1)
		die();
	if (fd_copy(3, 0) == -1)
		die();
	if (fd_copy(4, 0) == -1)
		die();
	if (fd_copy(5, 0) == -1)
		die();
	if (fd_copy(6, 0) == -1)
		die();
#ifdef EXTERNAL_TODO
	if (fd_copy(7,0) == -1)
		die();
	if (fd_copy(8,0) == -1)
		die();
#endif
	if (argv[1]) {
		qlargs[1] = argv[1];
		++argv;
	}
	if (argv[1]) {
		if (pipe(pi0) == -1)
			die();
		switch (fork())
		{
		case -1:
			die();
		case 0: /* execute logger */
			if (check_user(set_supplementary_groups, "qmaill")) {
				if (!(gidset = grpscan("qmaill", &ngroups)))
					die();
				gidset[0] = auto_gidq;
				gidset[ngroups] = auto_gidn;
				if (prot_gid(auto_gidn) == -1) /*- nofiles unix group */
					die();
				if (setgroups(ngroups + 1, gidset))
					die();
				alloc_free((char *) gidset);
			} else
			if (prot_gid(auto_gidn) == -1) /*- nofiles unix group */
				die();
			if (prot_uid(auto_uidl) == -1) /*- qmaill unix user */
				die();
			close(pi0[1]);
			if (fd_move(0, pi0[0]) == -1)
				die();
			close23456();
			execvp(argv[1], argv + 1); /*- splogger, etc */
			die();
		}
		close(pi0[0]);
		if (fd_move(1, pi0[1]) == -1)
			die();
	}
	if (pipe(pi1) == -1)
		die();
	if (pipe(pi2) == -1)
		die();
	if (pipe(pi3) == -1)
		die();
	if (pipe(pi4) == -1)
		die();
	if (pipe(pi5) == -1)
		die();
	if (pipe(pi6) == -1)
		die();
#ifdef EXTERNAL_TODO
	if (pipe(pi7) == -1)
		die();
	if (pipe(pi8) == -1)
		die();
	if (pipe(pi9) == -1)
		die();
	if (pipe(pi10) == -1)
		die();
#endif
	switch (fork())
	{
	case -1:
		die();
	case 0:
		if (fd_copy(0, pi1[0]) == -1)
			die();
		if (fd_copy(1, pi2[1]) == -1)
			die();
		close23456();
		closepipes();
		execvp(*qlargs, qlargs); /*- qmail-lspawn */
		die();
	}
	switch (fork())
	{
	case -1:
		die();
	case 0:
		if (check_user(set_supplementary_groups, "qmailr")) {
			if (!(gidset = grpscan("qmailr", &ngroups)))
				die();
			gidset[0] = auto_gidq;
			if (setgroups(ngroups, gidset))
				die();
			alloc_free((char *) gidset);
		}
		if (prot_uid(auto_uidr) == -1) /*- qmailr unix user */
			die();
		if (fd_copy(0, pi3[0]) == -1)
			die();
		if (fd_copy(1, pi4[1]) == -1)
			die();
		close23456();
		closepipes();
		execvp(*qrargs, qrargs); /*- qmail-rspawn */
		die();
	}
	switch (fork())
	{
	case -1:
		die();
	case 0:
		if (check_user(set_supplementary_groups, "qmailq")) {
			if (!(gidset = grpscan("qmailq", &ngroups)))
				die();
			gidset[0] = auto_gidq;
			if (setgroups(ngroups, gidset))
				die();
			alloc_free((char *) gidset);
		}
		if (prot_uid(auto_uidq) == -1) /*- qmailq unix user */
			die();
		if (fd_copy(0, pi5[0]) == -1)
			die();
		if (fd_copy(1, pi6[1]) == -1)
			die();
		close23456();
		closepipes();
		execvp(*qcargs, qcargs); /*- qmail-clean */
		die();
	}
#ifdef EXTERNAL_TODO
	switch (fork())
	{
	case -1:
		die();
	case 0:
		if (prot_uid(auto_uids) == -1)
			die();
		if (fd_copy(0,pi7[0]) == -1)
			die();
		if (fd_copy(1,pi8[1]) == -1)
			die();
		close23456();
		if (fd_copy(2,pi9[1]) == -1)
			die();
		if (fd_copy(3,pi10[0]) == -1)
			die();
		closepipes();
		execvp(*qtargs,qtargs); /*- qmail-todo */
		die();
	}

	switch (fork())
	{
	case -1:
		die();
	case 0:
		if (check_user(set_supplementary_groups, "qmailq")) {
			if (!(gidset = grpscan("qmailq", &ngroups)))
				die();
			gidset[0] = auto_gidq;
			if (setgroups(ngroups, gidset))
				die();
			alloc_free((char *) gidset);
		}
		if (prot_uid(auto_uidq) == -1)
			die();
		if (fd_copy(0,pi9[0]) == -1)
			die();
		if (fd_copy(1,pi10[1]) == -1)
			die();
		close23456();
		closepipes();
		execvp(*qcargs,qcargs); /*- qmail-clean */
		die();
	}
#endif
	if (check_user(set_supplementary_groups, "qmails")) {
		if (!(gidset = grpscan("qmails", &ngroups)))
			die();
		gidset[0] = auto_gidq;
		if (setgroups(ngroups, gidset))
			die();
		alloc_free((char *) gidset);
	}
	if (prot_uid(auto_uids) == -1) /*- qmails unix user */
		die();
	if (fd_copy(0, 1) == -1)
		die();
	if (fd_copy(1, pi1[1]) == -1)
		die();
	if (fd_copy(2, pi2[0]) == -1)
		die();
	if (fd_copy(3, pi3[1]) == -1)
		die();
	if (fd_copy(4, pi4[0]) == -1)
		die();
	if (fd_copy(5, pi5[1]) == -1)
		die();
	if (fd_copy(6, pi6[0]) == -1)
		die();
#ifdef EXTERNAL_TODO
	if (fd_copy(7,pi7[1]) == -1)
		die();
	if (fd_copy(8,pi8[0]) == -1)
		die();
#endif
	closepipes();
	execvp(*qsargs, qsargs); /*- qmail-send */
	die();
	/*- Not reached */
	return(0);
}

void
getversion_qmail_start_c()
{
	static char    *x = "$Id: qmail-start.c,v 1.20 2021-04-05 07:19:44+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
