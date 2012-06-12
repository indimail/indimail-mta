/*
 * $Log: pw_comp.c,v $
 * Revision 2.15  2012-06-12 15:36:13+05:30  Cprogrammer
 * added DISABLE_CRYPT to disable crypt method
 *
 * Revision 2.14  2011-12-22 11:59:27+05:30  Cprogrammer
 * moved AUTH methods defines to indimail.h
 *
 * Revision 2.13  2011-12-10 15:18:37+05:30  Cprogrammer
 * added hmac_sha256()
 *
 * Revision 2.12  2011-11-13 15:27:20+05:30  Cprogrammer
 * fixed password comparision for TRIVIAL_PASSWORD case
 *
 * Revision 2.11  2011-10-28 17:58:38+05:30  Cprogrammer
 * added AUTH CRAM-SHA1, CRAM-RIPEMD, DIGEST-MD5
 *
 * Revision 2.10  2011-10-25 20:49:23+05:30  Cprogrammer
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.9  2011-10-25 10:47:51+05:30  Cprogrammer
 * added trivial_password option to authenticate when using cram-md5
 *
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
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: pw_comp.c,v 2.15 2012-06-12 15:36:13+05:30 Cprogrammer Stab mbhangui $";
#endif

static char     hextab[] = "0123456789abcdef";


int
pw_comp(unsigned char *testlogin, unsigned char *localpw, unsigned char *challenge,
	unsigned char *response, int auth_method)
{
	unsigned char   digest[21], digascii[41];
	char            Crypted[MAX_BUFF];
	char           *crypt_pass, *e;
	int             i, len;

	if (!getenv("DISABLE_CRYPT") && (!challenge || (challenge && !*challenge)))
	{
		if (!(crypt_pass = in_crypt((char *) response, (char *) localpw)))
		{
			printf("454-CRYPT: %s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
		len = strlen(crypt_pass);
		i = strncmp((const char *) crypt_pass, (const char *) localpw, (size_t) len + 1);
		/*- non CRAM-MD5 aware app */
		if (i && getenv("TRIVIAL_PASSWORDS"))
		{
			mkpasswd3((char *) localpw, Crypted, MAX_BUFF);
			if (!(crypt_pass = in_crypt((char *) response, (char *) Crypted)))
			{
				printf("454-CRYPT: %s (#4.3.0)\r\n", strerror(errno));
				fflush(stdout);
				_exit (111);
			}
			len = strlen(crypt_pass);
			i = strncmp((const char *) crypt_pass, (const char *) Crypted, (size_t) len + 1);
		}
		return (i);
	}
	if ((!auth_method && !getenv("DISABLE_CRAM_MD5")) || auth_method == AUTH_CRAM_MD5) {
		hmac_md5(challenge, (int) strlen((const char *) challenge), localpw,
			(int) strlen((const char *) localpw), digest);
		digascii[32] = 0;
		for (i=0, e = (char *) digascii; i<16; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e=0;
		if (!(i = strcmp((const char *) digascii, (const char *) response)))
			return (i);
	}
	if ((!auth_method && !getenv("DISABLE_CRAM_SHA1")) || auth_method == AUTH_CRAM_SHA1) {
		hmac_sha1(challenge, (int) strlen((const char *) challenge), localpw,
			(int) strlen((const char *) localpw), digest);
		digascii[40] = 0;
		for (i=0, e = (char *) digascii; i<20; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e = 0;
		if (!(i = strcmp((const char *) digascii, (const char *) response)))
			return (i);
	}
	if ((!auth_method && !getenv("DISABLE_CRAM_SHA256")) || auth_method == AUTH_CRAM_SHA256) {
		hmac_sha256(challenge, (int) strlen((const char *) challenge), localpw,
			(int) strlen((const char *) localpw), digest);
		digascii[40] = 0;
		for (i=0, e = (char *) digascii; i<20; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e = 0;
		if (!(i = strcmp((const char *) digascii, (const char *) response)))
			return (i);
	}
	if ((!auth_method && !getenv("DISABLE_CRAM_RIPEMD")) || auth_method == AUTH_CRAM_RIPEMD) {
		hmac_ripemd(challenge, (int) strlen((const char *) challenge), localpw,
			(int) strlen((const char *) localpw), digest);
		digascii[40] = 0;
		for (i=0, e = (char *) digascii; i<20; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e = 0;
		if (!(i = strcmp((const char *) digascii, (const char *) response)))
			return (i);
	}
	if ((!auth_method && !getenv("DISABLE_DIGEST_MD5")) || auth_method == AUTH_DIGEST_MD5) {
		if ((i = digest_md5((char *) response, testlogin, challenge, localpw)) == -1)
		{
			printf("454-DIGEST-MD5: %s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		} else
		if (!i)
			return (i);
	}
	return (1);
}

void
getversion_pw_comp_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
