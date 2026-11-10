/*
 * dkim.c - sign / verify a message on descriptor 0 using dkim_functions.c
 *
 * Replaces dkim.cpp (which sat on top of the C++ libdkim).
 *
 * (cat /tmp/mail.txt | ./dkim -z 2 -y private -s /etc/indimail/control/domainkeys/private; \
 *   cat /tmp/mail.txt) | ./dkim -v
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/crypto.h>
#include "dkim_functions.h"

extern char    *dns_text(char *); /*- returns TXT record, "e=temp;" or "e=perm;" */

typedef struct {
	const char     *fn;
	const char     *selector;
	int             hash;
	char           *key;
} PRIVKEY;

static const char *program;
static const char *excl;
static const char *sigheader = "DKIM-Signature:";

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-lqthvVHSf] [-c <r|s|t|u>] [-d domain]\n%s\n", program,
			"\t[-i you@domain] [-x expire_time] [-z hash] [-y selector] -s privkeyfile [-T [selector=]dnstext]");
	fprintf(stderr, "l include body length tag when signing, honor it when verifying\n");
	fprintf(stderr, "q include query method tag\n");
	fprintf(stderr, "t include a timestamp tag\n");
	fprintf(stderr, "h include Copied Headers (z= tag)\n");
	fprintf(stderr, "f allow Unsigned From (default is to fail if not every From field is signed)\n");
	fprintf(stderr, "S allow Unsigned Subject (default is to fail if Subject field is not signed)\n");
	fprintf(stderr, "v verify the message\n");
	fprintf(stderr, "c <canonicalization> r for relaxed [DEFAULT], s - simple, t relaxed/simple, u - simple/relaxed\n");
	fprintf(stderr, "d <domain> the domain tag, if not provided, determined from the From header\n");
	fprintf(stderr, "i <identity> the identity, if not provided it will not be included\n");
	fprintf(stderr, "x <expire_time> seconds from now ( DEFAULT = 604800). If set to - it will not be included\n");
	fprintf(stderr, "z <hash> 1 - sha1, 2 - sha256, 3 - sha1+sha256, 4 - ed25519\n");
	fprintf(stderr, "y <selector> the selector tag DEFAULT=basename of privkeyfile\n");
	fprintf(stderr, "s <privkeyfile> sign the message using the private key in privkeyfile\n");
	fprintf(stderr, "                (may be repeated; -y and -z before an -s apply to that key)\n");
	fprintf(stderr, "T [selector=]DNSText Use DNSText as domainkey text record instead of using DNS.\n");
	fprintf(stderr, "                With selector= the record is used only for that selector and -T can be\n");
	fprintf(stderr, "                repeated. Without selector= it is used for every signature.\n");
	fprintf(stderr, "X excl Colon separated list of headers to exclude from signing\n");
	fprintf(stderr, "p <n>  obsolete, ignored (SSP/ADSP have been removed)\n");
	fprintf(stderr, "V set verbose mode\n");
	fprintf(stderr, "H this help\n");
	exit(1);
}

/* exact (case insensitive) match of the header's name against a colon separated list */
static int
name_in_list(const char *header, const char *list)
{
	const char     *p = list, *e;
	size_t          n;

	while (*p) {
		while (*p == ':')
			p++;
		for (e = p; *e && *e != ':'; e++);
		n = e - p;
		if (n && !strncasecmp(header, p, n) && header[n] == ':')
			return 1;
		p = e;
	}
	return 0;
}

/* returns 1 if the header is to be signed */
static int
SignThisHeader(const char *h)
{
	if ((!strncasecmp(h, "X-", 2) && strncasecmp(h, "X-Mailer:", 9))
			|| !strncasecmp(h, "Received:", 9)
			|| !strncasecmp(h, "Authentication-Results:", 23)
			|| !strncasecmp(h, "Arc-Authentication-Results:", 27)
			|| !strncasecmp(h, sigheader, strlen(sigheader))
			|| !strncasecmp(h, "Return-Path:", 12))
		return 0;
	if (excl && name_in_list(h, excl))
		return 0;
	return 1;
}

/*
 * -T selector=record  (repeatable): use "record" instead of DNS for the key
 * of that selector; selectors without a -T entry are looked up in DNS.
 * -T record (no selector=): the record is used for every signature.
 */
