/*
 * $Log: slowq-start.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2022-04-04 00:52:04+05:30  Cprogrammer
 * Use QUEUE_BASE, queue_base control for setting base directory of queue
 *
 * Revision 1.6  2022-01-30 09:42:10+05:30  Cprogrammer
 * made global variables as static
 *
 * Revision 1.5  2021-10-20 22:51:18+05:30  Cprogrammer
 * add queue directory as argument for identification in ps list
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2021-06-27 10:39:22+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.2  2021-06-24 12:17:10+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
 * Revision 1.1  2021-05-31 17:06:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <grp.h>
#include <str.h>
#include <fd.h>
#include <env.h>
#include <alloc.h>
#include <prot.h>
#include <setuserid.h>
#include <noreturn.h>
#include "auto_uids.h"
#include "set_queuedir.h"

no_return void
die()
{
	_exit(111);
}

/*
 * return 0 if userlist (USE_SETGROUPS env variable is not set
 * return 1 if userlist is set and == 1
 * else userlist is a list of users seperated by colon ':'
 *   return 1 if user is present in the list
 *   return 0 if user is not present in the list
 */
int
check_user(char *userlist, const char *user)
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

static int      pi0[2];
static int      pi1[2];
static int      pi2[2];
static int      pi3[2];
static int      pi4[2];
static int      pi5[2];
static int      pi6[2];

void
close23456()
{
	close(2);
	close(3);
	close(4);
	close(5);
	close(6);
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
}

int
main(int argc, char **argv)
{
	char           *set_supplementary_groups;
	const char     *ptr;
	gid_t          *gidset;
	int             ngroups;
	const char     *(qsargs[]) = { "slowq-send", 0, 0};
	const char     *(qcargs[]) = { "qmail-clean", 0, 0, 0};
	const char     *(qlargs[]) = { "qmail-lspawn", "./Mailbox", 0, 0};
	const char     *(qrargs[]) = { "qmail-rspawn", 0, 0};

	set_supplementary_groups = env_get("USE_SETGROUPS");
	if (chdir("/") == -1)
		die();
	if (uidinit(1, 1) == -1)
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
	if (!(ptr = env_get("QUEUEDIR"))) {
		if (!(ptr = set_queuedir("slowq-start", "slowq")))
			die();
		if (!env_put2("QUEUEDIR", ptr))
			die();
	}
	/*- pass the queue as argument for the ps command */
	qsargs[1] = ptr;
	qcargs[1] = ptr;
	qlargs[2] = ptr;
	qrargs[1] = ptr;
	if (argv[1]) { /*- set argument for qmail-local */
		qlargs[1] = argv[1];
		++argv;
	}
	if (argv[1]) { /*- logger (e.g. splogger) specified on command line */
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
		execvp(*qlargs, (char **) qlargs); /*- qmail-lspawn */
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
		execvp(*qrargs, (char **) qrargs); /*- qmail-rspawn */
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
		qcargs[2] = "slowq-send"; /*- pass qmail-send as argument for the ps command */
		execvp(*qcargs, (char **) qcargs); /*- qmail-clean */
		die();
	}
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
	closepipes();
	execvp(*qsargs, (char **) qsargs); /*- slowq-send */
	die();
	/*- Not reached */
	return(0);
}

void
getversion_slowq_start_c()
{
	const char     *x = "$Id: slowq-start.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
