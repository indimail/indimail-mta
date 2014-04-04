/*
 * $Log: user_over_quota.c,v $
 * Revision 2.13  2012-04-22 13:58:43+05:30  Cprogrammer
 * added case for specification of quota in gigabytes
 *
 * Revision 2.12  2009-10-14 20:45:54+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.11  2009-06-04 16:27:37+05:30  Cprogrammer
 * check return status of recalc_quota
 *
 * Revision 2.10  2009-06-03 11:24:49+05:30  Cprogrammer
 * recalculate quota if user over quota
 *
 * Revision 2.9  2008-11-11 15:58:19+05:30  Cprogrammer
 * fix for quota checks
 *
 * Revision 2.8  2008-11-06 15:39:51+05:30  Cprogrammer
 * do not return overquota if maildirsize contains 0S,??
 *
 * Revision 2.7  2008-07-13 19:48:59+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.6  2008-06-24 21:59:29+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.5  2005-12-21 09:50:55+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.4  2003-02-01 15:36:37+05:30  Cprogrammer
 * removed update_flag argument
 *
 * Revision 2.3  2002-10-20 22:19:59+05:30  Cprogrammer
 * corrections for solaris compilation
 *
 * Revision 2.2  2002-10-12 02:37:51+05:30  Cprogrammer
 * code to pick up maildirsize or .current_size from the base Maildir
 *
 * Revision 2.1  2002-09-04 23:01:23+05:30  Cprogrammer
 * determine quota from maildirsize itself
 *
 * Revision 1.4  2002-02-24 03:26:11+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.3  2001-11-24 12:20:18+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:14+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:16+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef linux
#include <sys/file.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: user_over_quota.c,v 2.13 2012-04-22 13:58:43+05:30 Cprogrammer Stab mbhangui $";
#endif

/* 
 * Check if the user is over quota
 * Do all quota recalculation needed
 * Return 1 if user is over quota
 * Return 0 if user is not over quota
 */
int
user_over_quota(char *Maildir, char *quota, int cur_msgsize)
{
#ifdef USE_MAILDIRQUOTA
	mdir_t          mail_count_limit, cur_mailbox_count;
	char            tmpbuf[MAX_BUFF];
	char           *tmpQuota;
	FILE           *fp;
#else
	int             i;
#endif
	char            maildir[MAX_BUFF];
	char           *ptr;
	mdir_t          mail_size_limit, cur_mailbox_size;

	scopy(maildir, Maildir, MAX_BUFF);
	if ((ptr = strstr(maildir, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
#ifdef USE_MAILDIRQUOTA
	if (!quota || !*quota)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/maildirsize", maildir);
		if (!(fp = fopen(tmpbuf, "r")))
			return(0);
		if (!fgets(tmpbuf, MAX_BUFF - 2, fp))
		{
			fclose(fp);
			return(-1);
		}
		fclose(fp);
		if ((tmpQuota = strchr(tmpbuf, '\n')))
			*tmpQuota = 0;
		tmpQuota = tmpbuf;
	} else
		tmpQuota = quota;
	if (!strncmp(tmpQuota, "0S", 2))
		return(0);
	if ((mail_size_limit = parse_quota(tmpQuota, &mail_count_limit)) == -1)
		return(-1);
#else
	if (!quota || !*quota)
		return(0);
	/*
	 * translate the quota to a number 
	 */
	mail_size_limit = atol(quota);
	for (i = 0; quota[i] != 0; ++i)
	{
		if (quota[i] == 'k' || quota[i] == 'K')
		{
			mail_size_limit = mail_size_limit * 1024;
			break;
		}
		if (quota[i] == 'm' || quota[i] == 'M')
		{
			mail_size_limit = mail_size_limit * 1048576;
			break;
		}
		if (quota[i] == 'g' || quota[i] == 'G')
		{
			mail_size_limit = mail_size_limit * 1073741824;
			break;
		}
	}
#endif
	/*
	 * Get their current total from maildirsize
	 */
#ifdef USE_MAILDIRQUOTA
	if ((CurBytes = cur_mailbox_size = recalc_quota(maildir, &cur_mailbox_count, 
		mail_size_limit, mail_count_limit, 0)) == -1)
		return (-1);
	CurCount = cur_mailbox_count;
	if (cur_mailbox_size + cur_msgsize > mail_size_limit) /*- if over quota recalculate quota */
	{
		if ((CurBytes = cur_mailbox_size = recalc_quota(maildir, &cur_mailbox_count, 
			mail_size_limit, mail_count_limit, 2)) == -1)
			return (-1);
		if (cur_mailbox_size + cur_msgsize > mail_size_limit)
			return(1);
	}
	if (mail_count_limit && (cur_mailbox_count + 1 > mail_count_limit))
		return(1);
#else
	if ((CurBytes = cur_mailbox_size = recalc_quota(maildir, 0)) == -1)
		return (-1);
	/*
	 * Check if this email would bring them over quota 
	 */
	if (cur_mailbox_size + cur_msgsize > mail_size_limit)
	{
		/*
		 * recalculate their quota since they might have
		 * deleted email 
		 */
		if ((CurBytes = cur_mailbox_size = recalc_quota(maildir, 2)) == -1)
			return (-1);
		if (cur_mailbox_size + cur_msgsize > mail_size_limit)
			return(1);
	}
#endif
	return(0);
}

void
getversion_user_over_quota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
