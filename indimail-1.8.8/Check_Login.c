/*
 * $Log: Check_Login.c,v $
 * Revision 2.1  2002-11-30 21:31:29+05:30  Cprogrammer
 * removed vclose()
 *
 * Revision 1.3  2001-11-24 12:15:20+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:09+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: Check_Login.c,v 2.1 2002-11-30 21:31:29+05:30 Cprogrammer Stab mbhangui $";
#endif

int
Check_Login(service, domain, type)
	const char *service, *domain, *type;
{
	char            TmpBuf1[MAX_BUFF], TmpBuf2[MAX_BUFF];

	snprintf(TmpBuf1, MAX_BUFF, "%s/control/%s/%s/nologin", INDIMAILDIR, domain, service);
	snprintf(TmpBuf2, MAX_BUFF, "%s/control/%s/%s/nologin", INDIMAILDIR, type, service);
	if(!access(TmpBuf1, F_OK) || !access(TmpBuf2, F_OK))
	{
		fprintf(stdout, "Login not permitted for %s\n", service);
		fprintf(stderr, "Login not permitted for %s\n", service);
		exit(1);
	}
	return(0);
}

void
getversion_Check_Login_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
