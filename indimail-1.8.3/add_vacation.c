/*
 * $Log: add_vacation.c,v $
 * Revision 2.9  2010-03-08 22:04:03+05:30  Cprogrammer
 * renamed qmail-autoresponder as autoresponder to shorten path
 *
 * Revision 2.8  2009-11-06 18:48:29+05:30  Cprogrammer
 * added missing newline in .qmail file for vacation
 *
 * Revision 2.7  2004-07-03 23:49:56+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.6  2004-05-17 14:00:32+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.5  2003-08-06 22:25:28+05:30  Cprogrammer
 * added some documentation
 *
 * Revision 2.4  2003-07-06 22:26:38+05:30  Cprogrammer
 * create vacation.dir
 * corrected Maildir path in .qmail file
 *
 * Revision 2.3  2003-07-06 21:06:48+05:30  Cprogrammer
 * use qmail-autoresponder
 *
 * Revision 2.2  2003-06-23 22:50:48+05:30  Cprogrammer
 * corrected permissions of .vacation.msg and .qmail file when run through root
 *
 * Revision 2.1  2002-05-12 01:06:48+05:30  Cprogrammer
 * routine for adding auto responder
 *
 */
#include "indimail.h"
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: add_vacation.c,v 2.9 2010-03-08 22:04:03+05:30 Cprogrammer Stab mbhangui $";
#endif

int
add_vacation(char *email, char *fname)
{
	char            TmpBuf[MAX_BUFF], buffer[MAX_BUFF], user[MAX_BUFF], domain[MAX_BUFF];
	FILE           *fp1, *fp2;
	uid_t           uid;
	gid_t           gid;
	char           *real_domain;
	struct passwd  *pw;
	int             err;

	if (parse_email(email, user, domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: Email too long\n", email);
		return (1);
	}
	if (!(real_domain = vget_real_domain(domain)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return (1);
	} else
	if (!vget_assign(real_domain, 0, 0, &uid, &gid))
	{
		fprintf(stderr, "%s: No such domain\n", real_domain);
		return (1);
	} else
	if (!(pw = vauth_getpw(user, real_domain)))
	{
		fprintf(stderr, "no such user %s@%s\n", user, real_domain);
		return (1);
	}
	/*- Remove Vacation */
	if (fname && *fname && !strncmp(fname, "-", 2))
	{
		err = 0;
		snprintf(TmpBuf, MAX_BUFF, "%s/.qmail", pw->pw_dir);
		if (!access(TmpBuf, F_OK) && unlink(TmpBuf))
		{
			perror(TmpBuf);
			err = 1;
		}
		snprintf(TmpBuf, MAX_BUFF, "%s/.vacation.msg", pw->pw_dir);
		if (!access(TmpBuf, F_OK) && unlink(TmpBuf))
		{
			perror(TmpBuf);
			err = 1;
		}
		snprintf(TmpBuf, MAX_BUFF, "%s/.vacation.dir", pw->pw_dir);
		if (!access(TmpBuf, F_OK) && vdelfiles(TmpBuf, user, 0))
			err = 1;
		return (err);
	}
	/*- Add vacation */
	snprintf(TmpBuf, MAX_BUFF, "%s/.qmail", pw->pw_dir);
	if (!(fp1 = fopen(TmpBuf, "w")))
	{
		perror(TmpBuf);
		return(1);
	}
	/*- fprintf(fp1, "| %s/bin/vacation\n", INDIMAILDIR); -*/
	fprintf(fp1, "| %s/bin/autoresponder -q %s/.vacation.msg %s/.vacation.dir\n%s/Maildir/\n",
		QMAILDIR, pw->pw_dir, pw->pw_dir, pw->pw_dir);
	fclose(fp1);
	if (chown(TmpBuf, uid, gid) || chmod(TmpBuf, INDIMAIL_QMAIL_MODE))
	{
		perror(TmpBuf);
		unlink(TmpBuf);
		return (-1);
	}
	snprintf(TmpBuf, MAX_BUFF, "%s/.vacation.dir", pw->pw_dir);
	if(r_mkdir(TmpBuf, INDIMAIL_DIR_MODE, uid, gid))
	{
		perror(TmpBuf);
		return (-1);
	}
	if (fname && *fname)
	{
		if (!strncmp(fname, "+", 2)) /*- Take text from STDIN */
		{
			fp1 = stdin;
			printf("Enter Text for Vacation Message. Type ^D (Ctrl-D) to end\n");
		} else
		if (!(fp1 = fopen(fname, "r"))) /*- Take text from an existing file */
		{
			perror(fname);
			snprintf(TmpBuf, MAX_BUFF, "%s/.qmail", pw->pw_dir);
			unlink(TmpBuf);
			return (1);
		}
		(void) snprintf(TmpBuf, MAX_BUFF, "%s/.vacation.msg", pw->pw_dir);
		if (!(fp2 = fopen(TmpBuf, "w")))
		{
			if (fp1 != stdin)
				fclose(fp1);
			perror(TmpBuf);
			snprintf(TmpBuf, MAX_BUFF, "%s/.qmail", pw->pw_dir);
			unlink(TmpBuf);
			return (1);
		}
		if (chown(TmpBuf, uid, gid) || chmod(TmpBuf, INDIMAIL_QMAIL_MODE))
		{
			if (fp1 != stdin)
				fclose(fp1);
			fclose(fp2);
			perror(TmpBuf);
			unlink(TmpBuf);
			snprintf(TmpBuf, MAX_BUFF, "%s/.qmail", pw->pw_dir);
			unlink(TmpBuf);
			return (-1);
		}
		for (;;)
		{
			if (!fgets(buffer, MAX_BUFF - 2, fp1))
				break;
			fprintf(fp2, "%s", buffer);
		}
		fclose(fp2);
		printf("Successfully wrote Vacation Message to %s\n", TmpBuf);
		if (fp1 != stdin)
			fclose(fp1);
	}
	return (0);
}

void getversion_add_vacation_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
