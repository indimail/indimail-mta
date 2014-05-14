/*
 * $Log: vlimit.c,v $
 * Revision 2.12  2013-06-10 16:06:58+05:30  Cprogrammer
 * allow setting of NOQUOTA for default user quota
 *
 * Revision 2.11  2011-07-29 09:26:34+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.10  2011-02-11 23:02:05+05:30  Cprogrammer
 * fix for setting & displaying > 2Gb in quota and message counts
 *
 * Revision 2.9  2010-04-11 18:54:38+05:30  Cprogrammer
 * fixed domain_expiry, passwd_expiry getting reset
 *
 * Revision 2.8  2009-12-01 16:28:42+05:30  Cprogrammer
 * removed delete permission for user quota
 *
 * Revision 2.7  2009-12-01 11:59:09+05:30  Cprogrammer
 * show limits by default
 *
 * Revision 2.6  2008-10-24 22:07:10+05:30  Cprogrammer
 * added Domain Password Expiry
 *
 * Revision 2.5  2008-07-13 19:50:28+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.4  2008-06-13 10:47:27+05:30  Cprogrammer
 * fixed compilation errors if ENABLE_DOMAIN_LIMITS was not defined
 * print error and quit if ENABLE_DOMAIN_LIMIT is not defined
 *
 * Revision 2.3  2006-01-23 22:12:40+05:30  Cprogrammer
 * added domain_expiry
 *
 * Revision 2.2  2003-10-26 22:06:41+05:30  Cprogrammer
 * changed usage() display
 *
 * Revision 2.1  2003-10-25 00:02:24+05:30  Cprogrammer
 * program for setting domain limits
 *
 *
 * Copyright (C) 1999-2002 Inter7 Internet Technologies, Inc.
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
static char     sccsid[] = "$Id: vlimit.c,v 2.12 2013-06-10 16:06:58+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef ENABLE_DOMAIN_LIMITS
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

extern char    *strptime(const char *, const char *, struct tm *);
void            usage();
int             get_options(int argc, char **argv);

char            Domain[MAX_BUFF];
char            DomainQuota[MAX_BUFF];
char            DefaultUserQuota[MAX_BUFF];
char            DomainMaxMsgCount[MAX_BUFF];
char            DefaultUserMaxMsgCount[MAX_BUFF];
char            MaxPopAccounts[MAX_BUFF];
char            MaxAliases[MAX_BUFF];
char            MaxForwards[MAX_BUFF];
char            MaxAutoresponders[MAX_BUFF];
char            MaxMailinglists[MAX_BUFF];
char            GidFlagString[MAX_BUFF];
char            PermAccountFlagString[MAX_BUFF];
char            PermAliasFlagString[MAX_BUFF];
char            PermForwardFlagString[MAX_BUFF];
char            PermAutoresponderFlagString[MAX_BUFF];
char            PermMaillistFlagString[MAX_BUFF];
char            PermMaillistUsersFlagString[MAX_BUFF];
char            PermMaillistModeratorsFlagString[MAX_BUFF];
char            PermQuotaFlagString[MAX_BUFF];
char            PermDefaultQuotaFlagString[MAX_BUFF];

int             GidFlag = 0;
int             PermAccountFlag = 0;
int             PermAliasFlag = 0;
int             PermForwardFlag = 0;
int             PermAutoresponderFlag = 0;
int             PermMaillistFlag = 0;
int             PermMaillistUsersFlag = 0;
int             PermMaillistModeratorsFlag = 0;
int             PermQuotaFlag = 0;
int             PermDefaultQuotaFlag = 0;
long            domain_expiry = 0;
long            passwd_expiry = 0;

int             QuotaFlag = 0;
int             ShowLimits = 1;
int             DeleteLimits = 0;

int
main(int argc, char *argv[])
{
	int             i;
	struct vlimits  limits;


	if (get_options(argc, argv))
		return (1);
	if (!vget_assign(Domain, NULL, 0, NULL, NULL))
	{
		fprintf(stderr, "%s: No such domain\n", Domain);
		return (1);
	}
	if (vget_limits(Domain, &limits))
	{
		fprintf(stderr, "vget_limits: Failed to get limits: %s\n", strerror(errno));
		return (-1);
	}
	if (DeleteLimits)
	{
		if (vdel_limits(Domain) == 0)
		{
			printf("Limits deleted\n");
			return (0);
		} else
		{
			fprintf(stderr, "Failed to delete limits\n");
			return (-1);
		}
	}
	if (domain_expiry)
		limits.domain_expiry = domain_expiry;
	if (passwd_expiry)
		limits.passwd_expiry = passwd_expiry;
	if (MaxPopAccounts[0] != 0)
		limits.maxpopaccounts = atoi(MaxPopAccounts);
	if (MaxAliases[0] != 0)
		limits.maxaliases = atoi(MaxAliases);
	if (MaxForwards[0] != 0)
		limits.maxforwards = atoi(MaxForwards);
	if (MaxAutoresponders[0] != 0)
		limits.maxautoresponders = atoi(MaxAutoresponders);
	if (MaxMailinglists[0] != 0)
		limits.maxmailinglists = atoi(MaxMailinglists);
	/*
	 * quota & message count limits 
	 */
	if (DomainQuota[0] != 0 && (limits.diskquota = parse_quota(DomainQuota, 0)) == -1)
	{
		fprintf(stderr, "diskquota: %s\n", strerror(errno));
		return (-1);
	}
	if (DomainMaxMsgCount[0] != 0)
		limits.maxmsgcount = strtoll(DomainMaxMsgCount, 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (limits.maxmsgcount == LLONG_MIN || limits.maxmsgcount == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
	{
		fprintf(stderr, "maxmsgcount: %s\n", strerror(errno));
		return (-1);
	}

	if (DefaultUserQuota[0] != 0)
	{
		if (!strncmp(DefaultUserQuota, "NOQUOTA", 8))
			limits.defaultquota = -1;
		else
		if ((limits.defaultquota = parse_quota(DefaultUserQuota, 0)) == -1)
		{
			fprintf(stderr, "defaultquota: %s\n", strerror(errno));
			return (-1);
		}
	}
	if (DefaultUserMaxMsgCount[0] != 0 &&
			(limits.defaultmaxmsgcount = strtoll(DefaultUserMaxMsgCount, 0, 0)) == -1)
	{
		fprintf(stderr, "defaultmaxmsgcount: %s\n", strerror(errno));
		return (-1);
	}
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (limits.defaultmaxmsgcount == LLONG_MIN || limits.defaultmaxmsgcount == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
	{
		fprintf(stderr, "defaultmaxmsgcount: %s\n", strerror(errno));
		return (-1);
	}
	if (GidFlag == 1)
	{
		GidFlag = 0;
		limits.disable_dialup = 0;
		limits.disable_passwordchanging = 0;
		limits.disable_pop = 0;
		limits.disable_smtp = 0;
		limits.disable_webmail = 0;
		limits.disable_imap = 0;
		limits.disable_relay = 0;
		for (i = 0; i < strlen(GidFlagString); i++)
		{
			switch (GidFlagString[i])
			{
			case 'u':
				limits.disable_dialup = 1;
				break;
			case 'd':
				limits.disable_passwordchanging = 1;
				break;
			case 'p':
				limits.disable_pop = 1;
				break;
			case 's':
				limits.disable_smtp = 1;
				break;
			case 'w':
				limits.disable_webmail = 1;
				break;
			case 'i':
				limits.disable_imap = 1;
				break;
			case 'r':
				limits.disable_relay = 1;
				break;
			case 'x':
				limits.disable_dialup = 0;
				limits.disable_passwordchanging = 0;
				limits.disable_pop = 0;
				limits.disable_smtp = 0;
				limits.disable_webmail = 0;
				limits.disable_imap = 0;
				limits.disable_relay = 0;
				break;
			}
		}
	}
	if (PermAccountFlag == 1)
	{
		limits.perm_account = 0;
		for (i = 0; i < strlen(PermAccountFlagString); i++)
		{
			switch (PermAccountFlagString[i])
			{
			case 'a':
				limits.perm_account |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_account |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_account |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_account |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermAliasFlag == 1)
	{
		limits.perm_alias = 0;
		for (i = 0; i < strlen(PermAliasFlagString); i++)
		{
			switch (PermAliasFlagString[i])
			{
			case 'a':
				limits.perm_alias |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_alias |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_alias |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_alias |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermForwardFlag == 1)
	{
		limits.perm_forward = 0;
		for (i = 0; i < strlen(PermForwardFlagString); i++)
		{
			switch (PermForwardFlagString[i])
			{
			case 'a':
				limits.perm_forward |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_forward |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_forward |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_forward |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermAutoresponderFlag == 1)
	{
		limits.perm_autoresponder = 0;
		for (i = 0; i < strlen(PermAutoresponderFlagString); i++)
		{
			switch (PermAutoresponderFlagString[i])
			{
			case 'a':
				limits.perm_autoresponder |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_autoresponder |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_autoresponder |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_autoresponder |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermMaillistFlag == 1)
	{
		limits.perm_maillist = 0;
		for (i = 0; i < strlen(PermMaillistFlagString); i++)
		{
			switch (PermMaillistFlagString[i])
			{
			case 'a':
				limits.perm_maillist |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_maillist |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_maillist |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_maillist |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermMaillistUsersFlag == 1)
	{
		limits.perm_maillist_users = 0;
		for (i = 0; i < strlen(PermMaillistUsersFlagString); i++)
		{
			switch (PermMaillistUsersFlagString[i])
			{
			case 'a':
				limits.perm_maillist_users |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_maillist_users |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_maillist_users |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_maillist_users |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermMaillistModeratorsFlag == 1)
	{
		limits.perm_maillist_moderators = 0;
		for (i = 0; i < strlen(PermMaillistModeratorsFlagString); i++)
		{
			switch (PermMaillistModeratorsFlagString[i])
			{
			case 'a':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermQuotaFlag == 1)
	{
		limits.perm_quota = 0;
		for (i = 0; i < strlen(PermQuotaFlagString); i++)
		{
			switch (PermQuotaFlagString[i])
			{
			case 'a':
				limits.perm_quota |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_quota |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_quota |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_quota |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermDefaultQuotaFlag == 1)
	{
		limits.perm_defaultquota = 0;
		for (i = 0; i < strlen(PermDefaultQuotaFlagString); i++)
		{
			switch (PermDefaultQuotaFlagString[i])
			{
			case 'a':
				limits.perm_defaultquota |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_defaultquota |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_defaultquota |= VLIMIT_DISABLE_MODIFY;
				break;
			}
		}
	}
	if (!ShowLimits && vset_limits(Domain, &limits) != 0)
	{
		fprintf(stderr, "vset_limits: Failed to set limits\n");
		return (-1);
	}
	if (ShowLimits)
	{
		printf("Domain Expiry Date   : %s", limits.domain_expiry == -1 ? "Never Expires\n" : ctime(&limits.domain_expiry));
		printf("Password Expiry Date : %s", limits.passwd_expiry == -1 ? "Never Expires\n" : ctime(&limits.passwd_expiry));
		printf("Max Domain Quota     : %"PRIu64"\n", limits.diskquota);
		printf("Max Domain Messages  : %"PRIu64"\n", limits.maxmsgcount);
		if (limits.defaultquota == -1)
			printf("Default User Quota   : unlimited\n");
		else
			printf("Default User Quota   : %"PRId64"\n", limits.defaultquota);
		printf("Default User Messages: %"PRIu64"\n", limits.defaultmaxmsgcount);
		printf("Max Pop Accounts     : %d\n", limits.maxpopaccounts);
		printf("Max Aliases          : %d\n", limits.maxaliases);
		printf("Max Forwards         : %d\n", limits.maxforwards);
		printf("Max Autoresponders   : %d\n", limits.maxautoresponders);
		printf("Max Mailinglists     : %d\n", limits.maxmailinglists);
		printf("GID Flags:\n");
		if (limits.disable_imap != 0)
			printf("  NO_IMAP\n");
		if (limits.disable_smtp != 0)
			printf("  NO_SMTP\n");
		if (limits.disable_pop != 0)
			printf("  NO_POP\n");
		if (limits.disable_webmail != 0)
			printf("  NO_WEBMAIL\n");
		if (limits.disable_passwordchanging != 0)
			printf("  NO_PASSWD_CHNG\n");
		if (limits.disable_relay != 0)
			printf("  NO_RELAY\n");
		if (limits.disable_dialup != 0)
			printf("  NO_DIALUP\n");
		printf("Flags for non postmaster accounts:\n");
		printf("  pop account           : ");
		printf((limits.perm_account & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_account & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_account & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  alias                 : ");
		printf((limits.perm_alias & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_alias & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_alias & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  forward               : ");
		printf((limits.perm_forward & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_forward & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_forward & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  autoresponder         : ");
		printf((limits.perm_autoresponder & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_autoresponder & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_autoresponder & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  mailinglist           : ");
		printf((limits.perm_maillist & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_maillist & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_maillist & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  mailinglist users     : ");
		printf((limits.perm_maillist_users & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_maillist_users & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_maillist_users & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  mailinglist moderators: ");
		printf((limits.perm_maillist_moderators & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_maillist_moderators & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_maillist_moderators & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  domain quota          : ");
		printf((limits.perm_quota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_quota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf((limits.perm_quota & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE "));
		printf("\n");
		printf("  default quota         : ");
		printf((limits.perm_defaultquota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE "));
		printf((limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY "));
		printf("\n");
		return (0);
	}
	return (0);
}

void
usage()
{
	fprintf(stderr, "usage: vlimit [options] domain\n");
	fprintf(stderr, "options: -v ( display the indimail version number )\n");
	fprintf(stderr, "         -s ( show current settings )\n");
	fprintf(stderr, "         -D ( delete limits for this domain, i.e. switch to default limits)\n");
	fprintf(stderr, "         -e edate ( set domain   expiry date (ddmmyyyyHHMMSS) | -n no of days\n");
	fprintf(stderr, "         -t edate ( set password expiry date (ddmmyyyyHHMMSS) | -N no of days\n");
	fprintf(stderr, "         -Q quota ( set domain quota )\n");
	fprintf(stderr, "         -M count ( set domain max msg count )\n");
	fprintf(stderr, "         -q quota ( set default user quota )\n");
	fprintf(stderr, "         -m count ( set default user max msg count )\n");
	fprintf(stderr, "         -P count ( set max amount of pop accounts )\n");
	fprintf(stderr, "         -A count ( set max amount of aliases )\n");
	fprintf(stderr, "         -F count ( set max amount of forwards )\n");
	fprintf(stderr, "         -R count ( set max amount of autoresponders )\n");
	fprintf(stderr, "         -L count ( set max amount of mailing lists )\n");
	fprintf(stderr, "the following options are bit flags in the gid int field\n");
	fprintf(stderr, "         -g \"flags\"  (set flags, see below)\n");
	fprintf(stderr, "          gid flags:\n");
	fprintf(stderr, "            u ( set no dialup flag )\n");
	fprintf(stderr, "            d ( set no password changing flag )\n");
	fprintf(stderr, "            p ( set no pop access flag )\n");
	fprintf(stderr, "            s ( set no smtp access flag )\n");
	fprintf(stderr, "            w ( set no web mail access flag )\n");
	fprintf(stderr, "            i ( set no imap access flag )\n");
	fprintf(stderr, "            r ( set no external relay flag )\n");
	fprintf(stderr, "            x ( clear all flags )\n");
	fprintf(stderr, "the following options are bit flags for non postmaster admins\n");
	fprintf(stderr, "         -p \"flags\"  (set pop account flags)\n");
	fprintf(stderr, "         -a \"flags\"  (set alias flags)\n");
	fprintf(stderr, "         -f \"flags\"  (set forward flags)\n");
	fprintf(stderr, "         -r \"flags\"  (set autoresponder flags)\n");
	fprintf(stderr, "         -l \"flags\"  (set mailinglist flags)\n");
	fprintf(stderr, "         -u \"flags\"  (set mailinglist users flags)\n");
	fprintf(stderr, "         -o \"flags\"  (set mailinglist moderators flags)\n");
	fprintf(stderr, "         -x \"flags\"  (set domain  quota flags)\n");
	fprintf(stderr, "         -z \"flags\"  (set default quota flags)\n");
	fprintf(stderr, "         perm flags:\n");
	fprintf(stderr, "            a ( set deny all flag )\n");
	fprintf(stderr, "            c ( set deny create flag )\n");
	fprintf(stderr, "            m ( set deny modify flag )\n");
	fprintf(stderr, "            d ( set deny delete flag )\n");
}

int
get_options(int argc, char **argv)
{
	int             c;
	extern char    *optarg;
	extern int      optind;
	char            flag[4];
	struct tm       tm;

	memset(Domain, 0, sizeof(Domain));
	memset(DomainQuota, 0, sizeof(DomainQuota));
	memset(DefaultUserQuota, 0, sizeof(DefaultUserQuota));
	memset(DomainMaxMsgCount, 0, sizeof(DomainMaxMsgCount));
	memset(DefaultUserMaxMsgCount, 0, sizeof(DefaultUserMaxMsgCount));
	memset(MaxPopAccounts, 0, sizeof(MaxPopAccounts));
	memset(MaxAliases, 0, sizeof(MaxAliases));
	memset(MaxForwards, 0, sizeof(MaxForwards));
	memset(MaxAutoresponders, 0, sizeof(MaxAutoresponders));
	memset(MaxMailinglists, 0, sizeof(MaxMailinglists));
	memset(GidFlagString, 0, sizeof(GidFlagString));

	memset(PermAccountFlagString, 0, sizeof(PermAccountFlagString));
	memset(PermAliasFlagString, 0, sizeof(PermAliasFlagString));
	memset(PermForwardFlagString, 0, sizeof(PermForwardFlagString));
	memset(PermAutoresponderFlagString, 0, sizeof(PermAutoresponderFlagString));
	memset(PermMaillistFlagString, 0, sizeof(PermMaillistFlagString));
	memset(PermMaillistUsersFlagString, 0, sizeof(PermMaillistUsersFlagString));
	memset(PermMaillistModeratorsFlagString, 0, sizeof(PermMaillistModeratorsFlagString));
	memset(PermQuotaFlagString, 0, sizeof(PermQuotaFlagString));
	memset(PermDefaultQuotaFlagString, 0, sizeof(PermDefaultQuotaFlagString));
	QuotaFlag = 0;
	GidFlag = 0;
	PermAccountFlag = 0;
	PermAliasFlag = 0;
	PermForwardFlag = 0;
	PermAutoresponderFlag = 0;
	PermMaillistFlag = 0;
	PermMaillistUsersFlag = 0;
	PermMaillistModeratorsFlag = 0;
	PermQuotaFlag = 0;
	PermDefaultQuotaFlag = 0;
	domain_expiry = 0;
	passwd_expiry = 0;
	/*- NoMakeIndex = 0; */
	DeleteLimits = 0;
	flag[0] = flag[1] = flag[2] = flag[3] = 0;
	while ((c = getopt(argc, argv, "vst:e:n:N:DQ:q:M:m:P:A:F:R:L:g:p:a:f:r:l:u:o:x:z:h")) != -1)
	{
		switch (c)
		{
		case 'v':
			printf("version: %s\n", VERSION);
			break;
		case 's':
			ShowLimits = 1;
			break;
		case 'e':
			ShowLimits = 0;
			flag[0] = 1;
			if (strncmp(optarg, "-1", 3))
			{
				if (strlen(optarg) != 14 || !strptime(optarg, "%d%m%Y%H%M%S", &tm))
				{
					fprintf(stderr, "Invalid domain expiry date [%s]\n", optarg);
					usage();
					return(1);
				} else
				if ((domain_expiry = mktime(&tm)) == -1)
				{
					fprintf(stderr, "Invalid start date [%s]\n", optarg);
					usage();
					return(1);
				}
			} else
				domain_expiry = -1; /* Disable check on expiry date*/
			break;
		case 'n':
			ShowLimits = 0;
			flag[1] = 1;
			domain_expiry = time(0) + (atol(optarg) * 86400);
			break;
		case 't':
			ShowLimits = 0;
			flag[2] = 1;
			if (strncmp(optarg, "-1", 3))
			{
				if (strlen(optarg) != 14 || !strptime(optarg, "%d%m%Y%H%M%S", &tm))
				{
					fprintf(stderr, "Invalid password expiry date [%s]\n", optarg);
					usage();
					return(1);
				} else
				if ((passwd_expiry = mktime(&tm)) == -1)
				{
					fprintf(stderr, "Invalid start date [%s]\n", optarg);
					usage();
					return(1);
				}
			} else
				passwd_expiry = -1; /* Disable check on expiry date*/
			break;
		case 'N':
			ShowLimits = 0;
			flag[3] = 1;
			passwd_expiry = time(0) + (atol(optarg) * 86400);
			break;
		case 'D':
			ShowLimits = 0;
			DeleteLimits = 1;
			break;
		case 'Q':
			ShowLimits = 0;
			snprintf(DomainQuota, sizeof(DomainQuota), "%s", optarg);
			break;
		case 'q':
			ShowLimits = 0;
			snprintf(DefaultUserQuota, sizeof(DefaultUserQuota), "%s", optarg);
			break;
		case 'M':
			ShowLimits = 0;
			snprintf(DomainMaxMsgCount, sizeof(DomainMaxMsgCount), "%s", optarg);
			break;
		case 'm':
			ShowLimits = 0;
			snprintf(DefaultUserMaxMsgCount, sizeof(DefaultUserMaxMsgCount), "%s", optarg);
			break;
		case 'P':
			ShowLimits = 0;
			snprintf(MaxPopAccounts, sizeof(MaxPopAccounts), "%s", optarg);
			break;
		case 'A':
			ShowLimits = 0;
			snprintf(MaxAliases, sizeof(MaxAliases), "%s", optarg);
			break;
		case 'F':
			ShowLimits = 0;
			snprintf(MaxForwards, sizeof(MaxForwards), "%s", optarg);
			break;
		case 'R':
			ShowLimits = 0;
			snprintf(MaxAutoresponders, sizeof(MaxAutoresponders), "%s", optarg);
			break;
		case 'L':
			ShowLimits = 0;
			snprintf(MaxMailinglists, sizeof(MaxMailinglists), "%s", optarg);
			break;
		case 'g':
			ShowLimits = 0;
			snprintf(GidFlagString, sizeof(GidFlagString), "%s", optarg);
			GidFlag = 1;
			break;
		case 'p':
			ShowLimits = 0;
			snprintf(PermAccountFlagString, sizeof(PermAccountFlagString), "%s", optarg);
			PermAccountFlag = 1;
			break;
		case 'a':
			ShowLimits = 0;
			snprintf(PermAliasFlagString, sizeof(PermAliasFlagString), "%s", optarg);
			PermAliasFlag = 1;
			break;
		case 'f':
			ShowLimits = 0;
			snprintf(PermForwardFlagString, sizeof(PermForwardFlagString), "%s", optarg);
			PermForwardFlag = 1;
			break;
		case 'r':
			ShowLimits = 0;
			snprintf(PermAutoresponderFlagString, sizeof(PermAutoresponderFlagString), "%s", optarg);
			PermAutoresponderFlag = 1;
			break;
		case 'l':
			ShowLimits = 0;
			snprintf(PermMaillistFlagString, sizeof(PermMaillistFlagString), "%s", optarg);
			PermMaillistFlag = 1;
			break;
		case 'u':
			ShowLimits = 0;
			snprintf(PermMaillistUsersFlagString, sizeof(PermMaillistUsersFlagString), "%s", optarg);
			PermMaillistUsersFlag = 1;
			break;
		case 'o':
			ShowLimits = 0;
			snprintf(PermMaillistModeratorsFlagString, sizeof(PermMaillistModeratorsFlagString), "%s", optarg);
			PermMaillistModeratorsFlag = 1;
			break;
		case 'x':
			ShowLimits = 0;
			snprintf(PermQuotaFlagString, sizeof(PermQuotaFlagString), "%s", optarg);
			PermQuotaFlag = 1;
			break;
		case 'z':
			ShowLimits = 0;
			snprintf(PermDefaultQuotaFlagString, sizeof(PermDefaultQuotaFlagString), "%s", optarg);
			PermDefaultQuotaFlag = 1;
			break;
		case 'h':
			ShowLimits = 0;
			usage();
			return (1);
		default:
			ShowLimits = 0;
			break;
		}
	}
	if (flag[0] && flag[1])
	{
		fprintf(stderr, "must supply either Domain expiry date or Number of days\n");
		usage();
		return(1);
	}
	if (flag[2] && flag[3])
	{
		fprintf(stderr, "must supply either Password expiry date or Number of days\n");
		usage();
		return(1);
	}
	if (optind < argc)
	{
		snprintf(Domain, sizeof(Domain), "%s", argv[optind]);
		++optind;
	}
	if (!*Domain)
	{
		usage();
		return (1);
	}
	return (0);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-domain-limits=y\n");
	return(0);
}
#endif

void
getversion_vlimit_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
