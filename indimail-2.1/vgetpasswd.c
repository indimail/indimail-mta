/*
 * $Log: vgetpasswd.c,v $
 * Revision 2.1  2008-06-25 15:40:16+05:30  Cprogrammer
 * removed sstrncmp
 *
 * Revision 1.5  2001-12-03 00:09:23+05:30  Cprogrammer
 * replaced ifndef linux with ifdef sun
 *
 * Revision 1.4  2001-11-24 12:22:02+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 21:55:41+05:30  Cprogrammer
 * prototype definition for getpass() for non linux based OS
 *
 * Revision 1.2  2001-11-20 11:00:52+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: vgetpasswd.c,v 2.1 2008-06-25 15:40:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef sun
char           *getpass(char *);
#endif
/*
 * prompt the command line and get a password twice, that matches 
 */
char           *
vgetpasswd(user)
	char           *user;
{
	static char     pass1[MAX_BUFF], pass2[MAX_BUFF], tmpstr[MAX_BUFF];

	snprintf(tmpstr, MAX_BUFF, "New IndiMail password for %s: ", user);
	while (1)
	{
		scopy(pass1, getpass(tmpstr), MAX_BUFF);
		scopy(pass2, getpass("Retype new IndiMail password: "), MAX_BUFF);
		if (strncmp(pass1, pass2, MAX_BUFF))
			fprintf(stderr, "Passwords do not match, try again\n");
		else
			break;
	}
	return (pass1);
}

void
getversion_vgetpasswd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
