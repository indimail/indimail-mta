/*
 * $Log: vdeloldusers.c,v $
 * Revision 2.18  2016-01-19 00:35:07+05:30  Cprogrammer
 * missing datatype for init_flag
 *
 * Revision 2.17  2011-11-09 19:46:00+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.16  2011-07-07 18:11:36+05:30  Cprogrammer
 * local variable folderlist used outside scope causing segfault
 *
 * Revision 2.15  2008-05-28 17:40:42+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.14  2007-12-22 00:33:29+05:30  Cprogrammer
 * added option to purge user mailboxes
 *
 * Revision 2.13  2005-12-29 22:52:21+05:30  Cprogrammer
 * added Help message for skipping gecos entries
 *
 * Revision 2.12  2005-01-01 00:23:51+05:30  Cprogrammer
 * make trash deletion work when -a, -u, -i are not specified
 *
 * Revision 2.11  2004-09-20 20:17:07+05:30  Cprogrammer
 * BUG - incorrect age comparision
 * made list of gecos to be skipped configurable
 *
 * Revision 2.10  2004-07-02 18:12:14+05:30  Cprogrammer
 * renamed IndiGroup to MailGroup
 *
 * Revision 2.9  2003-11-16 00:03:39+05:30  Cprogrammer
 * allow only one of c, i, p options
 * allow deletion of mails and trash if all users are active
 *
 * Revision 2.8  2003-06-22 10:53:37+05:30  Cprogrammer
 * added option to remove mails only
 *
 * Revision 2.7  2003-05-31 12:11:26+05:30  Cprogrammer
 * added IndiGroup to skipGecos
 *
 * Revision 2.6  2003-03-30 23:38:21+05:30  Cprogrammer
 * added code to skip users with specific gecos (mails3 for powermail)
 *
 * Revision 2.5  2002-08-11 00:31:14+05:30  Cprogrammer
 * added deletion of PID file
 *
 * Revision 2.4  2002-08-03 22:37:40+05:30  Cprogrammer
 * added option for fast deletion by skipping stat() system call
 * added option to specify no of days active for which trash deletion should be done
 *
 * Revision 2.3  2002-07-31 12:59:00+05:30  Cprogrammer
 * added checking timestamp of Maildir/cur as a safety check before deleting.
 *
 * Revision 2.2  2002-07-24 12:19:51+05:30  Cprogrammer
 * mailAge if set as -1 does not delete unread mails
 *
 * Revision 2.1  2002-07-09 14:47:01+05:30  Cprogrammer
 * report_only option was reporting purged count incorrectly
 *
 * Revision 1.18  2002-02-22 17:34:29+05:30  Cprogrammer
 * added report only option
 * added functionality via SIGUSR1 to terminate gracefully.
 *
 * Revision 1.17  2002-01-09 23:09:40+05:30  Cprogrammer
 * option 'V' to display version information
 *
 * Revision 1.16  2001-12-29 11:06:56+05:30  Cprogrammer
 * delete unread mails from both cur and new
 *
 * Revision 1.15  2001-12-24 00:58:05+05:30  Cprogrammer
 * coded added to delete unread mails and trash for active users
 *
 * Revision 1.14  2001-12-13 00:33:09+05:30  Cprogrammer
 * check to prevent simultaneous copies of vdeloldusers to run
 *
 * Revision 1.13  2001-12-08 17:45:40+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.12  2001-12-08 14:44:13+05:30  Cprogrammer
 * code correction where total count was not assigned properly
 *
 * Revision 1.11  2001-11-30 01:07:09+05:30  Cprogrammer
 * verbose switch added
 * code corrections for cases where either indimail or indibak has 0 rows
 *
 * Revision 1.10  2001-11-29 14:35:10+05:30  Cprogrammer
 * added vclose()
 *
 * Revision 1.9  2001-11-28 23:09:27+05:30  Cprogrammer
 * change due to different sort criteria for mysql and strcmp
 *
 * Revision 1.8  2001-11-24 20:27:20+05:30  Cprogrammer
 * added Info messages on user counts
 *
 * Revision 1.7  2001-11-24 12:21:48+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.6  2001-11-24 00:48:20+05:30  Cprogrammer
 * eliminated divide by zero
 *
 * Revision 1.5  2001-11-23 20:59:00+05:30  Cprogrammer
 * used registers to improve speed
 * changed logic in LocateUser to improve speed
 *
 * Revision 1.4  2001-11-23 00:14:37+05:30  Cprogrammer
 * rework on vdelolduser code
 *
 * Revision 1.3  2001-11-20 11:00:23+05:30  Cprogrammer
 * added code for moving inactive users to inactive table
 *
 * Revision 1.2  2001-11-14 19:28:43+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:34+05:30  Cprogrammer
 * Initial revision
 *
 * vdeloldusers
 * remove a user who has not authenticated in a long time.
 * part of the indimail package
 * 
 * Copyright (C) 1999,2001 Inter7 Internet Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdeloldusers.c,v 2.18 2016-01-19 00:35:07+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifndef ENABLE_AUTH_LOGGING
int
main()
{
	printf("auth logging was not enabled, reconfigure with --enable-auth-logging=y\n");
	return (1);
}
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#define DEFAULT_AGE      60
#define DEFAULTMAIL_AGE  30

char            Domain[MAX_BUFF];
char          **skipGecos, **mailboxArr;
int             Age, mailAge, purge_db, report_only, fast_option, TrashAge, shouldexit;
int             c_option, i_option, p_option;

void            usage();
int             get_options(int argc, char **argv);
int             LocateUser(char **, char *, int);
void            SigExit();

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	register char **ptr, **tmp, **lastauthptr, **indimailptr;
	unsigned long   totalcount, activecount, count, purged;
	struct stat     statbuf;
	char            CurDir[MAX_BUFF];
	int             len, isAtty, do_lastauth;
	time_t          tmval, diff;

	if (get_options(argc, argv))
		return (1);
	if (is_already_running("vdeloldusers"))
	{
		fprintf(stderr, "vdeloldusers is already running\n");
		return (1);
	}
	isAtty = (isatty(1) && isatty(2));
	signal(SIGUSR1, SigExit);
	if (verbose)
	{
		tmval = time(0);
		printf("Total  User List %s", ctime(&tmval));
	}
	if (c_option || i_option || p_option)
		indimailptr = getindimail(Domain, 1, skipGecos, &totalcount);
	else /*- For deleting trash ignore skipGecos */
		indimailptr = getindimail(Domain, 1, 0, &totalcount);
	if (totalcount == -1)
	{
		fprintf(stderr, "could not get entries in indimail\n");
		vclose();
		unlink("/tmp/vdeloldusers.PID");
		return (1);
	}
	if (verbose)
		printf("%ld Total Entries\n", totalcount);
	if (!c_option && !i_option && !p_option)
		goto trash_clean;
	if (verbose)
	{
		tmval = time(0);
		printf("Active User List %s", ctime(&tmval));
	}
	if (c_option || i_option || p_option)
		lastauthptr = getlastauth(Domain, Age, 1, 1, &activecount);
	else
		lastauthptr = 0;
	if (activecount == -1)
	{
		fprintf(stderr, "could not get entries in lastauth\n");
		vclose();
		unlink("/tmp/vdeloldusers.PID");
		return (1);
	}
	if (verbose)
		printf("%ld Active Entries\n", activecount);
	if (verbose)
	{
		tmval = time(0);
		printf("Dedup  User List %s", ctime(&tmval));
	}
	if (c_option || i_option || p_option)
	{
		for (purged = count = 0, ptr = lastauthptr; ptr && *ptr; ptr++)
		{
			if (LocateUser(indimailptr, *ptr, 0))
				purged++;
			diff = time(0) - tmval;
			count++;
			if (verbose && isAtty)
			{
				if (diff)
					printf("\r%-7ld %ld", count, count / diff);
				else
					printf("\r%-7ld Inf", count);
			}
		}
		if (verbose && isAtty)
			putchar(10);
	}
	if (totalcount == activecount)	/*- all records are active users */
		purge_db = 0;
	tmval = time(0);
	for (purged = 0, ptr = indimailptr; ptr && *ptr; ptr++)
	{
		if (report_only)
		{
			if (**ptr != ' ')
			{
				if (fast_option)
				{
					purged++;
					if (verbose)
						printf("%s\n", *ptr);
					continue;
				}
				len = slen(*ptr);
				snprintf(CurDir, sizeof(CurDir), "%s/Maildir/cur", *ptr + len + 2);
				if (!stat(CurDir, &statbuf) && ((tmval - statbuf.st_mtime) / 86400 <= Age))
				{
					fprintf(stderr, "user %s@%s Dir %s not old\n", *ptr, Domain, *ptr + len + 2);
					continue;
				}
				purged++;
				if (verbose)
					printf("%s\n", *ptr);
			}
			continue;
		}
		len = slen(*ptr);
		if (**ptr == ' ')
		{
			if (shouldexit)
			{
				vclose();
				unlink("/tmp/vdeloldusers.PID");
				return (0);
			}
			if (mailAge > 0)
			{
				/*- Delete from Maildir/cur */
				Delunreadmails(*ptr + len + 2, 2, mailAge);
				/*- Delete from Maildir/new */
				Delunreadmails(*ptr + len + 2, 1, mailAge);
			}
			**ptr = *(*ptr + len + 1); /*- Restore the first char */
			continue;
		}
		if (!purge_db)
			continue;
		snprintf(CurDir, sizeof(CurDir), "%s/Maildir/cur", *ptr + len + 2);
		if (!fast_option && !stat(CurDir, &statbuf) && ((tmval - statbuf.st_mtime) / 86400 <= Age))
		{
			fprintf(stderr, "user %s@%s Dir %s not old\n", *ptr, Domain, *ptr + len + 2);
			continue;
		}
		if (verbose && !purged)
			printf("Purging Inactive Users %s", ctime(&tmval));
		if (!vdeluser(*ptr, Domain, purge_db))
			purged++;
	}
	if (verbose && !report_only)
		printf("Purged %ld/%ld Inactive Users\n", purged, totalcount);
	if (c_option || i_option || p_option)
	{
		for (ptr = lastauthptr; ptr && *ptr; ptr++)
			free(*ptr);
		free(lastauthptr);
	}
	if (report_only)
	{
		if (verbose)
		{
			tmval = time(0);
			printf("Program Complete %s", ctime(&tmval));
		}
		vclose();
		unlink("/tmp/vdeloldusers.PID");
		return (0);
	}
