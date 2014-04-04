/*
 * $Log: mkpasswd3.c,v $
 * Revision 2.6  2008-09-12 09:57:46+05:30  Cprogrammer
 * use in_crypt() replacement
 *
 * Revision 2.5  2008-09-11 22:47:19+05:30  Cprogrammer
 * return error if length of passwd exceeds length
 *
 * Revision 2.4  2008-08-29 14:02:43+05:30  Cprogrammer
 * new function makesalt() to generate salt
 *
 * Revision 2.3  2008-07-13 19:45:22+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.2  2003-10-23 13:20:34+05:30  Cprogrammer
 * added better seeding for generation of salt
 *
 * Revision 2.1  2002-05-14 02:28:05+05:30  Cprogrammer
 * added MD5_PASSWORDS code
 *
 * Revision 1.3  2001-11-24 12:19:40+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:36+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: mkpasswd3.c,v 2.6 2008-09-12 09:57:46+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * encrypt a password 
 */
int
mkpasswd3(newpasswd, crypted, ssize)
	char           *newpasswd;
	char           *crypted;
	int             ssize;
{
	char           *tmpstr;
	char            salt[SALTSIZE + 1];
	int             len;
	static int      seeded;

	*crypted = 0;
	if (encrypt_flag)
		tmpstr = newpasswd;
	else
	{
		if (!seeded)
		{
			seeded = 1;
			srandom(time(0)^(getpid()<<15));
		}
		makesalt(salt, SALTSIZE);
		if(!(tmpstr = in_crypt(newpasswd, salt)))
			return (-1);
	}
	len = strlen(tmpstr);
	if (len > (ssize - 1))
		return (-1);
	scopy(crypted, tmpstr, ssize);
	return (0);
}

void
getversion_mkpasswd3_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
