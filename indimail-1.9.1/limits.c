/*
 * $Log: limits.c,v $
 * Revision 2.15  2013-06-10 15:44:28+05:30  Cprogrammer
 * changed defaultquota to signed int64
 *
 * Revision 2.14  2012-09-24 19:16:29+05:30  Cprogrammer
 * BUG - Fixed diskquota, maxmsgcount, defaultquota, defaultmaxmsgcount initialization
 *
 * Revision 2.13  2012-04-22 13:57:57+05:30  Cprogrammer
 * formatted sql statement
 *
 * Revision 2.12  2011-02-11 22:59:39+05:30  Cprogrammer
 * fix for specifing > 2GB quota
 *
 * Revision 2.11  2009-12-01 11:59:23+05:30  Cprogrammer
 * fixed syntax error in mysql_query
 *
 * Revision 2.10  2008-10-20 19:06:04+05:30  Cprogrammer
 * added passwd_expiry
 *
 * Revision 2.9  2008-09-08 09:45:18+05:30  Cprogrammer
 * change for using mysql_real_escape_string
 *
 * Revision 2.8  2008-05-28 16:36:39+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.7  2006-01-23 21:55:27+05:30  Cprogrammer
 * added domain_expiry limit
 *
 * Revision 2.6  2003-10-25 00:00:27+05:30  Cprogrammer
 * set default limits if no rows found for domain
 * corrected typo
 *
 * Revision 2.5  2003-10-24 23:14:55+05:30  Cprogrammer
 * made vdefault_limits() and vlimits_get_flag_mask() visible
 *
 * Revision 2.4  2003-10-23 13:17:50+05:30  Cprogrammer
 * vlimits_get_gid_mask() changed to vlimits_get_flag_mask()
 *
 * Revision 2.3  2003-10-01 02:11:11+05:30  Cprogrammer
 * new code for limits
 *
 * Revision 2.2  2003-07-02 18:25:13+05:30  Cprogrammer
 * mysql_query() not needed if table not present
 *
 * Revision 2.1  2002-10-28 18:28:57+05:30  Cprogrammer
 * function for setting domain administrative limits
 *
 * vlimits.c
 * handle domain limits in both file format
 * Brian Kolaci <bk@galaxy.net>
 */

#ifndef	lint
static char     sccsid[] = "$Id: limits.c,v 2.15 2013-06-10 15:44:28+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#ifdef ENABLE_DOMAIN_LIMITS
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <mysqld_error.h>

static void     vdefault_limits(struct vlimits *limits);

int
vget_limits(char *domain, struct vlimits *limits)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err, perm;
	MYSQL_ROW       row;
	MYSQL_RES      *res;

	/*- initialise a limits struct.  */
	vdefault_limits(limits);
	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"SELECT domain_expiry, passwd_expiry, maxpopaccounts, maxaliases, maxforwards,"
		"maxautoresponders, maxmailinglists, diskquota, maxmsgcount, defaultquota,"
		"defaultmaxmsgcount, disable_pop, disable_imap, disable_dialup,"
		"disable_passwordchanging, disable_webmail, disable_relay, disable_smtp,"
		"perm_account, perm_alias, perm_forward, perm_autoresponder, perm_maillist,"
		"perm_quota, perm_defaultquota FROM vlimits WHERE domain = \"%s\"", domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			fprintf(stderr, "vget_limits: No rows selected for domain '%s'\n", domain);
			create_table(ON_LOCAL, "vlimits", LIMITS_TABLE_LAYOUT);
			return(0);
		}
		mysql_perror("vget_limits: %s", SqlBuf);
		return (-1);
	}
	if (!(res = mysql_store_result(&mysql[1])))
	{
		mysql_perror("vget_limits: mysql_store_results\n");
		return (-1);
	}
	if (!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return (0);
	}
	if ((row = mysql_fetch_row(res)))
	{

		limits->domain_expiry = atoi(row[0]);
		limits->passwd_expiry = atoi(row[1]);
		limits->maxpopaccounts = atoi(row[2]);
		limits->maxaliases = atoi(row[3]);
		limits->maxforwards = atoi(row[4]);
		limits->maxautoresponders = atoi(row[5]);
		limits->maxmailinglists = atoi(row[6]);
		limits->diskquota = strtoll(row[7], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->diskquota == LLONG_MIN || limits->diskquota == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			mysql_free_result(res);
			return (-1);
		}
		limits->maxmsgcount = strtoll(row[8], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->maxmsgcount == LLONG_MIN || limits->maxmsgcount == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			mysql_free_result(res);
			return (-1);
		}
		limits->defaultquota = strtoll(row[9], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->defaultquota == LLONG_MIN || limits->defaultquota == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			mysql_free_result(res);
			return (-1);
		}
		limits->defaultmaxmsgcount = strtoll(row[10], 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
		if (limits->defaultmaxmsgcount == LLONG_MIN || limits->defaultmaxmsgcount == LLONG_MAX)
#else
		if (errno == ERANGE)
#endif
		{
			mysql_free_result(res);
			return (-1);
		}
		limits->disable_pop = atoi(row[11]);
		limits->disable_imap = atoi(row[12]);
		limits->disable_dialup = atoi(row[13]);
		limits->disable_passwordchanging = atoi(row[14]);
		limits->disable_webmail = atoi(row[15]);
		limits->disable_relay = atoi(row[16]);
		limits->disable_smtp = atoi(row[17]);
		limits->perm_account = atoi(row[18]);
		limits->perm_alias = atoi(row[19]);
		limits->perm_forward = atoi(row[20]);
		limits->perm_autoresponder = atoi(row[21]);
		perm = atol(row[22]);
		limits->perm_maillist = perm & VLIMIT_DISABLE_ALL;
		perm >>= VLIMIT_DISABLE_BITS;
		limits->perm_maillist_users = perm & VLIMIT_DISABLE_ALL;
		perm >>= VLIMIT_DISABLE_BITS;
		limits->perm_maillist_moderators = perm & VLIMIT_DISABLE_ALL;
		limits->perm_quota = atoi(row[23]);
		limits->perm_defaultquota = atoi(row[24]);
	}
	mysql_free_result(res);
	return (0);
}

