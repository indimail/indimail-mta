/*
 * $Log: getAddressBook.c,v $
 * Revision 2.2  2008-05-28 16:35:31+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2003-04-02 01:30:37+05:30  Cprogrammer
 * dummy version
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getAddressBook.c,v 2.2 2008-05-28 16:35:31+05:30 Cprogrammer Stab mbhangui $";
#endif

char  **
getAddressBook(char *emailid)
{
	return((char **) 0);
}

void
getversion_getAddressBook_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
