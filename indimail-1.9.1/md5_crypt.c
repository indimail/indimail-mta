/*
 * $Log: md5_crypt.c,v $
 * Revision 2.2  2011-12-05 14:48:18+05:30  Cprogrammer
 * added version information for global.h
 *
 * Revision 2.1  2011-12-05 13:43:48+05:30  Cprogrammer
 * md5_crypt() function for systems with missing md5_crypt()
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: md5_crypt.c,v 2.2 2011-12-05 14:48:18+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef HAVE_MD5_CRYPT
#include "global.h"
#include "md5.h"
static void
to64(char *s, unsigned long v, int n)
{
	/* 0 ... 63 => ascii - 64 */
	unsigned char itoa64[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	while (--n >= 0) {
		*s++ = itoa64[v&0x3f];
		v >>= 6;
	}
}

/*
 * UNIX password
 *
 * Use MD5 for what it is best at...
 */
char           *
md5_crypt(const char *pw, const char *salt)
{
	/*
	 * This string is magic for this algorithm.
	 * Having it this way, we can get get better later on
	 */
	static char    *magic = "$1$";
	static char     passwd[120], *p;
	static const char *sp, *ep;
	unsigned char   final[16];
	int             sl, pl, i, j;
	MD5_CTX         ctx, ctx1;
	unsigned long   l;

	/*- Refine the Salt first */
	sp = salt;
	/*- If it starts with the magic string, then skip that */
	if (!strncmp(sp, magic, strlen(magic)))
		sp += strlen(magic);

	/*- It stops at the first '$', max 8 chars */
	for (ep = sp; *ep && *ep != '$' && ep < (sp + 8); ep++)
		continue;
	/*- get the length of the true salt */
	sl = ep - sp;

	MD5Init(&ctx);

	/*- The password first, since that is what is most unknown */
	MD5Update(&ctx, (unsigned char *) pw, strlen(pw));

	/*- Then our magic string */
	MD5Update(&ctx, (unsigned char *) magic, strlen(magic));

	/*- Then the raw salt */
	MD5Update(&ctx, (unsigned char *) sp, sl);

	/*- Then just as many characters of the MD5(pw,salt,pw) */
	MD5Init(&ctx1);
	MD5Update(&ctx1, (unsigned char *) pw, strlen(pw));
	MD5Update(&ctx1, (unsigned char *) sp, sl);
	MD5Update(&ctx1, (unsigned char *) pw, strlen(pw));
	MD5Final(final, &ctx1);
	for (pl = strlen(pw); pl > 0; pl -= 16)
		MD5Update(&ctx, final, pl > 16 ? 16 : pl);

	/*- Don't leave anything around in vm they could use.  */
	memset(final, 0, sizeof final);

	/*- Then something really weird...  */
	for (j = 0, i = strlen(pw); i; i >>= 1)
		if (i & 1)
			MD5Update(&ctx, (unsigned char *) (final + j), 1);
		else
			MD5Update(&ctx, (unsigned char *) (pw + j), 1);

	/*- Now make the output string */
	strcpy(passwd, magic);
	strncat(passwd, sp, sl);
	strcat(passwd, "$");

	MD5Final(final, &ctx);

	/*
	 * and now, just to make sure things don't run too fast
	 * On a 60 Mhz Pentium this takes 34 msec, so you would
	 * need 30 seconds to build a 1000 entry dictionary...
	 */
	for (i = 0; i < 1000; i++)
	{
		MD5Init(&ctx1);
		if (i & 1)
			MD5Update(&ctx1, (unsigned char *) pw, strlen(pw));
		else
			MD5Update(&ctx1, final, 16);

		if (i % 3)
			MD5Update(&ctx1, (unsigned char *) sp, sl);

		if (i % 7)
			MD5Update(&ctx1, (unsigned char *) pw, strlen(pw));

		if (i & 1)
			MD5Update(&ctx1, final, 16);
		else
			MD5Update(&ctx1, (unsigned char *) pw, strlen(pw));
		MD5Final(final, &ctx1);
	}

	p = passwd + strlen(passwd);

	l = (final[0] << 16) | (final[6] << 8) | final[12];
	to64(p, l, 4);
	p += 4;
	l = (final[1] << 16) | (final[7] << 8) | final[13];
	to64(p, l, 4);
	p += 4;
	l = (final[2] << 16) | (final[8] << 8) | final[14];
	to64(p, l, 4);
	p += 4;
	l = (final[3] << 16) | (final[9] << 8) | final[15];
	to64(p, l, 4);
	p += 4;
	l = (final[4] << 16) | (final[10] << 8) | final[5];
	to64(p, l, 4);
	p += 4;
	l = final[11];
	to64(p, l, 2);
	p += 2;
	*p = '\0';

	/*- Don't leave anything around in vm they could use.  */
	memset(final, 0, sizeof final);

	return passwd;
}
#endif

#include <stdio.h>
void
getversion_md5_crypt_c()
{
	printf("%s\n", sccsid);
#ifndef HAVE_MD5_CRYPT
	printf("%s\n", sccsidglobalh);
	printf("%s\n", sccsidmd5h);
#endif
}
