/*
 * $Log: qr_digest_md5.c,v $
 * Revision 1.2  2011-12-05 15:07:24+05:30  Cprogrammer
 * added version information
 *
 * Revision 1.1  2011-10-29 20:42:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "global.h"
#include "md5.h"
#include "str.h"

static char     hextab[] = "0123456789abcdef";

char *
qr_digest_md5(char *user, int ulen, char *realm, int rlen, char *pass, int plen,
	char *authzid, char *nonce, int nlen, char *digesturi, int dlen,
	char *cnonce, const char *nc, const char *qop)
{
	unsigned char   digest[20], ea1[33], ea2[33];
	static char     encrypted[41];
	unsigned char  *e;
	int             j;
	MD5_CTX         md5;

	if (!ulen || !user || !*user)
		return ((char *) 0);
	if (!rlen || !realm || !*realm)
		return ((char *) 0);
	if (!plen || !pass || !*pass)
		return ((char *) 0);
	if (!nlen || !nonce || !*nonce)
		return ((char *) 0);
	if (!dlen || !digesturi || !*digesturi)
		return ((char *) 0);
	if (!cnonce || !nc || !qop)
		return ((char *) 0);
	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *) user, ulen);
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) realm, rlen);
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) pass, plen);
	MD5Final(digest, &md5);

	MD5Init(&md5);
	MD5Update(&md5, digest, 16);	/* md5(user+realm+pass) */
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) nonce, nlen);
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) cnonce, 32);
	if (authzid) {			/* warning: not tested with authzid! */
		MD5Update(&md5, (unsigned char *) ":", 1);
		MD5Update(&md5, (unsigned char *) authzid, strlen(authzid));
	}
	MD5Final(digest, &md5);
	for (e = ea1, j = 0; j < 16; j++) {
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = 0;					/* ea1 = ready */

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *) "AUTHENTICATE:", 13);
	MD5Update(&md5, (unsigned char *) digesturi, dlen);
	if (strlen(qop) > 4)	/* for auth-* type */
		MD5Update(&md5, (unsigned char *) ":00000000000000000000000000000000", 32);
	MD5Final(digest, &md5);
	for (e = ea2, j = 0; j < 16; j++) {
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
	MD5Update(&md5, (unsigned char *) nonce, nlen);
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) nc, str_len(nc));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) cnonce, 32);
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, (unsigned char *) qop, str_len(qop));
	MD5Update(&md5, (unsigned char *) ":", 1);
	MD5Update(&md5, ea2, 32);
	MD5Final(digest, &md5);
	for (e = (unsigned char *) encrypted, j = 0; j < 16; j++) {
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = 0;
	return (encrypted);
}

void
getversion_qr_digest_md5_c()
{
	const char     *x = "$Id: qr_digest_md5.c,v 1.2 2011-12-05 15:07:24+05:30 Cprogrammer Stab mbhangui $";
	x=sccsidmd5h;
	x=sccsidglobalh;
	x++;
}