int
vdel_limits(char *domain)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"DELETE low_priority FROM vlimits WHERE domain = \"%s\"", domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "vlimits", LIMITS_TABLE_LAYOUT))
				return (-1);
			return (0);
		}
		mysql_perror("vdel_limits: %s", SqlBuf);
		return (-1);
	}
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("vdel_limits: mysql_affected_rows");
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err && verbose)
		printf("Deleted limits (%d entries) for domain %s\n", err, domain);
	else
	if (verbose)
		printf("No limits for domain %s\n", domain);
	return 0;
}

int
vset_limits(char *domain, struct vlimits *limits)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"REPLACE INTO vlimits (domain, domain_expiry, passwd_expiry, maxpopaccounts, maxaliases, "
		"maxforwards, maxautoresponders, maxmailinglists, diskquota, maxmsgcount, defaultquota, "
		"defaultmaxmsgcount, disable_pop, disable_imap, disable_dialup, "
		"disable_passwordchanging, disable_webmail, disable_relay, disable_smtp, perm_account, "
		"perm_alias, perm_forward, perm_autoresponder, perm_maillist, perm_quota, "
		"perm_defaultquota)"
		"VALUES"
		"(\"%s\", %ld, %ld, %d, %d, %d, %d, %d, %"PRIu64", %"PRIu64", %"PRId64", %"PRIu64", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
		domain, limits->domain_expiry, limits->passwd_expiry, limits->maxpopaccounts,
		limits->maxaliases, limits->maxforwards, limits->maxautoresponders, limits->maxmailinglists,
		limits->diskquota, limits->maxmsgcount, limits->defaultquota, limits->defaultmaxmsgcount,
		limits->disable_pop, limits->disable_imap, limits->disable_dialup,
		limits->disable_passwordchanging, limits->disable_webmail, limits->disable_relay,
		limits->disable_smtp, limits->perm_account, limits->perm_alias, limits->perm_forward,
		limits->perm_autoresponder,
		(limits->perm_maillist | (limits->perm_maillist_users << VLIMIT_DISABLE_BITS) |
		(limits->perm_maillist_moderators << (VLIMIT_DISABLE_BITS * 2))),
		limits->perm_quota, limits->perm_defaultquota);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "vlimits", LIMITS_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("vset_limits: %s", SqlBuf);
				return (-1);
			}
		}
		mysql_perror("vset_limits: %s", SqlBuf);
		return (-1);
	}
	if ((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("vset_limits: mysql_affected_rows");
		return (-1);
	}
	if (!verbose)
		return (err ? 0 : 1);
	if (err)
		printf("Added limits for domain %s\n", domain);
	else
		printf("No limits added for domain %s\n", domain);
	return (err ? 0 : 1);
}

int
vlimits_get_flag_mask(struct vlimits *limits)
{
	int             mask = 0;

	if (limits->disable_pop != 0)
		mask |= NO_POP;
	if (limits->disable_smtp != 0)
		mask |= NO_SMTP;
	if (limits->disable_imap != 0)
		mask |= NO_IMAP;
	if (limits->disable_relay != 0)
		mask |= NO_RELAY;
	if (limits->disable_webmail != 0)
		mask |= NO_WEBMAIL;
	if (limits->disable_passwordchanging != 0)
		mask |= NO_PASSWD_CHNG;
	if (limits->disable_dialup != 0)
		mask |= NO_POP;
	 return mask; 
}

static void
vdefault_limits(struct vlimits *limits)
{
	/*- initialize structure */
	memset(limits, 0, sizeof(*limits));

	limits->domain_expiry = -1;
	limits->passwd_expiry = -1;
	limits->maxpopaccounts = -1;
	limits->maxaliases = -1;
	limits->maxforwards = -1;
	limits->maxautoresponders = -1;
	limits->maxmailinglists = -1;
	limits->diskquota = 0;
	limits->maxmsgcount = 0;
	limits->defaultquota = 0;
	limits->defaultmaxmsgcount = 0;

	limits->disable_pop = 0;
	limits->disable_imap = 0;
	limits->disable_dialup = 0;
	limits->disable_passwordchanging = 0;
	limits->disable_webmail = 0;
	limits->disable_relay = 0;
	limits->disable_smtp = 0;
	limits->perm_account = 0;
	limits->perm_alias = 0;
	limits->perm_forward = 0;
	limits->perm_autoresponder = 0;
	limits->perm_maillist = 0;
	limits->perm_maillist_users = 0;
	limits->perm_maillist_moderators = 0;
	limits->perm_quota = 0;
	limits->perm_defaultquota = 0;
}
#endif /*- #ifdef ENABLE_DOMAIN_LIMITS */

void
getversion_limits_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
