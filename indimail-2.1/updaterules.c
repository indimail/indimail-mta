/*
 * $Log: updaterules.c,v $
 * Revision 2.2  2008-06-13 10:17:13+05:30  Cprogrammer
 * fix compilation error if POP_AUTH_OPEN_RELAY was not defined
 *
 * Revision 2.1  2008-05-28 16:38:04+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 1.4  2001-11-24 12:20:17+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:13+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:24:24+05:30  Cprogrammer
 * vauth_open for distributed arch
 *
 * Revision 1.1  2001-10-24 18:15:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: updaterules.c,v 2.2 2008-06-13 10:17:13+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#include <stdio.h>

int
main()
{
	if (vauth_open((char *) 0))
		return(1);
	if (update_rules(1))
	{
		vclose();
		return(1);
	}
	vclose();
	return(0);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-roaming-users=y\n");
	return(1);
}
#endif

void
getversion_updaterules_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