trash_clean:
	if (mailboxArr)
	{
		tmval = time(0);
		for (tmp = mailboxArr, do_lastauth = 1; *tmp; tmp++)
		{
			if (!strncmp(*tmp, ".Trash", 7))
			{
				/*-
				 * Trash gets filled only if the user logon and delete mails.
				 * Folders like .BulkMail gets filled without the need for
				 * the user to logon. Hence for such folders include inactive
				 * users too.
				 */
				do_lastauth = 0; 
				break;
			}
		}
		if (do_lastauth)
		{
			if (verbose)
				printf("Deleting Trash %s", ctime(&tmval));
			/*- Get Users Active in the Last TrashAge Days */
			lastauthptr = getlastauth(Domain, TrashAge, 1, 1, &activecount);
			for (count = 0, ptr = lastauthptr; ptr && *ptr; ptr++)
				LocateUser(indimailptr, *ptr, !count++);
		} else
		{
			if (verbose)
			{
				printf("Deleting Folders");
				for (tmp = mailboxArr; *tmp; tmp++)
					printf(" %s", *tmp);
				printf("%s", ctime(&tmval));
			}
		}
		for (ptr = indimailptr; ptr && *ptr; ptr++)
		{
			if (shouldexit)
			{
				vclose();
				unlink("/tmp/vdeloldusers.PID");
				return (0);
			}
			if (do_lastauth)
			{
				if (**ptr == ' ')
				{
					for (tmp = mailboxArr; *tmp; tmp++)
						mailboxpurge(*ptr + slen(*ptr) + 2, *tmp, TrashAge, 1);
				}
			} else
			for (tmp = mailboxArr; *tmp; tmp++)
				mailboxpurge(*ptr + slen(*ptr) + 2, *tmp, TrashAge, 1);
		}
	} /*- if (mailboxArr) */
	if (verbose)
	{
		tmval = time(0);
		printf("Program Complete %s", ctime(&tmval));
	}
	vclose();
	unlink("/tmp/vdeloldusers.PID");
	return (0);
}

