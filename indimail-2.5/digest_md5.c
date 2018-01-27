/*
 * $Log: digest_md5.c,v $
 * Revision 2.3  2011-12-05 14:47:34+05:30  Cprogrammer
 * added version information for global.h
 *
 * Revision 2.2  2011-11-09 19:43:38+05:30  Cprogrammer
 * removed DIGEST_MAX, BUFSIZE definition
 *
 * Revision 2.1  2011-10-28 17:59:47+05:30  Cprogrammer
 * digest_md5() function
 *
 */
#include <string.h>
#include <unistd.h>
#include "global.h"
#include "md5.h"

static char     hextab[] = "0123456789abcdef";

#ifndef	lint
static char     sccsid[] = "$Id: digest_md5.c,v 2.3 2011-12-05 14:47:34+05:30 Cprogrammer Stab mbhangui $";
#endif

int
digest_md5(char *greeting, unsigned char *r_user, unsigned char *r_pass,
	unsigned char *l_pass)
{
	unsigned char   digest[20], encrypted[41];
	int             j;
	char           *e, *nc = 0, *qop = 0, *realm = 0, *nonce = 0, *cnonce = 0,
				   *authzid = 0, *digesturi = 0;
	unsigned char   ea1[33], ea2[33];
	int             len;
	MD5_CTX         md5;
	/*
	 * digest-md5 checking
	 *
	 * r_user=username
	 * r_pass=response  (md5 hash = 32)
	 * r_greet=greeting (nc,qop,realm,nonce,cnonce,digesturi)
	 *
	 *   a1 = md5(user:realm:pass) : nonce : cnonce
	 *   a2 = ...
	 * resp = hex(a1) + nonce + nc + conce + qop + hex(a2)
	 */

	len = strlen(greeting);
	for (j = 0; greeting[j]; j++)	/* terminate strings */
		if (greeting[j] == '\n')
			greeting[j] = 0;
	for (j = 0; j < len; j += strlen(greeting + j) + 1) {
		char           *x = greeting + j;
		if (!strncmp(x, "nc", 2))
			nc = x + 3;
		if (!strncmp(x, "qop", 3))
			qop = x + 4;
		if (!strncmp(x, "realm", 5))
			realm = x + 6;
		if (!strncmp(x, "nonce", 5))
			nonce = x + 6;
		if (!strncmp(x, "cnonce", 6))
			cnonce = x + 7;
		if (!strncmp(x, "authzid", 7))
			authzid = x + 8;
		if (!strncmp(x, "digest-uri", 10))
			digesturi = x + 11;
	}
	if (!nc || !qop || !realm || !nonce || !cnonce || !digesturi || !r_pass)
		return (-1);
	MD5Init(&md5);
	MD5Update(&md5, r_user, strlen((char *) r_user));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) realm, strlen(realm));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, l_pass, strlen((char *) l_pass));
	MD5Final(digest, &md5);

	MD5Init(&md5);
	MD5Update(&md5, digest, 16);	/* md5(user+realm+pass) */
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) nonce, strlen(nonce));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) cnonce, strlen(cnonce));
	if (authzid) {			/* warning: not tested with authzid! */
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) authzid, strlen(authzid));
	}
	MD5Final(digest, &md5);
	for (e = (char *) ea1, j = 0; j < 16; j++) {
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = 0;					/* ea1 = ready */

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *) "AUTHENTICATE:", 13);
	MD5Update(&md5, (unsigned char *) digesturi, strlen(digesturi));
	if (strlen(qop) > 4)	/* for auth-* type */
		MD5Update(&md5, (unsigned char *) ":00000000000000000000000000000000", 32);
	MD5Final(digest, &md5);
	for (e = (char *) ea2, j = 0; j < 16; j++) {
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = 0;					/* ea2 = ready */

	/*
	 * resp = hex(a1) + nonce + nc + cnonce + qop + hex(a2) 
	 */
	MD5Init(&md5);
	MD5Update(&md5, ea1, 32);
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) nonce, strlen(nonce));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) nc, strlen(nc));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) cnonce, strlen(cnonce));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) qop, strlen(qop));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, ea2, 32);
	MD5Final(digest, &md5);
	for (e = (char *) encrypted, j = 0; j < 16; j++) {
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = 0;

	if (!strcmp((char *) r_pass, (char *) encrypted)) {
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) digesturi, strlen(digesturi));
		MD5Final(digest, &md5);
		for (e = (char *) ea2, j = 0; j < 16; j++) {
			*e = hextab[digest[j] / 16];
			++e;
			*e = hextab[digest[j] % 16];
			++e;
		}
		*e = 0;				/* ea2 = ready */

		/*
		 * rspauth = hex(a1) + nonce + nc + conce + qop + hex(a2) 
		 */
		MD5Init(&md5);
		MD5Update(&md5, ea1, 32);
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) nonce, strlen(nonce));
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) nc, strlen(nc));
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) cnonce, strlen(cnonce));
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) qop, strlen(qop));
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, ea2, 32);
		MD5Final(digest, &md5);
		for (e = (char *) encrypted, j = 0; j < 16; j++) {
			*e = hextab[digest[j] / 16];
			++e;
			*e = hextab[digest[j] % 16];
			++e;
		}
		*e = 0;
		if ((len = write(6, encrypted, 32)) == -1)
			return (-1);
		return (0);
	}	/* if correct password */
	return (1);
}

#include <stdio.h>
void
getversion_digest_md5_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidglobalh);
	printf("%s\n", sccsidmd5h);
}
