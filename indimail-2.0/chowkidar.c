/*
 * $Log: chowkidar.c,v $
 * Revision 2.13  2017-03-13 13:37:08+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.12  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.11  2010-04-23 11:03:44+05:30  Cprogrammer
 * for -T, -B, -S option, chdir to qmail control directory
 *
 * Revision 2.10  2008-06-13 08:43:36+05:30  Cprogrammer
 * do not compile sync_mode if CLUSTERED_SITE is not defined
 *
 * Revision 2.9  2005-12-29 22:39:45+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.8  2004-09-09 23:55:31+05:30  Cprogrammer
 * added badmailpatterns, badrcptpatters, spamignorepatterns
 *
 * Revision 2.7  2003-12-23 20:03:38+05:30  Cprogrammer
 * spamdb functionality
 *
 * Revision 2.6  2003-12-21 20:55:18+05:30  Cprogrammer
 * added spamdb as a control file
 *
 * Revision 2.5  2003-12-21 14:28:57+05:30  Cprogrammer
 * added option to update spamdb
 *
 * Revision 2.4  2003-02-01 22:51:11+05:30  Cprogrammer
 * added option to for updating badrcptto
 *
 * Revision 2.3  2002-12-29 02:22:08+05:30  Cprogrammer
 * added sync mode of operation
 *
 * Revision 2.2  2002-10-24 01:42:08+05:30  Cprogrammer
 * ignore only badmailfrom and the spam ignore file
 *
 * Revision 2.1  2002-10-19 19:32:51+05:30  Cprogrammer
 * Spam Reporting and Administration Tool
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BADMAIL 1
#define BADRCPT 2
#define SPAMDB  3

#ifndef	lint
static char     sccsid[] = "$Id: chowkidar.c,v 2.13 2017-03-13 13:37:08+05:30 Cprogrammer Exp mbhangui $";
#endif

void            usage();

int
main(int argc, char **argv)
{
	char           *ptr, *revision = "$Revision: 2.13 $";
	int             spamNumber, spamFilter, c, silent, type, relative;
	char            ignfile[SQL_BUF_SIZE], bad_from_rcpt_file[MAX_BUFF];
	char           *filename = (char *) 0, *outfile = (char *) 0;
	char           *sysconfdir, *controldir, *ign;
#ifdef CLUSTERED_SITE
	int             total, sync_mode;
	char          **Ptr, **bmfptr;
#endif

#ifdef CLUSTERED_SITE
	type = sync_mode = silent = 0;
#else
	type = silent = 0;
#endif
	spamNumber = spamFilter = 0;
	filename = (char *) 0;
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
#ifdef CLUSTERED_SITE
	while ((c = getopt(argc, argv, "f:t:b:s:n:o:BTSVrqv")) != -1)
#else
	while ((c = getopt(argc, argv, "f:t:b:s:n:o:BTSVqv")) != -1)
#endif
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'q':
			silent = 1;
			break;
#ifdef CLUSTERED_SITE
		case 'r':
			sync_mode = 1;
			break;
#endif
		case 'V':
			ptr = revision + 11;
			if (*ptr)
			{
				printf("IndiMail Chowkidar Version ");
				for (;*ptr;ptr++)
				{
					if (isspace((int) *ptr))
						break;
					putchar(*ptr);
				}
				putchar('\n');
			}
			return(0);
			break;
		case 'n':
			spamNumber = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		case 'b':
			outfile = optarg;
		case 'B':
			if (!outfile)
			{
				outfile = "badmailfrom";
				if (chdir(sysconfdir) || chdir(controldir))
				{
					fprintf(stderr, "unable to cd to sysconfdir or controldir\n");
					return (1);
				}
			}
			if (type)
			{
				fprintf(stderr, "only one of -b|-B, -t|-T, -s|-S can be selected\n");
				return(1);
			}
			type = BADMAIL;
			break;
		case 't':
			outfile = optarg;
		case 'T':
			if (!outfile)
			{
				outfile = "badrcptto";
				if (chdir(sysconfdir) || chdir(controldir))
				{
					fprintf(stderr, "unable to cd to sysconfdir or controldir\n");
					return (1);
				}
			}
			if (type)
			{
				fprintf(stderr, "only one of -b|-B, -t|-T, -s|-S can be selected\n");
				return(1);
			}
			type = BADRCPT;
			break;
		case 's':
			outfile = optarg;
		case 'S':
			if (!outfile)
			{
				outfile = "spamdb";
				if (chdir(sysconfdir) || chdir(controldir))
				{
					fprintf(stderr, "unable to cd to sysconfdir or controldir\n");
					return (1);
				}
			}
			if (type)
			{
				fprintf(stderr, "only one of -b|-B, -t|-T, -s|-S can be selected\n");
				return(1);
			}
			type = SPAMDB;
			break;
		default:
			spamNumber = -1;
			usage();
			return(1);
		}
	}
#ifdef CLUSTERED_SITE
	if (!type || (!sync_mode && !filename) || !outfile)
#else
	if (!type || !filename || !outfile)
#endif
	{
		usage();
		return(1);
	}
#ifdef CLUSTERED_SITE
	if (sync_mode)
	{
		if (outfile && *outfile)
		{
			if (strchr(outfile, '.') || strchr(outfile, '/'))
			{
				fprintf(stderr, "PATH not allowed in badmailfrom/badrcptto/spamdb file\n");
				return(1);
			}
		}
		if (!(bmfptr = LoadBMF(&total, outfile)))
		{
			fprintf(stderr, "LoadBMF: no entries loaded\n");
			return(1);
		}
		for (Ptr = bmfptr;!silent && Ptr && *Ptr;Ptr++)
			fprintf(stderr, "%s\n", *Ptr);
		return(0);
	} 
#endif
	/*- Update mode */
	if (!readLogFile(filename, type, spamNumber))
	{
		/*
		 * ignore list is our list of priviliged mail users 
		 * firstly we read our ignore file  
		 * ... and from qmail's badmailfrom, since we do not want
		 * to have duplicate spammer addresses 
		 */
		if (strchr(outfile, '.') || strchr(outfile, '/'))
			strncpy(bad_from_rcpt_file, outfile, MAX_BUFF);
		else {
			if (relative)
				snprintf(bad_from_rcpt_file, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, outfile);
			else
				snprintf(bad_from_rcpt_file, MAX_BUFF, "%s/%s", controldir, outfile);
		}
		switch (type)
		{
			case BADMAIL:
				if (relative)
					snprintf(ignfile, MAX_BUFF, "%s/%s/badmailpatterns", sysconfdir, controldir);
				else
					snprintf(ignfile, MAX_BUFF, "%s/badmailpatterns", controldir);
				if (loadIgnoreList(ignfile))
					return(1);
				break;
			case BADRCPT:
				if (relative)
					snprintf(ignfile, MAX_BUFF, "%s/%s/badrcptpatterns", sysconfdir, controldir);
				else
					snprintf(ignfile, MAX_BUFF, "%s/badrcptpatterns", controldir);
				if (loadIgnoreList(ignfile))
					return(1);
				break;
			case SPAMDB:
				if (relative)
					snprintf(ignfile, MAX_BUFF, "%s/%s/spamignorepatterns", sysconfdir, controldir);
				else
					snprintf(ignfile, MAX_BUFF, "%s/spamignorepatterns", controldir);
				if (loadIgnoreList(ignfile))
					return(1);
				break;
		}
		if (relative)
			snprintf(ignfile, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir,
				(ign = getenv("SPAMIGNORE")) ? ign : (ign = "spamignore"));
		else
			snprintf(ignfile, MAX_BUFF, "%s/%s", controldir,
				(ign = getenv("SPAMIGNORE")) ? ign : (ign = "spamignore"));
		if (!loadIgnoreList(ignfile) && !loadIgnoreList(bad_from_rcpt_file))
			return(spamReport(spamNumber, outfile));
	}
	return(1);
}

