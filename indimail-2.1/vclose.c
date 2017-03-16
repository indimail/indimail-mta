/*
 * $Log: vclose.c,v $
 * Revision 2.6  2008-11-13 19:28:12+05:30  Cprogrammer
 * fixed problem with vclose when called twice
 *
 * Revision 2.5  2008-05-28 15:50:08+05:30  Cprogrammer
 * removed ldap, cdb module
 *
 * Revision 2.4  2002-10-12 23:05:22+05:30  Cprogrammer
 * moved clustered code within the correct #ifdef location for clustered code
 *
 * Revision 2.3  2002-08-30 23:30:41+05:30  Cprogrammer
 * use isopen_vauthinit to determine whether mysql_close should be called or not
 *
 * Revision 2.2  2002-07-04 00:42:57+05:30  Cprogrammer
 * close ldap connection if USE_LDAP_PASSWD is defined
 *
 * Revision 2.1  2002-06-26 03:21:06+05:30  Cprogrammer
 * added function ldapClose()
 *
 * Revision 1.7  2001-12-23 00:48:21+05:30  Cprogrammer
 * initialize mysql_host and cntrl_host on vclose
 *
 * Revision 1.6  2001-12-11 11:34:29+05:30  Cprogrammer
 * removed open_relay_db()
 *
 * Revision 1.5  2001-11-29 20:58:41+05:30  Cprogrammer
 * code change for closing multiple mysql connections
 *
 * Revision 1.4  2001-11-24 12:21:27+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:59:57+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:27:39+05:30  Cprogrammer
 * vclose() to close both mysql databases
 *
 * Revision 1.1  2001-10-24 18:15:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vclose.c,v 2.6 2008-11-13 19:28:12+05:30 Cprogrammer Stab mbhangui $";
#endif

void
vclose()
{
	/*
	 * disconnection from the database 
	 */
#ifdef CLUSTERED_SITE
	if (isopen_vauthinit[0] || isopen_vauthinit[1])
	{
		if (is_open)
			*mysql_host = 0;
		if (isopen_cntrl)
			*cntrl_host = 0;
		close_db();
		if (isopen_cntrl == 1 && !isopen_vauthinit[0])
			mysql_close(&mysql[0]);
		if (is_open == 1 && !isopen_vauthinit[1])
			mysql_close(&mysql[1]);
		isopen_vauthinit[0] = isopen_vauthinit[1] = 0;
		is_open = isopen_cntrl = 0;
		return;
	}
#endif
	if (is_open)
		*mysql_host = 0;
	if (is_open == 1)
	{
		is_open = 0;
		mysql_close(&mysql[1]);
	} 
#ifdef CLUSTERED_SITE
	else
	if (is_open == 2)
		is_open = 0;
	if (isopen_cntrl)
		*cntrl_host = 0;
	if (isopen_cntrl == 1)
	{
		isopen_cntrl = 0;
		mysql_close(&mysql[0]);
	} else
	if (isopen_cntrl == 2)
		isopen_cntrl = 0;
#endif
}

void
getversion_vclose_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
