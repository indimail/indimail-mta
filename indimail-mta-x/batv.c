/*
 * $Log: batv.c,v $
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
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
 */
#ifdef BATV
#include <unistd.h>
#include <ctype.h>
#include <openssl/md5.h>
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
#include <noreturn.h>

#define FATAL "batv: fatal: "
#define BATVLEN 3 /*- number of bytes */

stralloc        signkey = {0};
stralloc        nosign = {0};
struct constmap mapnosign;
stralloc        nosigndoms = {0};
struct constmap mapnosigndoms;
int             signkeystale = 7; /*- accept signkey for a week */

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

no_return void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, "batv: out of memory\n");
	substdio_flush(subfderr);
	_exit(111);
}

stralloc        newsender = { 0 };
int
checkbatv(char *recipient)
{
	int             daynumber = (now() / 86400) % 1000;
	int             i, len;
	int             md5pos;
	int             atpos, slpos;
	char            kdate[] = "0000";
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];
	unsigned long   signday;

	len = str_len(recipient);
	if (len >= (11 + 2 * BATVLEN) && !str_diffn(recipient, "prvs=", 5)) {
		atpos = str_rchr(recipient, '@');
		if (atpos < len)
			recipient[atpos] = 0;	/*- just for a moment */
		slpos = str_rchr(recipient, '='); /*- prefer an = sign */
		if (atpos < len)
			recipient[atpos] = '@';
		byte_copy(kdate, 4, recipient + 5);
		md5pos = 9;
	} else
		return 0; /*- no BATV */
	if (kdate[0] != '0')
		return 0; /*- not known format 0 */
	if (scan_ulong(kdate + 1, &signday) != 3)
		return 0;
	if ((unsigned) (daynumber - signday) > signkeystale)
		return 0; /*- stale bounce */
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4); /* date */
	MD5_Update(&md5, recipient + slpos + 1, len - slpos - 1);
	MD5_Update(&md5, signkey.s, signkey.len);
	MD5_Final(md5digest, &md5);
	for (i = 0; i < BATVLEN; i++) {
		int             c, x;
		c = recipient[md5pos + 2 * i];
		if (isdigit(c))
			x = c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x = 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x = 10 + c - 'A';
		else
			return 0;
		x <<= 4;
		c = recipient[md5pos + 2 * i + 1];
		if (isdigit(c))
			x += c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x += 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x += 10 + c - 'A';
		else
			return 0;
		if (x != md5digest[i])
			return 0;
	}
	/*
	 * peel off the signature 
	 */
	byte_copy(recipient, len - (slpos + 1), recipient + (slpos + 1));
	recipient[len - slpos - 1] = 0;
	return 1;
}

char *
signbatv(char *sender)
{
	int             daynumber = (now() / 86400) % 1000;
	int             i, len;
	char            kdate[] = "0000";
	static char     hex[] = "0123456789abcdef";
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];

	if (!str_diffn(sender, "prvs=", 5))
		return sender; /*- already signed */
	if (!str_diffn(sender, "sb*-", 4)) { /* don't sign this */
		return sender + 4;
	}
	len = str_len(sender);
	if (!stralloc_ready(&newsender, len + (2 * BATVLEN + 10)))
		die_nomem();
	if (!stralloc_copyb(&newsender, "prvs=", 5))
		die_nomem();
	/*- only one key so far */
	kdate[1] = '0' + daynumber / 100;
	kdate[2] = '0' + (daynumber / 10) % 10;
	kdate[3] = '0' + daynumber % 10;
	if (!stralloc_catb(&newsender, kdate, 4))
		die_nomem();
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	MD5_Update(&md5, sender, len);
	MD5_Update(&md5, signkey.s, signkey.len);
	MD5_Final(md5digest, &md5);
	for (i = 0; i < BATVLEN; i++) {
		char            md5hex[2];

		md5hex[0] = hex[md5digest[i] >> 4];
		md5hex[1] = hex[md5digest[i] & 15];
		if (!stralloc_catb(&newsender, md5hex, 2))
			die_nomem();
	}
	/*-	separator */    
	if (!stralloc_catb(&newsender, "=", 1))
		die_nomem();    
	if (!stralloc_catb(&newsender, sender, len))
		die_nomem();    
	if (!stralloc_0(&newsender))
		die_nomem();    
	return newsender.s;
}

char           *usage =
				"usage: batv -k key [-s sender | -v recipient]\n"
				"        -k key       (signing key)\n"
				"        -s sender    (batv signing)\n"
				"        -t stale     (batv signing key validity period in days\n"
				"        -v recipient (batv  verify)";

int
main(int argc, char **argv)
{
	char           *ptr = 0, *arg;
	int             opt, signing;

	signing = 0;
	while ((opt = getopt(argc, argv, "svk:t:")) != opteof) {
		switch (opt) {
		case 'k':
			if (!stralloc_copys(&signkey, optarg))
				die_nomem();
			break;
		case 't':
			scan_int(optarg, &signkeystale);
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
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	arg = argv[optind++];
	if (signing)
		ptr = signbatv(arg);
	else
	if (!signing)
	{
		if (checkbatv(arg))
			ptr = arg;
		else
			ptr = (char *) 0;
	}
	if (ptr)
		out(ptr);
	else
	{
		out("could not ");
		out(signing ? "sign" : "verify");
		out(" ");
		out(arg);
	}
	out("\n");
	flush();
	return (ptr ? 0 : 111);
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
	static char    *x = "$Id: batv.c,v 1.6 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