void
usage()
{
	fprintf(stderr, "Usage: chowkidar [-V] [-r] [-f filename]\n");
	fprintf(stderr, "       [-b badmailfrom_file | -t badrcptto_file | -s spamdb_file]\n");
	fprintf(stderr, "       [-B | -T | -S] [-q] -n count\n");
	fprintf(stderr, "options:  -f multilog logfile\n");
	fprintf(stderr, "          -n Spam Threshold Count\n");
	fprintf(stderr, "          -V display version number\n");
	fprintf(stderr, "          -v set verbose mode\n");
#ifdef CLUSTERED_SITE
	fprintf(stderr, "          -r sync mode operation\n");
#endif
	fprintf(stderr, "          -q silent mode (for sync mode operation)\n");
	fprintf(stderr, "          -b badmailfrom filename (badmailfrom or any other filename)\n");
	fprintf(stderr, "          -B Select badmailfrom mode\n");
	fprintf(stderr, "                                   or\n");
	fprintf(stderr, "          -t badrcptto   filename (badrcptto or any other filename)\n");
	fprintf(stderr, "          -T Select badrcptto mode\n");
	fprintf(stderr, "                                   or\n");
	fprintf(stderr, "          -s spamdb      filename (spamdb or any other filename)\n");
	fprintf(stderr, "          -S Select spamdb mode\n");
}

void
getversion_chowkidar_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
