/*
 * $Log: batv.c,v $
 * Revision 1.11  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.10  2022-10-13 23:58:43+05:30  Cprogrammer
 * added link to batv draft used by indimail-mta
 *
 * Revision 1.9  2022-10-13 20:29:09+05:30  Cprogrammer
 * use definitions from batv.h
 *
 * Revision 1.8  2022-05-18 13:29:25+05:30  Cprogrammer
 * openssl 3.0.0 port
 *
 * Revision 1.7  2022-03-20 16:34:23+05:30  Cprogrammer
 * refactored batv code
 *
 * Revision 1.7  2021-09-21 13:32:31+05:30  Cprogrammer
 * refactored batv code
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2015-08-20 18:34:28+05:30  Cprogrammer
 * added usage for invalid option
 *
 * Revision 1.4  2015-08-19 16:26:16+05:30  Cprogrammer
 * exit 111 for ENOMEM
 *
 * Revision 1.3  2015-07-23 13:51:00+05:30  Cprogrammer
 * added option to pass key validity period in days
 * fixed bug with passing email address without @ sign
 *
 * Revision 1.2  2009-12-07 08:21:02+05:30  Cprogrammer
 * corrected usage
 *
 * Revision 1.1  2009-09-01 22:23:19+05:30  Cprogrammer
 * Initial revision
 *
 * https://datatracker.ietf.org/doc/html/draft-levine-mass-batv-02
 *
 */
#ifdef BATV
#include <unistd.h>
#include <ctype.h>
#include <openssl/ssl.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/evp.h>
#else
#include <openssl/md5.h>
#endif
#include <sgetopt.h>
#include <stralloc.h>
#include <now.h>
#include <env.h>
#include <str.h>
#include <scan.h>
#include <constmap.h>
#include <strerr.h>
#include <subfd.h>
#include <byte.h>
#include <fmt.h>
#include <noreturn.h>
#include "batv.h"

#define FATAL "batv: fatal: "
#define BATVLEN 3 /*- number of bytes */

stralloc        batvkey = {0};
stralloc        nosign = {0};
int             batvkeystale = 7; /*- accept batvkey for a week */
stralloc        newsender = { 0 };

no_return void
die_nomem()
{
	substdio_puts(subfderr, "batv: out of memory\n");
	substdio_flush(subfderr);
	_exit(111);
}

void
print_batv_err(int code, int days)
{
	char            strnum[FMT_ULONG];

	switch (code)
	{
	case BATV_NOSIGNATURE:
		if (substdio_puts(subfderr, "missing BATV address") == -1)
			_exit(111);
		break;
	case BATV_BADFORMAT:
		if (substdio_puts(subfderr, "bad BATV address encoding") == -1)
			_exit(111);
		break;
	case BATV_STALE:
		strnum[fmt_int(strnum, days)] = 0;
		if (substdio_puts(subfderr, "expired BATV address [") == -1 ||
				substdio_puts(subfderr, strnum) == -1 ||
				substdio_puts(subfderr, " days]") == -1)
			_exit(111);
		break;
	}
	if (substdio_flush(subfderr) == -1)
		_exit(111);
	return;
}

int
checkbatv(char *sender, int *days)
{
	int             daynumber = (now() / 86400) % 1000;
	int             i, len;
	int             md5pos;
	int             atpos, slpos;
	char            kdate[] = "0000";
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_MD_CTX     *mdctx;
	EVP_MD         *md = 0;
	unsigned char   md5digest[EVP_MAX_MD_SIZE];
	unsigned int    md_len;
#else
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];
#endif
	unsigned long   signday;

	if (!stralloc_copys(&newsender, sender) || !stralloc_0(&newsender))
		die_nomem();
	newsender.len--;
	len = newsender.len;
	if (len >= (11 + 2 * BATVLEN) && !str_diffn(newsender.s, "prvs=", 5)) {
		atpos = str_rchr(newsender.s, '@');
		newsender.s[atpos] = 0;	/*- just for a moment */
		slpos = str_rchr(newsender.s, '='); /*- prefer an = sign */
		newsender.s[atpos] = '@'; /*- put back the @ */
		byte_copy(kdate, 4, newsender.s + 5); /*- string after prvs= */
		md5pos = 9;
	} else
		return BATV_NOSIGNATURE; /*- no BATV */
	if (kdate[0] != '0')
		return BATV_BADFORMAT; /*- not known format 0 */
	if (scan_ulong(kdate + 1, &signday) != 3)
		return BATV_BADFORMAT;
	if (days)
		*days = daynumber - signday;
	if ((unsigned) (daynumber - signday) > batvkeystale)
		return BATV_STALE; /*- stale bounce */
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!(md = EVP_MD_fetch(NULL, "md5", NULL)))
		strerr_die2x(111, FATAL, "EVP_MD_fetch: unable to fetch digest implementation for MD5");
	if (!(mdctx = EVP_MD_CTX_new()))
		die_nomem();
	if (!EVP_DigestInit_ex(mdctx, md, NULL) ||
			!EVP_DigestUpdate(mdctx, kdate, 4) ||/*- date */
			!EVP_DigestUpdate(mdctx, newsender.s + slpos + 1, len - slpos - 1) ||
			!EVP_DigestUpdate(mdctx, batvkey.s, batvkey.len) ||
			!EVP_DigestFinal_ex(mdctx, md5digest, &md_len))
		strerr_die2x(111, FATAL, "unable to hash md5 message digest");
	EVP_MD_free(md);
