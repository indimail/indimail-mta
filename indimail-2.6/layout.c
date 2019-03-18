/*
 * $Log: layout.c,v $
 * Revision 2.4  2019-03-16 19:27:04+05:30  Cprogrammer
 * removed mailing list code
 *
 * Revision 2.3  2008-06-13 09:29:38+05:30  Cprogrammer
 * conditional definitions of tables
 *
 * Revision 2.2  2008-05-28 16:36:11+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-02-09 00:44:24+05:30  Cprogrammer
 * function to fetch layout for a indimail table
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: layout.c,v 2.4 2019-03-16 19:27:04+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <string.h>

char           *
layout(char *tablename)
{
	struct layout
	{
		char           *tablename;
		char           *layout;
	};
	static struct layout ptr[] = {
		{"dbinfo", DBINFO_TABLE_LAYOUT},
#ifdef CLUSTERED_SITE
		{"hostcntrl", CNTRL_TABLE_LAYOUT},
		{"host_table", HOST_TABLE_LAYOUT},
		{"smtp_port", SMTP_TABLE_LAYOUT},
		{"aliasdomain", ALIASDOMAIN_TABLE_LAYOUT},
		{"spam", SPAM_TABLE_LAYOUT},
		{"badmailfrom", BADMAILFROM_TABLE_LAYOUT},
		{"badrcptto", BADMAILFROM_TABLE_LAYOUT},
#endif
		{"indimail", SMALL_TABLE_LAYOUT},
		{"indibak", SMALL_TABLE_LAYOUT},
		{"indimail", LARGE_TABLE_LAYOUT},
#if defined(POP_AUTH_OPEN_RELAY)
		{"relay", RELAY_TABLE_LAYOUT},
#endif
#ifdef IP_ALIAS_DOMAINS
		{"ip_alias_map", IP_ALIAS_TABLE_LAYOUT},
#endif
#ifdef ENABLE_AUTH_LOGGING
		{"lastauth", LASTAUTH_TABLE_LAYOUT},
		{"userquota", USERQUOTA_TABLE_LAYOUT},
#endif
		{"dir_control_", DIR_CONTROL_TABLE_LAYOUT},
#ifdef VALIAS
		{"valias", VALIAS_TABLE_LAYOUT},
#endif
		{"mgmtaccess", MGMT_TABLE_LAYOUT},
#ifdef ENABLE_MYSQL_LOGGING
		{"vlog", VLOG_TABLE_LAYOUT},
#endif
		{"bulkmail", BULKMAIL_TABLE_LAYOUT},
		{"fstab", FSTAB_TABLE_LAYOUT},
#ifdef VFILTER
		{"vfilter", FILTER_TABLE_LAYOUT},
#endif
#ifdef ENABLE_DOMAIN_LIMITS
		{"vlimits", LIMITS_TABLE_LAYOUT},
#endif
		{0, 0}
	};
	int             i;

	for (i = 0; ptr[i].tablename; i++)
	{
		if (!strncmp(ptr[i].tablename, tablename, strlen(ptr[i].tablename)))
			return (ptr[i].layout);
	}
	fprintf(stderr, "layout: No layout for %s\n", tablename);
	return((char *) 0);
}

void
getversion_layout_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
