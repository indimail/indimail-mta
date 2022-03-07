/*
 * $Log: qmail-start.c,v $
 * Revision 1.26  2022-03-07 22:38:09+05:30  Cprogrammer
 * fixed qmail-send arguments
 *
 * Revision 1.25  2022-01-30 09:15:36+05:30  Cprogrammer
 * added qscheduler, removed qmail-daemon
 * added compat mode option (support trigger mode in ipc mode)
 * allow configurable big/small todo/intd
 *
 * Revision 1.24  2021-10-20 22:45:12+05:30  Cprogrammer
 * added queue directory as argument for identification in ps list
 *
 * Revision 1.23  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.22  2021-06-27 10:40:10+05:30  Cprogrammer
 * uidnit new argument to disable/enable error on missing uids
 *
 * Revision 1.21  2021-06-24 12:17:21+05:30  Cprogrammer
 * use uidinit function proto from auto_uids.h
 *
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
#include <fd.h>
#include <env.h>
#include <alloc.h>
#include <prot.h>
#include <sgetopt.h>
#include <noreturn.h>
#include <strerr.h>
#include "haslibrt.h"
#include "auto_uids.h"
#include "setuserid.h"

no_return void
die(char *arg)
{
	strerr_die3sys(111, "fatal: qmail-start: ", arg, ": ");
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

static int      pi0[2];
static int      pi1[2];
static int      pi2[2];
static int      pi3[2];
static int      pi4[2];
static int      pi5[2];
static int      pi6[2];
static int      pi7[2];
static int      pi8[2];
static int      pi9[2];
static int      pi10[2];

void
close2345678()
{ 
	close(2);
	close(3);
	close(4);
	close(5);
	close(6); 
	close(7);
	close(8);
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
	close(pi7[0]);
	close(pi7[1]);
	close(pi8[0]);
	close(pi8[1]);
	close(pi9[0]);
	close(pi9[1]);
	close(pi10[0]);
	close(pi10[1]);
}

int             verbose;

int
main(int argc, char **argv)
{
	char           *set_supplementary_groups, *ptr;
	char           *(qsargs[]) = { "qmail-send", 0, (char *) NULL, (char *) NULL};
	char           *(qcargs[]) = { "qmail-clean", 0, 0, (char *) NULL};
	char           *(qlargs[]) = { "qmail-lspawn", "./Mailbox", 0, (char *) NULL};
	char           *(qrargs[]) = { "qmail-rspawn", 0, (char *) NULL};
	char           *(qtargs[]) = { "qmail-todo", 0, 0, 0, (char *) NULL};
	gid_t          *gidset;
	int             ngroups, i = 1, j = 1;
	int             opt;

	set_supplementary_groups = env_get("USE_SETGROUPS");
	if (chdir("/") == -1)
		die("chdir");
	umask(077);
	if (fd_copy(2, 0) == -1)
		die("unable to copy fd2");
	if (fd_copy(3, 0) == -1)
		die("unable to copy fd3");
	if (fd_copy(4, 0) == -1)
		die("unable to copy fd4");
	if (fd_copy(5, 0) == -1)
		die("unable to copy fd5");
	if (fd_copy(6, 0) == -1)
		die("unable to copy fd6");
	if (fd_copy(7,0) == -1)
		die("unable to copy fd7");
	if (fd_copy(8,0) == -1)
		die("unable to copy fd8");
	if (uidinit(1, 1) == -1)
		die("unable to initialize uids/gids");
	if (prot_gid(auto_gidq) == -1) /*- qmail unix group */
		die("unable to set qmail group");
	while ((opt = getopt(argc, argv, "cds")) != opteof) {
		switch (opt)
		{
			case 'c':
				qtargs[i++] = "-c";
				break;
			case 'd':
				qsargs[j++] = "-d";
				qtargs[i++] = "-d";
				break;
			case 's':
				qsargs[j++] = "-s";
				qtargs[i++] = "-s";
				break;
		}
	}
	argc -= optind;
	argv += optind; /*- first arg excluding -d, -s will be argv[0] */
	if ((ptr = env_get("QUEUEDIR"))) { /*- pass the queue as argument for the ps command */
		qsargs[j++] = ptr;
		qcargs[1] = ptr;
		qtargs[i++] = ptr;
		qlargs[2] = ptr;
		qrargs[1] = ptr;
	}
	if (argv[0]) { /*- argument to qmail-lspawn, qmail-local */
		qlargs[1] = argv[0];
		++argv;
	}
	if (argv[0]) {
		if (pipe(pi0) == -1)
			die("unable to move read end of pipe to fd 0");
		switch (fork())
		{
		case -1:
			die("unable to fork logger");
		case 0: /* execute logger */
			if (check_user(set_supplementary_groups, "qmaill")) {
				if ((gidset = grpscan("qmaill", &ngroups))) {
					gidset[0] = auto_gidq;
					gidset[ngroups] = auto_gidn;
					if (prot_gid(auto_gidn) == -1) /*- nofiles unix group */
						die("unable to set nofiles group");
					if (setgroups(ngroups + 1, gidset))
						die("unable to set supplementary group(s) for logger");
					alloc_free((char *) gidset);
				} else
					strerr_warn1("qmail-start: warn: no supplementary groups defined for qmaill (logger)", 0);
			} else
			if (prot_gid(auto_gidn) == -1) /*- nofiles unix group */
				die("unable to set nofiles group");
			if (prot_uid(auto_uidl) == -1) /*- qmaill unix user */
				die("unable to set qmaill group");
			close(pi0[1]);
			if (fd_move(0, pi0[0]) == -1)
				die("unable to move fd0 for logger");
			close2345678();
			execvp(argv[0], argv); /*- splogger, etc */
			die("unable to exec logger");
		}
		close(pi0[0]);
		if (fd_move(1, pi0[1]) == -1)
			die("unable to move write end of pipe to fd 1");
	}
	if (pipe(pi1) == -1)
		die("failed to create pipe1");
	if (pipe(pi2) == -1)
		die("failed to create pipe2");
	if (pipe(pi3) == -1)
		die("failed to create pipe3");
	if (pipe(pi4) == -1)
		die("failed to create pipe4");
	if (pipe(pi5) == -1)
		die("failed to create pipe5");
	if (pipe(pi6) == -1)
		die("failed to create pipe6");
	if (pipe(pi7) == -1)
		die("failed to create pipe7");
	if (pipe(pi8) == -1)
		die("failed to create pipe9");
	if (pipe(pi9) == -1)
		die("failed to create pipe9");
	if (pipe(pi10) == -1)
		die("failed to create pipe10");
	switch (fork())
	{
	case -1:
		die("unable to fork qmail-lspawn");
	case 0:
		if (fd_copy(0, pi1[0]) == -1)
			die("unable to copy fd0 to pipe1");
		if (fd_copy(1, pi2[1]) == -1)
			die("unable to copy fd1 to pipe2");
		close2345678();
		closepipes();
		execvp(*qlargs, qlargs); /*- qmail-lspawn */
		die("unable to exec qmail-lspawn");
	}
	switch (fork())
	{
	case -1:
		die("unable to fork qmail-rspawn");
	case 0:
		if (check_user(set_supplementary_groups, "qmailr")) {
			if ((gidset = grpscan("qmailr", &ngroups))) {
				gidset[0] = auto_gidq;
				if (setgroups(ngroups, gidset))
					die("unable to set supplementary group(s) for qmail-rspawn");
				alloc_free((char *) gidset);
			} else
				strerr_warn1("qmail-start: warn: no supplementary groups defined for qmailr (qmail-rspawn)", 0);
		}
		if (prot_uid(auto_uidr) == -1) /*- qmailr unix user */
			die("unable to set qmailr uid");
		if (fd_copy(0, pi3[0]) == -1)
			die("unable to copy fd0 to pipe3");
		if (fd_copy(1, pi4[1]) == -1)
			die("unable to copy fd1 to pipe4");
		close2345678();
		closepipes();
		execvp(*qrargs, qrargs); /*- qmail-rspawn */
		die("unable to exec qmail-rspawn");
	}
	switch (fork())
	{
	case -1:
		die("unable to fork qmail-clean (qmail-todo)");
	case 0:
		if (check_user(set_supplementary_groups, "qmailq")) {
			if ((gidset = grpscan("qmailq", &ngroups))) {
				gidset[0] = auto_gidq;
				if (setgroups(ngroups, gidset))
					die("unable to set supplementary group(s) for qmail-clean");
				alloc_free((char *) gidset);
			} else
				strerr_warn1("qmail-start: warn: no supplementary groups defined for qmailq (qmail-clean)", 0);
		}
		if (prot_uid(auto_uidq) == -1) /*- qmailq unix user */
			die("unable to set qmailq uid");
		if (fd_copy(0, pi5[0]) == -1)
			die("unable to copy fd0 to pipe5");
		if (fd_copy(1, pi6[1]) == -1)
			die("unable to copy fd1 to pipe6");
		close2345678();
		closepipes();
		qcargs[2] = "qmail-todo"; /*- pass qmail-todo as argument for the ps command */
		execvp(*qcargs, qcargs); /*- qmail-clean */
		die("unable to exec qmail-clean (qmail-todo)");
	}
	switch (fork())
	{
	case -1:
		die("unable to fork qmail-todo");
	case 0:
		if (prot_uid(auto_uids) == -1)
			die("unable to set qmails uid");
		if (fd_copy(0, pi7[0]) == -1)
			die("unable to copy fd0 to pipe7");
		if (fd_copy(1, pi8[1]) == -1)
			die("unable to copy fd1 to pipe8");
		close2345678();
		if (fd_copy(2, pi9[1]) == -1)
			die("unable to copy fd2 to pipe9");
		if (fd_copy(3, pi10[0]) == -1)
			die("unable to copy fd3 to pipe10");
		closepipes();
		execvp(*qtargs, qtargs); /*- qmail-todo */
		die("unable to exec qmail-todo");
	}

	switch (fork())
	{
	case -1:
		die("unable to fork qmail-clean (qmail-send)");
	case 0:
		if (check_user(set_supplementary_groups, "qmailq")) {
			if ((gidset = grpscan("qmailq", &ngroups))) {
				gidset[0] = auto_gidq;
				if (setgroups(ngroups, gidset))
					die("unable to set supplementary group(s) for qmail-clean");
				alloc_free((char *) gidset);
			} else
				strerr_warn1("qmail-start: warn: no supplementary groups defined for qmailq (qmail-clean)", 0);
		}
		if (prot_uid(auto_uidq) == -1)
			die("unable to set qmailq uid");
		if (fd_copy(0, pi9[0]) == -1)
			die("unable to copy fd0 to pipe9");
		if (fd_copy(1, pi10[1]) == -1)
			die("unable to copy fd1 to pipe10");
		close2345678();
		closepipes();
		qcargs[2] = "qmail-send"; /*- pass qmail-send as argument for the ps command */
		execvp(*qcargs, qcargs); /*- qmail-clean */
		die("unable to exec qmail-clean (qmail-send)");
	}
	if (check_user(set_supplementary_groups, "qmails")) {
		if ((gidset = grpscan("qmails", &ngroups))) {
			gidset[0] = auto_gidq;
			if (setgroups(ngroups, gidset))
				die("unable to set supplementary group(s) for qmail-send");
			alloc_free((char *) gidset);
		} else
			strerr_warn1("qmail-start: warn: no supplementary groups defined for qmails", 0);
	}
	if (prot_uid(auto_uids) == -1) /*- qmails unix user */
		die("unable to set qmails uid");
	if (fd_copy(0, 1) == -1)
		die("unable to copy fd0 to fd1");
	if (fd_copy(1, pi1[1]) == -1)
		die("unable to copy fd0 to pipe1");
	if (fd_copy(2, pi2[0]) == -1)
		die("unable to copy fd2 to pipe2");
	if (fd_copy(3, pi3[1]) == -1)
		die("unable to copy fd3 to pipe3");
	if (fd_copy(4, pi4[0]) == -1)
		die("unable to copy fd4 to pipe4");
	if (fd_copy(5, pi5[1]) == -1)
		die("unable to copy fd5 to pipe5");
	if (fd_copy(6, pi6[0]) == -1)
		die("unable to copy fd6 to pipe6");
	if (fd_copy(7, pi7[1]) == -1)
		die("unable to copy fd7 to pipe7");
	if (fd_copy(8, pi8[0]) == -1)
		die("unable to copy fd8 to pipe8");
	closepipes();
	execvp(*qsargs, qsargs); /*- qmail-send */
	die("unable to exec qmail-send");
	/*- Not reached */
	return(0);
}

void
getversion_qmail_start_c()
{
	static char    *x = "$Id: qmail-start.c,v 1.26 2022-03-07 22:38:09+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