#define MAX_TXT 32
static struct {
	char           *selector;
	char           *record;
} txt[MAX_TXT];
static int      ntxt;

/* OPENSSL_strdup() only exists in OpenSSL >= 1.1.0; the library frees with OPENSSL_free() */
static char *
openssl_strdup(const char *s)
{
	size_t          n = strlen(s) + 1;
	char           *p = OPENSSL_malloc(n);

	if (p)
		memcpy(p, s, n);
	return p;
}

static char *
dns_lookup(const char *name)
{
	const char     *p = strstr(name, "._domainkey.");
	size_t          n = p ? (size_t) (p - name) : 0;
	int             i;

	for (i = 0; i < ntxt; i++)
		if (strlen(txt[i].selector) == n && !strncasecmp(name, txt[i].selector, n))
			return openssl_strdup(txt[i].record);
	return dns_text((char *) name);
}

/* "selector=record" has a selector name (more than one char, no spaces/';') before the first '=' */
static int
split_selector(char *arg, char **record)
{
	char           *eq = strchr(arg, '=');
	char           *p;

	if (!eq || eq - arg < 2)
		return 0; /*- single letter before '=' is a record tag (v=, k=, p= ...) */
	for (p = arg; p < eq; p++)
		if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') ||
				*p == '.' || *p == '-' || *p == '_'))
			return 0;
	*eq = 0;
	*record = eq + 1;
	return 1;
}

static void
status_header(int ret)
{
	printf("DKIM-Status: %s\n", dkim_status_text(ret, NULL));
}

static char *
read_file(const char *fn)
{
	int             fd;
	struct stat     st;
	char           *p;
	ssize_t         n;

	if ((fd = open(fn, O_RDONLY)) == -1) {
		fprintf(stderr, "%s: %s\n", fn, strerror(errno));
		return NULL;
	}
	if (fstat(fd, &st) == -1) {
		fprintf(stderr, "fstat: %s: %s\n", fn, strerror(errno));
		close(fd);
		return NULL;
	}
	if (!(p = malloc(st.st_size + 1))) {
		fprintf(stderr, "malloc: %lld bytes: %s\n", (long long) st.st_size + 1, strerror(errno));
		close(fd);
		return NULL;
	}
	if ((n = read(fd, p, st.st_size)) != st.st_size) {
		fprintf(stderr, "read: %s: %s\n", fn, strerror(errno));
		close(fd);
		free(p);
		return NULL;
	}
	close(fd);
	p[n] = 0;
	return p;
}

static void
verror(int ret)
{
	const char     *t = dkim_status_text(ret, NULL);

	if (ret != DKIM_SUCCESS)
		fprintf(stderr, "%s\n", t);
}

