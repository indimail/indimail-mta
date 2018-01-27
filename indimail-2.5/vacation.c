/*
 * $Log: vacation.c,v $
 * Revision 2.13  2017-03-13 14:10:51+05:30  Cprogrammer
 * use PREFIX for bin prefix
 *
 * Revision 2.12  2011-12-04 21:04:14+05:30  Cprogrammer
 * added option to specify charset
 *
 * Revision 2.11  2011-07-29 09:26:29+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.10  2010-03-07 14:44:28+05:30  Cprogrammer
 * changed welcome message
 *
 * Revision 2.9  2009-02-18 09:08:09+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.8  2008-07-13 19:49:06+05:30  Cprogrammer
 * removed compilation warning on FC 9
 *
 * Revision 2.7  2008-05-28 15:18:33+05:30  Cprogrammer
 * removed CDB
 *
 * Revision 2.6  2005-12-29 22:50:56+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.5  2004-07-03 23:53:31+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.4  2003-07-06 15:28:21+05:30  Cprogrammer
 * added check for sender
 *
 * Revision 2.3  2003-06-23 22:52:02+05:30  Cprogrammer
 * added correct return path
 * added help message
 *
 * Revision 2.2  2002-05-12 01:22:41+05:30  Cprogrammer
 * moved code to add auto responder as a separate function add_vacation()
 *
 * Revision 2.1  2002-05-11 15:55:42+05:30  Cprogrammer
 * added option to create vacation
 *
 * Revision 1.4  2001-11-24 12:20:21+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:17+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:25:06+05:30  Cprogrammer
 * use mtime instead of ctime (as backup and restore will restore the correct mtime
 *
 * Revision 1.1  2001-10-24 18:15:16+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>
#include <sys/param.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: vacation.c,v 2.13 2017-03-13 14:10:51+05:30 Cprogrammer Stab mbhangui $";
#endif

char           *getuserinfo(char *);
int             VacationChk(char *, char *);
static void     usage();

int
main(int argc, char **argv)
{
	FILE           *fp, *inject_fp;
	char            buffer[MAX_BUFF + 2], VacationFname[MAXPATHLEN];
	char            cmmd[MAX_BUFF], ToId[MAX_BUFF], FromId[MAX_BUFF], Subject[4096];
	char           *ptr, *cptr, *HomeDir, *sender;

	if(argc == 2 || argc == 3)
	{
		if(!strncmp(argv[1], "-h", 3))
		{
			usage();
			_exit(0);
		}
		return(add_vacation(argv[1], ((argc == 3) ? argv[2] : (char *) 0)));
	} else
	if(argc > 3)
	{
		usage();
		_exit(0);
	}
	*Subject = 0;
	HomeDir = (char *) NULL;
	/*- RECIPIENT=satyam.net.in-mbhangui@satyam.net.in -*/
	*VacationFname = 0;
	if((ptr = (char *) getenv("RECIPIENT")) != (char *) NULL)
	{
		if((cptr = strchr(ptr, '-')) != (char *) NULL)
		{
			cptr++;
			scopy(FromId, cptr, MAX_BUFF);
		}
		if ((HomeDir = getuserinfo(FromId)) != (char *) NULL)
			(void) snprintf(VacationFname, MAXPATHLEN, "%s/.vacation.msg", HomeDir);
	} else
	{
		fprintf(stderr, "No RECIPIENT in environment\n");
		_exit(0);
	}
	if(!(sender = getenv("SENDER")))
	{
		fprintf(stderr, "No SENDER in environment\n");
		_exit(0);
	} else
	if (!strcmp(sender, "#@[]"))
	{
		fprintf(stderr, "SENDER is <#@[]> (double bounce message)");
		_exit(0);
	} else
	if (!strchr(sender, '@'))
	{
		fprintf(stderr, "SENDER did not contain a hostname");
		_exit(0);
	} else
	if (!strncasecmp(sender, "mailer-daemon", 13))
	{
		fprintf(stderr, "SENDER was mailer-daemon");
		_exit(0);
	}
	/*- RPLINE=Return-Path: <mbhangui@yahoo.com> -*/
	if((ptr = (char *) getenv("RPLINE")) != (char *) NULL)
	{
		ptr += 12;
		for (; isspace((int) *ptr); ptr++);
		for (cptr = ToId; *ptr; ptr++)
		{
			if (*ptr != '<' && *ptr != '>' &&
				*ptr != '\n')
				*cptr++ = *ptr;
		}
		*cptr = 0;
	} else
	{
		fprintf(stderr, "No RPLINE in environment\n");
		_exit(0);
	}
	for (;;)
	{
		if (!fgets(buffer, MAX_BUFF, stdin))
		{
			if (feof(stdin))
				break;
			perror("fgets");
			_exit(111);
		}
		if (*buffer == '\n')
			break;
		if(!strncmp(buffer, "Subject:", 8))
		{
			(void) snprintf(Subject, 4096, "Re:%s", buffer + 8);
			break;
		}
	}
	if (VacationChk(ToId, HomeDir))
		return (0);
	if (*VacationFname)
	{
		snprintf(cmmd, MAX_BUFF, PREFIX"/bin/qmail-inject -f%s %s", FromId, ToId);
		if (!(inject_fp = popen(cmmd, "w")))
		{
			perror(cmmd);
			_exit(111);
		}
		fprintf(inject_fp, "To: %s\n", ToId);
		fprintf(inject_fp, "From: %s\n", FromId);
		if(*Subject)
			fprintf(inject_fp, "Subject: %s\n", Subject);
		else
			fprintf(inject_fp, "Subject: Auto Response from %s\n", FromId);
		if ((ptr = getenv("CHARSET")))
		{
			fprintf(inject_fp, "Mime-Version: 1.0\n");
			fprintf(inject_fp, "Content-Type: text/plain; charset=\"%s\"\n", ptr);
		}
		if((fp = fopen(VacationFname, "r")) != (FILE *) 0)
		{
			for (;;)
			{
				if (!fgets(buffer, MAX_BUFF, fp))
				{
					if (feof(fp))
						break;
					perror("fgets");
					_exit (111);
				}
				fprintf(inject_fp, "%s", buffer);
			}
		} else
			fprintf(inject_fp, "This is an Auto Reply from %s\n", FromId);
		fprintf(inject_fp, "-----------------------------------------------------------------------------\n");
		fprintf(inject_fp, "Note: Further Auto Response will be deferred to %s\n", ToId);
		fprintf(inject_fp, "      till 24 Hrs Elapses and a new message is received\n");
		fprintf(inject_fp, "      from %s\n", ToId);
		fprintf(inject_fp, "Welcome to IndiMail - Fastest Mail on Earth\n");
		fprintf(inject_fp, "Download for Free, OpenSource IndiMail at http://www.indimail.org\n");
		fprintf(inject_fp, "-----------------------------------------------------------------------------\n");
		if(fp)
			fclose(fp);
		pclose(inject_fp);
	}
	_exit (0);
}

