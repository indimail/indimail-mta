/*
 * $Log: pw_comp.c,v $
 * Revision 2.8  2008-09-12 09:57:54+05:30  Cprogrammer
 * use in_crypt() replacement
 *
 * Revision 2.7  2008-09-08 09:51:15+05:30  Cprogrammer
 * removed hmac_md5()
 *
 * Revision 2.6  2008-08-29 14:03:34+05:30  Cprogrammer
 * compare password lenght using length of crypted passwd returned by crypt ()
 *
 * Revision 2.5  2008-07-13 19:46:32+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.4  2005-12-21 09:47:51+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.3  2002-08-31 14:28:18+05:30  Cprogrammer
 * corrected error for multiline smtp output
 *
 * Revision 2.2  2002-07-02 12:20:40+05:30  Cprogrammer
 * replaced bzero with memset, bcopy with memcpy
 *
 * Revision 2.1  2002-05-11 00:21:43+05:30  Cprogrammer
 * implmented CRAM-MD5 authentication
 *
 */
#include "indimail.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: pw_comp.c,v 2.8 2008-09-12 09:57:54+05:30 Cprogrammer Stab mbhangui $";
#endif

static char     hextab[] = "0123456789abcdef";

int
pw_comp(unsigned char *testlogin, unsigned char *password, unsigned char *challenge, unsigned char *response)
{
	unsigned char   digest[16];
	unsigned char   digascii[33];
	char           *crypt_pass;
	unsigned char   h;
	int             j, len;

	if(!response || (response && !*response))
	{
		if (!(crypt_pass = in_crypt((char *) challenge, (char *) password)))
		{
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
		len = strlen(crypt_pass);
		return(strncmp((const char *) crypt_pass, (const char *) password, (size_t) len + 1));
	}
	hmac_md5(challenge, (int) strlen((const char *) challenge), password,
		(int) strlen((const char *) password), digest);
	digascii[32] = 0;
	for (j = 0; j < 16; j++)
	{
		h = digest[j] >> 4;
		digascii[2 * j] = hextab[h];
		h = digest[j] & 0x0f;
		digascii[(2 * j) + 1] = hextab[h];
	}
	return (strcmp((const char *) digascii, (const char *) response) && strcmp((const char *) password, (const char *) challenge));
}

void
getversion_pw_comp_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