int
main(int argc, char **argv)
{
	PRIVKEY        *keys = NULL;
	int             nkeys = 0, nHash = 0, ch, i, ret, verbose = 0, bSign = 1;
	const char     *selector = NULL, *dnstext = NULL, *p;
	char           *rec;
	char            buffer[8192];
	time_t          t = time(NULL);
	DKIMSignOptions sopts;
	DKIMVerifyOptions vopts;

#if OPENSSL_VERSION_NUMBER < 0x10101000L
	fprintf(stderr, "openssl version too old\n");
	exit(0);
#endif
	memset(&sopts, 0, sizeof(sopts));
	memset(&vopts, 0, sizeof(vopts));
	if (!(program = strrchr(argv[0], '/')))
		program = argv[0];
	else
		program++;
	sopts.nCanon = DKIM_SIGN_RELAXED;
	sopts.nHash = DKIM_HASH_SHA256;
	sopts.expireTime = t + 604800;
	sopts.pfnHeaderCallback = SignThisHeader;
	vopts.nSubjectRequired = 1;
	vopts.pfnDnsCallback = dns_lookup;
	if ((p = getenv("DKIMSIGN_HEADER")) && *p)
		sigheader = p;
	excl = getenv("EXCLUDE_DKIMSIGN");

	while ((ch = getopt(argc, argv, "lqtfhHSvVp:b:c:d:i:s:x:X:y:z:T:")) != -1) {
		switch (ch)
		{
		case 'l':
			vopts.nHonorBodyLengthTag = 1;
			sopts.nIncludeBodyLengthTag = 1;
			break;
		case 'q':
			sopts.nIncludeQueryMethod = 1;
			break;
		case 'S':
			vopts.nSubjectRequired = 0;
			break;
		case 'f':
			vopts.nAllowUnsignedFromHeaders = 1;
			break;
		case 't':
			sopts.nIncludeTimeStamp = 1;
			break;
		case 'h':
			sopts.nIncludeCopiedHeaders = 1;
			break;
		case 'H':
			usage();
			break;
		case 'v':
			bSign = 0;
			break;
		case 'V':
			verbose = vopts.verbose = 1;
			break;
		case 'p': /*- obsolete */
		case 'b':
			break;
		case 'c':
			switch (*optarg)
			{
			case 'r': sopts.nCanon = DKIM_SIGN_RELAXED; break;
			case 's': sopts.nCanon = DKIM_SIGN_SIMPLE; break;
			case 't': sopts.nCanon = DKIM_SIGN_RELAXED_SIMPLE; break;
			case 'u': sopts.nCanon = DKIM_SIGN_SIMPLE_RELAXED; break;
			default:
				fprintf(stderr, "%s: unrecognized canonicalization.\n", program);
				return 1;
			}
			break;
		case 'd':
			strncpy(sopts.szDomain, optarg, sizeof(sopts.szDomain) - 1);
			break;
		case 'i':
			if (*optarg == '-')
				sopts.szIdentity[0] = 0;
			else
				strncpy(sopts.szIdentity, optarg, sizeof(sopts.szIdentity) - 1);
			break;
		case 'y':
			selector = optarg;
			break;
		case 'z':
			switch (*optarg)
			{
			case '1': nHash = DKIM_HASH_SHA1; break;
			case '2': nHash = DKIM_HASH_SHA256; break;
			case '3': nHash = DKIM_HASH_SHA1_AND_SHA256; break;
			case '4': nHash = DKIM_HASH_ED25519; break;
			default:
				fprintf(stderr, "%s: unrecognized hash.\n", program);
				return 1;
			}
			break;
		case 'x':
			sopts.expireTime = (*optarg == '-') ? 0 : t + atoi(optarg);
			break;
		case 's': {
			PRIVKEY        *n = realloc(keys, sizeof(PRIVKEY) * (nkeys + 1));
			const char     *slash;

			if (!n) {
				fprintf(stderr, "malloc: %s\n", strerror(errno));
				return 1;
			}
			keys = n;
			if (!(keys[nkeys].key = read_file(optarg)))
				return 1;
			keys[nkeys].fn = optarg;
			keys[nkeys].selector = selector ? selector :
				((slash = strrchr(optarg, '/')) ? slash + 1 : optarg);
			keys[nkeys].hash = nHash ? nHash : DKIM_HASH_SHA256;
			nkeys++;
			bSign = 1;
			selector = NULL;
			nHash = 0;
			break;
		}
		case 'T':
			if (split_selector(optarg, &rec)) {
				if (ntxt == MAX_TXT) {
					fprintf(stderr, "%s: too many -T options\n", program);
					return 1;
				}
				txt[ntxt].selector = optarg;
				txt[ntxt++].record = rec;
			} else
				dnstext = optarg;
			break;
		case 'X':
			excl = optarg;
			break;
		default:
			usage();
		}
	}

	if (bSign) {
		DKIMSign       *s;

		if (!nkeys) {
			fprintf(stderr, "Private Key(s) not provided\n");
			usage();
		}
		if (!(s = dkim_sign_new(&sopts))) {
			fprintf(stderr, "out of memory\n");
			return 1;
		}
		for (;;) {
			ssize_t         n = read(0, buffer, sizeof(buffer));

			if (n == -1) {
				fprintf(stderr, "read: %s\n", strerror(errno));
				dkim_sign_free(s);
				return 1;
			}
			if (!n)
				break;
			if ((ret = dkim_sign_process(s, buffer, n))) {
				verror(ret);
				dkim_sign_free(s);
				return 1;
			}
		}
		/* the message has been read once; every key signs from the same state */
		for (i = 0; i < nkeys; i++) {
			char           *sig = NULL;

			strncpy(sopts.szSelector, keys[i].selector, sizeof(sopts.szSelector) - 1);
			sopts.nHash = keys[i].hash;
			if ((ret = dkim_sign_get(s, &sopts, keys[i].key, &sig))) {
				fprintf(stderr, "%s: ", keys[i].fn);
				verror(ret);
				dkim_sign_free(s);
				return 1;
			}
			fputs(sig, stdout);
			dkim_free(sig);
		}
		dkim_sign_free(s);
		for (i = 0; i < nkeys; i++)
			free(keys[i].key);
		free(keys);
		return 0;
	} else {
		DKIMVerify     *v;
		DKIMVerifyDetails *d;
		int             nsig = 0;
		const char     *dkimverify;

		vopts.pszSelectorOverride = dnstext;
		if (!(v = dkim_verify_new(&vopts))) {
			fprintf(stderr, "out of memory\n");
			return 1;
		}
		for (;;) {
			ssize_t         n = read(0, buffer, sizeof(buffer));

			if (n == -1) {
				fprintf(stderr, "read: %s\n", strerror(errno));
				dkim_verify_free(v);
				return 1;
			}
			if (!n)
				break;
			if ((ret = dkim_verify_process(v, buffer, n))) {
				verror(ret);
				dkim_verify_free(v);
				return 1;
			}
		}
		if ((ret = dkim_verify_finish(v, &nsig, &d))) {
			verror(ret);
			dkim_verify_free(v);
			return 1;
		}
		if (verbose) {
			for (i = 0; i < nsig; i++) {
				fprintf(stderr, "Signature # %02d: d=%s s=%s a=%s: ", i + 1,
						d[i].szDomain ? d[i].szDomain : "?",
						d[i].szSelector ? d[i].szSelector : "?",
						d[i].szAlgorithm ? d[i].szAlgorithm : "?");
				if (d[i].nResult >= 0)
					fprintf(stderr, "Success\n");
				else
					fprintf(stderr, "Failure %d (%s)\n", d[i].nResult, dkim_code_name(d[i].nResult));
			}
		}
		ret = dkim_verify_status(nsig, d);
		if (ret != DKIM_SUCCESS)
			verror(ret);
		dkim_verify_free(v);
		status_header(ret);
		if ((dkimverify = getenv("DKIMVERIFY"))) {
			if (ret < 0) {
				if (strchr(dkimverify, 'F' - ret))
					return 14; /*- permanent error */
				if (strchr(dkimverify, 'f' - ret))
					return 88; /*- temporary error */
			} else {
				if (strchr(dkimverify, 'A' + ret))
					return 14;
				if (strchr(dkimverify, 'a' + ret))
					return 88;
			}
			return 0;
		}
		return ret < 0 ? 1 : 0;
	}
}