#else
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4); /* date */
	MD5_Update(&md5, newsender.s + slpos + 1, len - slpos - 1);
	MD5_Update(&md5, batvkey.s, batvkey.len);
	MD5_Final(md5digest, &md5);
#endif
	for (i = 0; i < BATVLEN; i++) {
		int             c, x;

		c = newsender.s[md5pos + 2 * i];
		if (isdigit(c))
			x = c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x = 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x = 10 + c - 'A';
		else
			return BATV_BADFORMAT;
		x <<= 4;
		c = newsender.s[md5pos + 2 * i + 1];
		if (isdigit(c))
			x += c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x += 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x += 10 + c - 'A';
		else
			return BATV_BADFORMAT;
		if (x != md5digest[i])
			return BATV_BADFORMAT;
	}
	/*- peel off the signature */
	if (!stralloc_copyb(&newsender, newsender.s + slpos + 1, len - (slpos + 1)) ||
			!stralloc_0(&newsender))
		die_nomem();
	return BATV_OK;
}

char *
signbatv(char *sender)
{
	int             daynumber = (now() / 86400) % 1000;
	int             i, len;
	char            kdate[] = "0000";
	static char     hex[] = "0123456789abcdef";
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_MD_CTX     *mdctx;
	EVP_MD         *md = 0;
	unsigned char   md5digest[EVP_MAX_MD_SIZE];
	unsigned int    md_len;
#else
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];
#endif

	if (!str_diffn(sender, "prvs=", 5))
		return sender; /*- already signed */
	if (!str_diffn(sender, "sb*-", 4)) /* don't sign this */
		return sender + 4;
	len = str_len(sender);
	if (!stralloc_ready(&newsender, len + (2 * BATVLEN + 10)) ||
			!stralloc_copyb(&newsender, "prvs=", 5))
		die_nomem();
	/*- only one key so far */
	kdate[1] = '0' + daynumber / 100;
	kdate[2] = '0' + (daynumber / 10) % 10;
	kdate[3] = '0' + daynumber % 10;
	if (!stralloc_catb(&newsender, kdate, 4))
		die_nomem();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	if (!(md = EVP_MD_fetch(NULL, "md5", NULL)))
		strerr_die2x(111, FATAL, "EVP_MD_fetch: unable to fetch digest implementation for MD5");
	if (!(mdctx = EVP_MD_CTX_new()))
		die_nomem();
	if (!EVP_DigestInit_ex(mdctx, md, NULL) ||
			!EVP_DigestUpdate(mdctx, kdate, 4) ||/*- date */
			!EVP_DigestUpdate(mdctx, sender, len) ||
			!EVP_DigestUpdate(mdctx, batvkey.s, batvkey.len) ||
			!EVP_DigestFinal_ex(mdctx, md5digest, &md_len))
		strerr_die2x(111, FATAL, "unable to hash md5 message digest");
	EVP_MD_free(md);
#else
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	MD5_Update(&md5, sender, len);
	MD5_Update(&md5, batvkey.s, batvkey.len);
	MD5_Final(md5digest, &md5);
#endif
	for (i = 0; i < BATVLEN; i++) {
		char            md5hex[2];

		md5hex[0] = hex[md5digest[i] >> 4];
		md5hex[1] = hex[md5digest[i] & 15];
		if (!stralloc_catb(&newsender, md5hex, 2))
			die_nomem();
	}
	/*-	separator */
	if (!stralloc_catb(&newsender, "=", 1) ||
			!stralloc_catb(&newsender, sender, len) ||
			!stralloc_0(&newsender))
		die_nomem();
	return newsender.s;
}

const char     *usage =
				"usage: batv -k key [-t stale ] -s sender | -v recipient\n"
				"        -k key       (signing key)\n"
				"        -t stale     (key validity period in days default 7)\n"
				"        -s sender    (batv signing)\n"
				"        -v recipient (batv  verify)";

int
main(int argc, char **argv)
{
	char           *ptr = 0, *arg;
	int             opt, signing, i, days;

	signing = 0;
	while ((opt = getopt(argc, argv, "svk:t:")) != opteof) {
		switch (opt)
		{
		case 'k':
			if (!stralloc_copys(&batvkey, optarg))
				die_nomem();
			break;
		case 't':
			scan_int(optarg, &batvkeystale);
			break;
		case 's':
			signing = 1;
			break;
		case 'v':
			signing = 0;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (optind + 1 != argc || !batvkey.len)
		strerr_die1x(100, usage);
	arg = argv[optind++];
	if (signing)
		ptr = signbatv(arg);
	else {
		if (!(i = checkbatv(arg, &days)))
			ptr = newsender.s;
		else {
			if (substdio_puts(subfderr, "batv: could not verify ") == -1 ||
					substdio_puts(subfderr, arg) == -1 ||
					substdio_puts(subfderr, ": ") == -1)
				_exit(111);
			print_batv_err(i, days);
			if (substdio_puts(subfderr, "\n") == -1 || substdio_flush(subfderr) == -1)
				_exit(111);
			return i;
		}
	}
	if (ptr) {
		if (substdio_puts(subfdout, ptr) == -1 ||
				substdio_puts(subfdout, "\n") == -1 ||
				substdio_flush(subfdout) == -1)
			_exit(111);
	}
}
#else
#warning "not compiled with -DBATV"
#include "substdio.h"
#include <unistd.h>

static char     sserrbuf[512];
struct substdio sserr;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	substdio_fdbuf(&sserr, write, 2, sserrbuf, sizeof(sserrbuf));
	substdio_puts(&sserr, "not compiled with -DBATV\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif

void
getversion_batv_c()
{
	const char     *x = "$Id: batv.c,v 1.11 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
