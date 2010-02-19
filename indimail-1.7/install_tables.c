/*
 * $Log: install_tables.c,v $
 * Revision 2.4  2010-01-06 22:28:33+05:30  Cprogrammer
 * fix for non-clustered setup
 *
 * Revision 2.3  2009-01-11 19:54:08+05:30  Cprogrammer
 * disabled mysql_escape()
 *
 * Revision 2.2  2008-06-13 08:57:56+05:30  Cprogrammer
 * compile relay table, hostcntrl table only if POP_AUTH_OPEN_RELAY or CLUSTERED_SITE defined
 *
 * Revision 2.1  2008-05-27 22:38:12+05:30  Cprogrammer
 * utility to install all indimail tables
 *
 */
#include "indimail.h"
#ifdef CLUSTERED_SITE
#include <unistd.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: install_tables.c,v 2.4 2010-01-06 22:28:33+05:30 Cprogrammer Stab mbhangui $";
#endif

int
main(int argc, char **argv)
{
#ifdef POP_AUTH_OPEN_RELAY
	char           *relay_table;
#endif
#ifdef CLUSTERED_SITE
	static char     host_path[MAX_BUFF];
	char           *qmaildir, *controldir;
#endif
	int             i;

	getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
	getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
	disable_mysql_escape(1);
	for (i = 0; i < 2; i++)
	{
		if (create_table(ON_LOCAL, i == 0 ? default_table : inactive_table,
				 site_size == LARGE_SITE ? LARGE_TABLE_LAYOUT : SMALL_TABLE_LAYOUT))
			return (1);
		else
			printf("created table %s on local\n", i == 0 ? default_table : inactive_table);
	}
	for (i = 0; IndiMailTable[i].table_name; i++)
	{
		if (create_table(IndiMailTable[i].which, IndiMailTable[i].table_name, IndiMailTable[i].template))
			return (1);
		else
			printf("created table %s on %s\n", IndiMailTable[i].table_name, IndiMailTable[i].which == ON_LOCAL ? "local" : "master");
	}
#if defined(POP_AUTH_OPEN_RELAY)
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	if (create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT))
		return (1);
	else
		printf("created table %s on local\n", relay_table);
#endif
#ifdef CLUSTERED_SITE
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	if (snprintf(host_path, MAX_BUFF, "%s/%s/host.master", qmaildir, controldir) == -1)
		host_path[MAX_BUFF - 1] = 0;
	getEnvConfigStr(&cntrl_table, "CNTRL_TABLE", CNTRL_DEFAULT_TABLE);
	if (access(host_path, F_OK))
		return (0);
	if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
		return (1);
	else
		printf("created table %s on master\n", cntrl_table);
#endif
	return (0);
}

void
getversion_install_tables_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
