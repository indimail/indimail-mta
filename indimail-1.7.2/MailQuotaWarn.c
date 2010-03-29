/*
 * $Log: MailQuotaWarn.c,v $
 * Revision 2.16  2010-03-24 10:14:50+05:30  Cprogrammer
 * moved overquota.sh to libexec directory
 *
 * Revision 2.15  2010-03-22 14:35:34+05:30  Cprogrammer
 * fixed typo in comments
 *
 * Revision 2.14  2009-10-14 20:43:39+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.13  2009-09-25 23:49:58+05:30  Cprogrammer
 * check return value of recalc_quota()
 *
 * Revision 2.12  2009-09-23 15:00:11+05:30  Cprogrammer
 * change for new runcmmd
 *
 * Revision 2.11  2009-06-03 09:29:36+05:30  Cprogrammer
 * fix division by zero is quota alloted is non-numeric (e.g. NOQUOTA)
 *
 * Revision 2.10  2009-06-02 15:41:20+05:30  Cprogrammer
 * fixed division by zero
 *
 * Revision 2.9  2008-06-24 21:56:18+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.8  2008-06-13 09:51:24+05:30  Cprogrammer
 * fixed compilation warning if USE_MAILDIRQUOTA is not defined
 *
 * Revision 2.7  2007-12-22 00:19:25+05:30  Cprogrammer
 * send overquota warning if previous warning > 7 days
 *
 * Revision 2.6  2005-12-29 22:46:32+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.5  2004-07-13 15:10:52+05:30  Cprogrammer
 * bug fix for calling overquota.sh
 *
 * Revision 2.4  2004-07-12 22:55:32+05:30  Cprogrammer
 * quotawarnings made configurable
 * option to run a script on overquota
 *
 * Revision 2.3  2003-02-01 14:11:53+05:30  Cprogrammer
 * change for CopyEmailFile() change
 *
 * Revision 2.2  2002-12-01 18:50:16+05:30  Cprogrammer
 * added unread and unseen message count
 *
 * Revision 2.1  2002-10-12 02:37:22+05:30  Cprogrammer
 * code to pick up maildirsize or .current_size from the base Maildir
 *
 * Revision 1.5  2002-02-24 03:23:12+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.4  2001-12-22 21:44:28+05:30  Cprogrammer
 * added trash information in Mail Quota Warning
 *
 * Revision 1.3  2001-11-24 12:22:51+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:19+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef	lint
static char     sccsid[] = "$Id: MailQuotaWarn.c,v 2.16 2010-03-24 10:14:50+05:30 Cprogrammer Stab mbhangui $";
#endif

int
MailQuotaWarn(char *username, char *domain, char *Maildir, char *QuotaAlloted)
{
	char            maildir[MAX_BUFF], quotawarn[16], quota_cmd[MAX_BUFF], tmpbuf[MAX_BUFF];
	char           *ptr;
	int             i, percent_warn, percent_usage_disk, warn_usage, warn_mail;
	struct stat     statbuf;
	mdir_t          Quota, total_usage;
#ifdef USE_MAILDIRQUOTA
	mdir_t          QuotaCount, mailcount;
	int             percent_usage_mail;
#endif

	/*-
	 * If the age of file QuotaWarn is more than a week send warning
	 * to user if overquota
	 */
	if (!strncmp(QuotaAlloted, "NOQUOTA", 7))
		return(0);
	snprintf(tmpbuf, MAX_BUFF, "%s/QuotaWarn", Maildir);
	if (!((stat(tmpbuf, &statbuf) ? time(0) : time(0) - statbuf.st_mtime) > 7 * 86400))
		return(0);
	scopy(maildir, Maildir, MAX_BUFF);
	if((ptr = strstr(maildir, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
#ifdef USE_MAILDIRQUOTA
	if ((Quota = parse_quota(QuotaAlloted, &QuotaCount)) == -1)
	{
		fprintf(stderr, "parse_quota: %s: %s\n", QuotaAlloted, strerror(errno));
		return (-1);
	} else
	if (!Quota)
		return (0);
	total_usage = recalc_quota(maildir, &mailcount, Quota, QuotaCount, 0);
#else
	if (!(Quota = atol(QuotaAlloted)))
		return (0);
	total_usage = recalc_quota(maildir, 0);
#endif
	if (total_usage == -1)
		return (-1);
	warn_usage = warn_mail = 0;
	for (i = 10;i;i--)
	{
		snprintf(quotawarn, sizeof(quotawarn), "QUOTAWARN%d", i);
		if ((ptr = getenv(quotawarn)))
		{
			percent_warn = atoi(ptr);
			if (percent_warn < 0 || percent_warn > 100)
				continue;
			else
			{
				if ((percent_usage_disk = (total_usage * 100) / Quota) > percent_warn)
					warn_usage = 1;
#ifdef USE_MAILDIRQUOTA
				if (QuotaCount && (percent_usage_mail = (mailcount * 100) / QuotaCount) > percent_warn)
					warn_mail = 1;
#endif
				if (warn_usage || warn_mail)
				{
					close(open(tmpbuf, O_CREAT | O_TRUNC , 0644));
					getEnvConfigStr(&ptr, "OVERQUOTA_CMD", INDIMAILDIR"/libexec/overquota.sh");
					if (!access(ptr, X_OK))
					{
						/*
						 * Call overquota command with 7 arguments
						 * email maildir total_usage mailcount Quota QuotaCount percent_warn
						 */
#ifdef USE_MAILDIRQUOTA
						snprintf(quota_cmd, sizeof(quota_cmd),
							"%s %s@%s %s %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %d",
							ptr, username, domain, maildir, total_usage, mailcount, Quota,
							QuotaCount, percent_warn);
#else
						snprintf(quota_cmd, sizeof(quota_cmd),
							"%s %s@%s %s %"PRIu64" %"PRIu64" %"PRIu64" %"PRIu64" %d",
							ptr, username, domain, maildir, total_usage, -1, Quota,
							-1, percent_warn);
#endif
						runcmmd(quota_cmd, 0);
					}
					return(1);
				}
			}
		}
	}
	return (0);
}

void
getversion_MailQuotaWarn_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
