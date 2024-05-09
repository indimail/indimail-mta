/*
 * $Id: authcram.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <strerr.h>
#include <authmethods.h>
#include <sgetopt.h>
#include <base64.h>
#include <case.h>
#include <hmac.h>
#include <openssl/ripemd.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define FATAL "authcram: fatal: "

static stralloc slop = { 0 };
static stralloc auth = { 0 };
static stralloc chal = { 0 };
static stralloc user = { 0 };
static stralloc pass = { 0 };
static char     hextab[] = "0123456789abcdef";

void
die_nomem()
{
	substdio_puts(subfderr, "out of memory\n");
	_exit(111);
}

const char     *usage =
		"usage: cramresp -t AUTH_TYPE user password challenge\n"
		"    AUTH_TYPE    One of CRAM-MD5, CRAM-RIPEMD, CRAM-SHA1, CRAM-SHA224,\n"
		"                 CRAM-SHA384, CRAM-SHA512\n"
		"    user         username\n"
		"    password     password\n"
		"    challenge    challenge";

int
main(int argc, char **argv)
{
	int             j, opt, type = -1, iter = -1;
	unsigned char   digest[129], encrypted[65];
	unsigned char  *e;

	while ((opt = getopt(argc, argv, "t:")) != opteof) {
		switch (opt)
		{
		case 't':
			if (!case_diffs(optarg, "CRAM-MD5"))
				type = AUTH_CRAM_MD5;
			else
			if (!case_diffs(optarg, "CRAM-RIPEMD"))
				type = AUTH_CRAM_RIPEMD;
			else
			if (!case_diffs(optarg, "CRAM-SHA1"))
				type = AUTH_CRAM_SHA1;
			else
			if (!case_diffs(optarg, "CRAM-SHA224"))
				type = AUTH_CRAM_SHA224;
			else
			if (!case_diffs(optarg, "CRAM-SHA256"))
				type = AUTH_CRAM_SHA256;
			else
			if (!case_diffs(optarg, "CRAM-SHA384"))
				type = AUTH_CRAM_SHA384;
			else
			if (!case_diffs(optarg, "CRAM-SHA512"))
				type = AUTH_CRAM_SHA512;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (optind + 3 != argc || type == -1)
		strerr_die1x(100, usage);
	if (!stralloc_copys(&user, argv[optind++]))
		die_nomem();
	if (!stralloc_copys(&pass, argv[optind++]))
		die_nomem();
	if (!stralloc_copys(&slop, argv[optind++]))
		die_nomem();
	if (b64decode((unsigned char *) slop.s, slop.len, &chal))
		strerr_die2x(111, FATAL, "unable to base64decode challenge");
	switch (type)
	{
	case AUTH_CRAM_MD5:
		iter = MD5_DIGEST_LENGTH;
		hmac_md5((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_RIPEMD:
		iter = RIPEMD160_DIGEST_LENGTH;
		hmac_ripemd((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA1:
		iter = SHA_DIGEST_LENGTH;
		hmac_sha1((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA224:
		iter = SHA224_DIGEST_LENGTH;
		hmac_sha224((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA256:
		iter = SHA256_DIGEST_LENGTH;
		hmac_sha256((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA384:
		iter = SHA384_DIGEST_LENGTH;
		hmac_sha384((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	case AUTH_CRAM_SHA512:
		iter = SHA512_DIGEST_LENGTH;
		hmac_sha512((unsigned char *) chal.s, chal.len, (unsigned char *) pass.s, pass.len, digest);
		break;
	}

	for (j = 0, e = encrypted; j < iter; j++) {	/*- HEX => ASCII */
		*e = hextab[digest[j] / 16];
		++e;
		*e = hextab[digest[j] % 16];
		++e;
	}
	*e = '\0';

	/*- copy user-id and digest */
	if (!stralloc_copy(&slop, &user) ||
			!stralloc_catb(&slop, " ", 1) ||
			!stralloc_cats(&slop, (char *) encrypted))
		die_nomem();

	if (b64encode(&slop, &auth))
		strerr_die2x(111, FATAL, "unable to base64encode username+digest");
	if (substdio_put(subfdout, auth.s, auth.len) == -1 ||
			substdio_put(subfdout, "\r\n", 2) == -1)
		strerr_die2sys(111, FATAL, "unable to write to descriptor 1");
	substdio_flush(subfdout);
	_exit(0);
}

/*
 * $Log: authcram.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2024-02-18 17:03:00+05:30  Cprogrammer
 * Initial revision
 *
 */
