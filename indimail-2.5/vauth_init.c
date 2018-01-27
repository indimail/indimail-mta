/*
 * $Log: vauth_init.c,v $
 * Revision 2.4  2008-05-28 16:39:25+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2005-12-29 22:51:36+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2002-10-12 23:04:42+05:30  Cprogrammer
 * conditional compilation of clustered code
 *
 * Revision 2.1  2002-08-30 23:30:18+05:30  Cprogrammer
 * set isopen_vauthinit for hostcntrl and local mysql connects
 *
 * Revision 1.1  2002-04-07 13:42:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: vauth_init.c,v 2.4 2008-05-28 16:39:25+05:30 Cprogrammer Stab mbhangui $";
#endif

/*- NOTE: Not safe to be called on a socket by multiple processes simultaneously */
void
vauth_init(int which, MYSQL *mysqlStruct)
{
	mysql[which]=*mysqlStruct;
	/* adjust connection pointers */
	mysql[which].net.last_error[0]=0;
	mysql[which].net.last_errno=0;
	mysql[which].info=0;
	mysql[which].affected_rows= ~(my_ulonglong) 0;
	switch(which)
	{
#ifdef CLUSTERED_SITE
		case 0:
			isopen_cntrl = 1;
			isopen_vauthinit[0] = 1;
			getEnvConfigStr(&cntrl_table, "CNTRL_TABLE", CNTRL_DEFAULT_TABLE);
			break;
#endif /*- #ifdef CLUSTERED_SITE */
		case 1:
			is_open = 1; /*- prevent vauth_open() from opening a new connection */
#ifdef CLUSTERED_SITE
			isopen_vauthinit[1] = 1;
#endif /*- #ifdef CLUSTERED_SITE */
			getEnvConfigStr(&default_table, "MYSQL_TABLE", MYSQL_DEFAULT_TABLE);
			getEnvConfigStr(&inactive_table, "MYSQL_INACTIVE_TABLE", MYSQL_INACTIVE_TABLE);
			break;
	}
	return;
}

void
getversion_vauth_init_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
