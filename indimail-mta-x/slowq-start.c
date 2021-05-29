/*
 * $Log: $
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

char           *(qsargs[]) = { "slowq-send", 0};
char           *(qcargs[]) = { "qmail-clean", 0};
char           *(qlargs[]) = { "qmail-lspawn", "./Mailbox", 0};
char           *(qrargs[]) = { "qmail-rspawn", 0};

void
die()
{
	_exit(111);
}

int             pi0[2];
int             pi1[2];
int             pi2[2];
int             pi3[2];
int             pi4[2];
int             pi5[2];
int             pi6[2];

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

int             uidinit(int);

int             verbose;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
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
		execvp(*qlargs, qlargs); /*- qmail-lspawn */
		die();
	}
	switch (fork())
	{
	case -1:
		die();
	case 0:
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
	execvp(*qsargs, qsargs); /*- slowq-send */
	die();
	/*- Not reached */
	return(0);
}

void
getversion_slowq_start_c()
{
	static char    *x = "$Id: $";

	x++;
}
