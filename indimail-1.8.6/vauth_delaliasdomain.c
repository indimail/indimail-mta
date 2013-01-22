/*
 * $Log: vauth_delaliasdomain.c,v $
 * Revision 2.4  2008-05-28 16:38:52+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2003-12-08 20:29:56+05:30  Cprogrammer
 * create aliasdomain table on master if table does not exist
 *
 * Revision 2.2  2002-08-03 04:34:13+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-05-05 22:20:14+05:30  Cprogrammer
 * use islocalif() to determine if mailstore is local
 *
 * Revision 1.3  2002-03-29 22:10:06+05:30  Cprogrammer
 * compare hostid instead of ip address
 *
 * Revision 1.2  2001-12-22 18:30:28+05:30  Cprogrammer
 * changed error string for opening master db
 *
 * Revision 1.1  2001-12-21 01:12:24+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "indimail.h"
#ifndef	lint
static char     sccsid[] = "$Id: vauth_delaliasdomain.c,v 2.4 2008-05-28 16:38:52+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>
#include <string.h>
#include <errno.h>
int
vauth_delaliasdomain(char *aliasdomain)
{
	char           *ptr, *mailstore;
	char            tmpbuf[MAX_BUFF], SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "vauth_delaliasdomain: Failed to open Master Db\n");
		return (1);
	}
	snprintf(tmpbuf, MAX_BUFF, "postmaster@%s", aliasdomain);
	if ((mailstore = findhost(tmpbuf, 1)) != (char *) 0)
	{
		if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
			*ptr = 0;
		for (; *mailstore && *mailstore != ':'; mailstore++);
		mailstore++;
		if (!islocalif(mailstore))
		{
			fprintf(stderr, "postmaster@%s not local (mailstore %s). Not deleting alias domain\n", 
				aliasdomain, mailstore);
			return (1);
		}
	} else
	{
		fprintf(stderr, "vauth_delaliasdomain: can't figure out postmaster host\n");
		return (1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from aliasdomain where alias=\"%s\"", aliasdomain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT))
				return(-1);
			return(0);
		}
		fprintf(stderr, "vauth_delaliasdomain: mysql error: %s\n", mysql_error(&mysql[0]));
		return (1);
	}
	return (0);
}
#endif

void
getversion_vauth_delaliasdomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
