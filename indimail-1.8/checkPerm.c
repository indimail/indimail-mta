/*
 * $Log: checkPerm.c,v $
 * Revision 2.4  2008-06-13 08:35:48+05:30  Cprogrammer
 * compile if CLUSTERED_SITE is enabled
 *
 * Revision 2.3  2008-05-28 16:33:42+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-09-16 12:28:14+05:30  Cprogrammer
 * check for existence of user
 *
 * Revision 2.1  2003-09-14 01:59:58+05:30  Cprogrammer
 * function to verify privileges
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: checkPerm.c,v 2.4 2008-06-13 08:35:48+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>

int
checkPerm(char *user, char  *program, char **argv)
{
	char           *ptr1, *ptr2, *tuser;
	char          **Ptr;

	if(!user || !*user || !program || !*program || !argv || !*argv)
		return(1);
	if(mgmtpassinfo(user, 0) && userNotFound)
		return(1);
	tuser = user;
	ptr2 = program;
	for(;;)
	{
		if(!(ptr1 = vpriv_select(&tuser, &ptr2)))
			break;
		if(!strncmp(ptr1, "*", 2))
			return(0);
		for(Ptr = argv;Ptr && *Ptr;Ptr++)
		{
			if(!strstr(ptr1, *Ptr))
				return(1);
		}
		return(0);
	}
	return(1);
}
#endif

void
getversion_checkPerm_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