char           *
getuserinfo(username)
	char           *username;
{
	char           *ptr, *cptr, *default_domain;
	struct passwd  *mypw;
	static char     HomeDir[MAX_BUFF];
	char            Email[MAX_BUFF], User[MAX_BUFF], Domain[MAX_BUFF];
	extern int      userNotFound;

	for (cptr = Email, ptr = username; *ptr; ptr++)
	{
		if (*ptr != '<' && *ptr != '>')
			*cptr++ = *ptr;
	}
	*cptr = 0;
	if (parse_email(Email, User, Domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: Email too long\n", Email);
		exit (100);
	}
	ptr = vget_real_domain(Domain);
	if (!(mypw = vauth_getpw(User, ptr)))
	{
		if(!userNotFound)
			exit(111);
		getEnvConfigStr(&default_domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		if (!*Domain)
			printf("no such user %s@%s\n", User, default_domain);
		else
			printf("no such user %s@%s\n", User, Domain);
		vclose();
		exit(100);
	}
	vclose();
	scopy(HomeDir, mypw->pw_dir, MAX_BUFF);
	return (HomeDir);
}

int
VacationChk(EmailAddr, Homedir)
	char           *EmailAddr, *Homedir;
{
	char            fname[MAXPATHLEN];
	struct stat     statbuf;
	time_t          curtime;

	curtime = time(0);
	snprintf(fname, MAXPATHLEN, "%s/.vacation.dir", Homedir);
	if (access(fname, F_OK) && mkdir(fname, 0755))
		return (0);
	snprintf(fname, MAXPATHLEN, "%s/.vacation.dir/%s", Homedir,
			EmailAddr);
	if (stat(fname, &statbuf))
	{
		if (errno == 2)
			(void) open(fname, O_WRONLY | O_CREAT | O_TRUNC,
						0644);
		return (0);
	}
	if ((curtime - statbuf.st_mtime) > 86400)
	{
		unlink(fname);
		(void) open(fname, O_WRONLY | O_CREAT | O_TRUNC,
					0644);
	} else
		return (1);
	return(0);
}

static void
usage()
{
	fprintf(stderr, "USAGE: vacation [email_addr] [vacation_mesg_file]\n");
	fprintf(stderr, "where vacation_mesg_file can be either of the following\n");
	fprintf(stderr, "-         for removing vacation\n");
	fprintf(stderr, "+         for adding new vacation with text from stdin\n");
	fprintf(stderr, "file_path for taking the content for vacation from file_path\n");
}

void
getversion_vacation_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
