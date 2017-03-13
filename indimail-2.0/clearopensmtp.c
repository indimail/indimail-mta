/*
 * $Log: clearopensmtp.c,v $
 * Revision 2.13  2017-03-13 13:37:29+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.12  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.11  2011-11-09 19:42:23+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.10  2010-05-05 14:40:51+05:30  Cprogrammer
 * added connect_all argument to vclear_open_smtp
 *
 * Revision 2.9  2008-05-28 21:34:27+05:30  Cprogrammer
 * added option to selectively run clearing of relay table or run update tcp.smtp.cdb
 *
 * Revision 2.8  2008-05-28 16:33:57+05:30  Cprogrammer
 * removed USE_MYSQL, removed cdb code
 *
 * Revision 2.7  2005-12-29 22:40:16+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.6  2004-10-27 14:55:34+05:30  Cprogrammer
 * close mysql before exit
 *
 * Revision 2.5  2002-08-25 22:48:48+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.4  2002-08-11 00:25:05+05:30  Cprogrammer
 * added deletion of PID file
 *
 * Revision 2.3  2002-07-07 22:28:13+05:30  Cprogrammer
 * changed name of variable clear_minutes to clear_seconds
 *
 * Revision 2.2  2002-06-26 03:17:17+05:30  Cprogrammer
 * correction for non-distributed code
 *
 * Revision 2.1  2002-05-13 12:35:42+05:30  Cprogrammer
 * removed argument mytime to vclear_open_smtp()
 *
 * Revision 1.4  2001-12-12 19:24:04+05:30  Cprogrammer
 * added is_alread_running() to prevent multiple instances to run
 *
 * Revision 1.3  2001-11-24 12:17:53+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:39+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: clearopensmtp.c,v 2.13 2017-03-13 13:37:29+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

void            usage();

int
main(int argc, char **argv)
{
	char           *mcdfile, *controldir, *tmpstr;
	char            TmpBuf[MAX_BUFF];
	time_t          clear_seconds;
	int             c, errflag, job_type = 1, cluster_opt = 0;

	errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "ts")) != -1)
	{
		switch (c)
		{
		case 'C':
			cluster_opt = 1;
			break;
		case 't':
			job_type = 2;
			break;
		case 's':
			job_type = 3;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag > 0)
	{
		usage();
		return(1);
	}
	if (is_already_running("clearopensmtp"))
	{
		fprintf(stderr, "clearopensmtp is already running\n");
		return(1);
	}
	if (job_type == 1 || job_type == 3)
	{
		clear_seconds = atoi(RELAY_CLEAR_MINUTES) * 60;
		errflag = vclear_open_smtp(clear_seconds, cluster_opt);
#ifdef CLUSTERED_SITE
		close_db();
#else
		vclose();
#endif
	}
	if (job_type == 1 || job_type == 2)
	{
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
		if (*mcdfile == '/')
			scopy(TmpBuf, mcdfile, MAX_BUFF);
		else {
			if (*controldir == '/')
				snprintf(TmpBuf, MAX_BUFF, "%s/%s", controldir, mcdfile);
			else {
				getEnvConfigStr(&tmpstr, "SYSCONFDIR", SYSCONFDIR);
				snprintf(TmpBuf, MAX_BUFF, "%s/%s/%s", tmpstr, controldir, mcdfile);
			}
		}
		if (access(TmpBuf, F_OK))
		{
			if (job_type == 2 && vauth_open((char *) 0))
				return(1);
			errflag = update_rules(1);
			vclose();
		}
		unlink("/tmp/clearopensmtp.PID");
	}
	return (errflag);
}

void
usage()
{
	char           *relay_table;

	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	printf("usage: clearopensmtp [options]\n");
	printf("options: -V (print version number)\n");
	printf("         -t (run updaterules)\n");
	printf("         -s (clear MySQL RELAY TABLE (%s))\n", relay_table);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-roaming-users=y\n");
	return(0);
}
#endif

void
getversion_clearopensmtp_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
