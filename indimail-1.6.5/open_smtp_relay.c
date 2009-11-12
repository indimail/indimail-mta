/*
 * $Log: open_smtp_relay.c,v $
 * Revision 2.4  2008-05-28 21:55:50+05:30  Cprogrammer
 * removed ldap,cdb code
 *
 * Revision 2.3  2008-05-28 15:13:16+05:30  Cprogrammer
 * removed CDB module
 *
 * Revision 2.2  2005-12-29 22:47:01+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2002-08-25 22:49:01+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 1.10  2002-04-04 16:38:59+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.9  2002-04-01 02:10:59+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.8  2002-03-31 21:49:03+05:30  Cprogrammer
 * RemoveLock() after releasing lock
 *
 * Revision 1.7  2002-03-27 01:52:05+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.6  2002-03-25 00:35:17+05:30  Cprogrammer
 * added semaphore locking code
 *
 * Revision 1.5  2002-03-24 19:12:40+05:30  Cprogrammer
 * additinal argument added to get_write_lock()
 *
 * Revision 1.4  2002-03-03 15:39:43+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.3  2001-11-24 12:19:46+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:41+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: open_smtp_relay.c,v 2.4 2008-05-28 21:55:50+05:30 Cprogrammer Stab mbhangui $";
#endif

int
open_smtp_relay(char *user, char *domain)
{
#ifdef POP_AUTH_OPEN_RELAY
	char           TmpBuf1[MAX_BUFF];
	char           *mcdfile, *qmaildir, *controldir;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if(!(mcdfile = (char *) getenv("MCDFILE")))
		mcdfile = MCDFILE;
	if(*mcdfile == '/')
		scopy(TmpBuf1, mcdfile, MAX_BUFF);
	else
		snprintf(TmpBuf1, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
	if(vopen_smtp_relay(user, domain) && access(TmpBuf1, F_OK))
		update_rules(1);
#endif /*- #ifdef POP_AUTH_OPEN_RELAY */
	return (0);
}

void
getversion_open_smtp_relay_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