void
getversion_dkim_c()
{
	static char    *x = (char *) "$Id: dkim.c,v 1.41 2026-11-10 15:14:08+05:30 Cprogrammer Exp mbhangui $";

	if (x++) x--;
}

/*
 * $Log: dkim.c,v $
 * Revision 1.41  2026-11-10 15:14:08+05:30  Cprogrammer
 * fix compilation with older version of openssl
 *
 * Revision 1.40  2026-11-09 23:21:51+05:30  Cprogrammer
 * non-libdkim2 version
 *
 * Revision 1.39  2026-10-21 21:47:10+05:30  Cprogrammer
 * fix compiler warnings
 *
 * Revision 1.38  2025-02-08 23:23:12+05:30  Cprogrammer
 * make DKIM-Signature header name configurable
 *
 * Revision 1.37  2024-05-07 12:55:27+05:30  Cprogrammer
 * use const char * instead of char *
 *
 * Revision 1.36  2024-01-06 21:26:06+05:30  Cprogrammer
 * added new error code DKIM_BAD_IDENTITY for invalid identity domain (i= tag)
 *
 * Revision 1.35  2023-11-20 10:07:40+05:30  Cprogrammer
 * Added -X option to have colon separated list of headers to be excluded from DKIM signing
 * use EXCLUDE_DKIMSIGN to specify colon separated list of headers to be excluded from DKIM signing
 *
 * Revision 1.34  2023-02-19 08:48:17+05:30  Cprogrammer
 * fixed usage strings
 *
 * Revision 1.33  2023-02-12 08:07:00+05:30  Cprogrammer
 * added dkim_error_str function to return DKIM error
 *
 * Revision 1.32  2023-02-04 11:56:10+05:30  Cprogrammer
 * generate DKIM-Signature for each -s option
 *
 * Revision 1.31  2023-01-30 10:41:09+05:30  Cprogrammer
 * set verbose flag for dkimvery, dkimsign methods
 *
 * Revision 1.30  2023-01-27 19:38:28+05:30  Cprogrammer
 * fixed openssl version for ed25519
 *
 * Revision 1.29  2023-01-27 17:11:41+05:30  Cprogrammer
 * removed -b option. Option kept for backward compatibility
 * added -z 4 for setting Ed25519 DKIM signature
 *
 * Revision 1.28  2022-11-27 09:41:38+05:30  Cprogrammer
 * updated help message for -h option
 *
 * Revision 1.27  2021-07-20 23:20:09+05:30  Cprogrammer
 * removed use of register
 *
 * Revision 1.26  2020-10-01 14:14:34+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.25  2020-06-08 23:16:27+05:30  Cprogrammer
 * quench compiler warnings
 *
 * Revision 1.24  2019-06-24 23:14:33+05:30  Cprogrammer
 * fixed return value interpretation of DKIMVERIFY
 *
 * Revision 1.23  2019-06-14 21:24:59+05:30  Cprogrammer
 * BUG - honor body length tag in verification
 *
 * Revision 1.22  2019-01-13 10:10:27+05:30  Cprogrammer
 * added missing usage string for allowing unsigned subject.
 *
 * Revision 1.21  2018-08-08 23:57:02+05:30  Cprogrammer
 * issue success if at lease one one good signature is found
 *
 * Revision 1.20  2018-05-22 10:03:26+05:30  Cprogrammer
 * changed return type of writeHeader() to void
 *
 * Revision 1.19  2016-03-01 16:23:38+05:30  Cprogrammer
 * added -S option to allow email with unsigned subject
 *
 * Revision 1.18  2016-02-01 10:53:32+05:30  Cprogrammer
 * use basename of private key as the selector in absense of -y option
 *
 * Revision 1.17  2015-12-15 15:36:01+05:30  Cprogrammer
 * added case 3 for 3rd party signature without SSP and ADSP
 * increased buffer size for Apple mail with X-BrightMail-Tracker header issue
 *
 * Revision 1.16  2012-08-16 08:01:19+05:30  Cprogrammer
 * do not skip X-Mailer headers
 *
 * Revision 1.15  2011-06-04 13:55:50+05:30  Cprogrammer
 * set AllowUnsignedFromHeaders
 *
 * Revision 1.14  2011-06-04 09:36:36+05:30  Cprogrammer
 * added AllowUnsignedFromHeaders option
 *
 * Revision 1.13  2011-02-07 22:05:23+05:30  Cprogrammer
 * added case DKIM_3PS_SIGNATURE
 *
 * Revision 1.12  2010-05-04 14:00:13+05:30  Cprogrammer
 * make option '-z' work on systems without SHA_256
 *
 * Revision 1.11  2009-04-20 08:35:45+05:30  Cprogrammer
 * corrected usage()
 *
 * Revision 1.10  2009-04-15 21:30:32+05:30  Cprogrammer
 * added DKIM-Signature to list of excluded headers
 *
 * Revision 1.9  2009-04-15 20:45:04+05:30  Cprogrammer
 * corrected usage
 *
 * Revision 1.8  2009-04-05 19:04:44+05:30  Cprogrammer
 * improved formating of usage
 *
 * Revision 1.7  2009-04-03 12:05:25+05:30  Cprogrammer
 * minor changes on usage display
 *
 * Revision 1.6  2009-03-28 20:15:23+05:30  Cprogrammer
 * invoke DKIMVerifyGetDetails()
 *
 * Revision 1.5  2009-03-27 20:43:48+05:30  Cprogrammer
 * added HAVE_OPENSSL_EVP_H conditional
 *
 * Revision 1.4  2009-03-27 20:19:28+05:30  Cprogrammer
 * added ADSP
 *
 * Revision 1.3  2009-03-26 15:10:53+05:30  Cprogrammer
 * added ADSP
 *
 * Revision 1.2  2009-03-25 08:37:45+05:30  Cprogrammer
 * added dkim_error
 *
 * Revision 1.1  2009-03-21 08:24:47+05:30  Cprogrammer
 * Initial revision
 */