void
usage()
{
	printf("usage: vdeloldusers [options]\n");
	printf("options: -d domain\n");
	printf("         -a age_in_days (will delete accounts older than this date)\n");
	printf("                        (default is 2 months or 60 days)\n");
	printf("         -u age_in_days (will delete mails    older than this date)\n");
	printf("                        (default is 1 months or 30 days)\n");
	printf("                        (-1 for not to delete mails)\n");
	printf("         -t days        (will clean trash for user active for this number of days)\n");
	printf("                        (default is 1 day)\n");
	printf("                        (-1 for not to delete trash)\n");
	printf("         -m mailboxes   (List of mailboxes to be purged. Multiple -m can be given)\n");
	printf("                        (default value .Trash)\n");
	printf("         -s gecos       (Skip entries having this gecos. Multiple -s can be given)\n");
	printf("         -c Remove Mails only\n");
	printf("         -p Purge entry from Database\n");
	printf("         -i Mark the user as Inactive\n");
	printf("         -r Report only\n");
	printf("         -f Run Fast    (avoids costly stat in Maildir/cur)\n");
	printf("         -v (verbose)\n");
}

int
get_options(int argc, char **argv)
{
	int             c, errflag, len, gecosCount, mailboxCount;
	char           *ptr, *gecosarr, *mailboxarr;
	static int      gecoslen, mailboxlen;
	static char    *(folderlist[]) = { ".Trash", ".BulkMail", 0 };

	memset(Domain, 0, MAX_BUFF);
	gecosCount = gecoslen = mailboxCount = mailboxlen = 0;
	gecosarr = (char *) 0;
	mailboxarr = (char *) 0;
	Age = DEFAULT_AGE;
	TrashAge = 1;
	mailAge = DEFAULTMAIL_AGE;
	purge_db = errflag = 0;
	c_option = i_option = p_option = 0;
	while (!errflag && (c = getopt(argc, argv, "vrpicfd:a:u:t:m:s:")) != -1)
	{
		switch (c)
		{
		case 'd':
			scopy(Domain, optarg, MAX_BUFF);
			break;
		case 's':
			if (!(gecosarr = realloc(gecosarr, (len = slen(optarg)) + 1 + gecoslen)))
			{
				fprintf(stderr, "vdeloldusers: realloc: %s\n", strerror(errno));
				return (1);
			}
			gecosCount++;
			strcpy(gecosarr + gecoslen, optarg);
			gecoslen += (len + 1);
			break;
		case 'a': /*- Inactive Days */
			Age = atoi(optarg);
			break;
		case 'u': /*- Max age of mails */
			mailAge = atoi(optarg);
			break;
		case 't': /*- Max age of mails in Trash or in any folders specified by 'm' option */
			TrashAge = atoi(optarg);
			break;
		case 'm':
			if (!(mailboxarr = realloc(mailboxarr, (len = slen(optarg)) + 2 + mailboxlen)))
			{
				fprintf(stderr, "vdeloldusers: realloc: %s\n", strerror(errno));
				return (1);
			} /*- Trash has to be specified explicitly.. Case sensitive */
			mailboxCount++;
			mailboxarr[mailboxlen] = '.';
			strcpy(mailboxarr + mailboxlen + 1, optarg);
			mailboxlen += (len + 2);
			break;
		case 'r':
			report_only = 1;
		case 'v':
			verbose = 1;
			break;
		case 'f':
			fast_option = 1;
			break;
		case 'c': /*- Remove Mails only */
			c_option = 1;
			if (p_option || i_option)
			{
				fprintf(stderr, "only one of c, i, p option can be specified\n");
				errflag = 1;
			}
			purge_db = 0;
			break;
		case 'p': /*- delete the user from the database */
			p_option = 1;
			if (c_option || i_option)
			{
				fprintf(stderr, "only one of c, i, p option can be specified\n");
				errflag = 1;
			}
			purge_db = 1;
			break;
		case 'i': /*- Delete the home directory and move the user to indibak (inactivate) */
			if (p_option || c_option)
			{
				fprintf(stderr, "only one of c, i, p option can be specified\n");
				errflag = 1;
			}
			i_option = 1;
			purge_db = 2;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (!*Domain || errflag == 1)
	{
		usage();
		return (1);
	}
	if (gecosCount)
	{
		if (!(skipGecos = (char **) malloc(sizeof(char *) * (gecosCount + 2))))
		{
			fprintf(stderr, "vdeloldusers: malloc: %s\n", strerror(errno));
			return (1);
		}
		for (c = 0, ptr = gecosarr; c < gecosCount; c++)
		{
			skipGecos[c] = ptr;
			ptr += slen(ptr) + 1;
		}
		skipGecos[gecosCount] = "MailGroup"; /*- By default this should be skipped */
		skipGecos[gecosCount + 1] = (char *) 0;
	}
	if (mailboxCount)
	{
		if (!(mailboxArr = (char **) malloc(sizeof(char *) * (mailboxCount + 1))))
		{
			fprintf(stderr, "vdeloldusers: malloc: %s\n", strerror(errno));
			return (1);
		}
		for (c = 0, ptr = mailboxarr; c < mailboxCount; c++)
		{
			mailboxArr[c] = ptr;
			ptr += slen(ptr) + 1;
		}
		mailboxArr[mailboxCount] = (char *) 0;
	} else
		mailboxArr = folderlist; /*- Only Trash */
	return (0);
}

int
LocateUser(Table, username, init_flag)
	char          **Table;
	char           *username;
	int             init_flag;
{
	static char   **Table_ptr, **ptr;
	int             ret;

	if (!Table_ptr || init_flag)
		Table_ptr = Table;
	for (ptr = Table_ptr; ptr && *ptr; ptr++)
	{
		ret = strcmp(*ptr, username);
		if (!ret)
		{
			**ptr = ' ';
			Table_ptr = ptr + 1;
			return (1);
		}
	}
	fprintf(stderr, "Warning [%s] not found\n", username);
	return (0);
}

void
SigExit()
{
	shouldexit = 1;
	signal(SIGUSR1, SigExit);
	return;
}
#endif

void
getversion_vdeloldusers_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
