/*
 * $Log: dkim.cpp,v $
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
 *
 *
 * This code incorporates intellectual property owned by Yahoo! and licensed 
 * pursuant to the Yahoo! DomainKeys Patent License Agreement.
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */
/*
 * (cat /tmp/test.msg|./dkimtest -z 2 -b 1 -y private \
 * -s /var/indimail/control/domainkeys/private ;cat /tmp/test.msg )|./dkimtest -v
 */
#ifndef __cplusplus
#error A C++ compiler is required!
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dkim.h"
#include "dns.h"

#ifdef HAVE_OPENSSL_EVP_H
#include <openssl/evp.h>
#define DKIM_MALLOC(s)     OPENSSL_malloc(s)
#define DKIM_MFREE(s)      OPENSSL_free(s); s = NULL;
#else
#define DKIM_MALLOC(s)     malloc(s)
#define DKIM_MFREE(s)      free(s); s = NULL;
#endif

int DKIM_CALL
SignThisHeader(const char *szHeader)
{
	if ((!strncasecmp(szHeader, "X-", 2) && strncasecmp(szHeader, "X-Mailer", 8))
		|| !strncasecmp(szHeader, "Received:", 9)
		|| !strncasecmp(szHeader, "DKIM-Signature:", 15)
		|| !strncasecmp(szHeader, "Authentication-Results:", 23)
		|| !strncasecmp(szHeader, "DomainKey-Signature", 19)
		|| !strncasecmp(szHeader, "Return-Path:", 12)) 
	{
		return 0;
	}
	return 1;
}

char           *program;

void
usage()
{
#ifdef HAVE_EVP_SHA256
	fprintf(stderr, "usage: %s [-lqthvH] [-p <0|1|2>] [-b <1|2|3>] [-c <r|s|t|u>]\n\t[-d domain] [-i you@domain] [-x expire_time] [-z hash] [-y selector] -s privkeyfile\n", program);
#else
	fprintf(stderr, "usage: %s [-lqthvH] [-p <0|1|2>] [-b <1|2|3>] [-c <r|s|t|u>]\n\t[-d domain] [-i you@domain] [-x expire_time] [-y selector] -s privkeyfile\n", program);
#endif
	fprintf(stderr, "l                    include body length tag\n");
	fprintf(stderr, "q                    include query method tag\n");
	fprintf(stderr, "t                    include a timestamp tag\n");
	fprintf(stderr, "h                    include Copied Headers\n");
	fprintf(stderr, "f                    allow Unsigned From (default is to reject if From field is not signed)\n");
	fprintf(stderr, "S                    allow Unsigned Subject (default is to reject if Subject field is not signed)\n");
	fprintf(stderr, "v                    verify the message\n");
	fprintf(stderr, "p <ssp|adsp>         0 - disable practice (default), 1- SSP, or 2 - ADSP verification\n");
	fprintf(stderr, "b <standard>         1 - allman, 2 - ietf or 3 - both\n");
	fprintf(stderr, "c <canonicalization> r for relaxed [DEFAULT], s - simple, t relaxed/simple, u - simple/relaxed\n");
	fprintf(stderr, "d <domain>           the domain tag, if not provided, determined from the sender/from header\n");
	fprintf(stderr, "i <identity>         the identity, if not provided it will not be included\n");
	fprintf(stderr, "x <expire_time>      the expire time in seconds since epoch ( DEFAULT = current time + 604800)\n");
	fprintf(stderr, "                     if set to - then it will not be included\n");
#ifdef HAVE_EVP_SHA256
	fprintf(stderr, "z <hash>             1 for sha1, 2 for sha256, 3 for both\n");
#endif
	fprintf(stderr, "y <selector>         the selector tag DEFAULT=private\n");
	fprintf(stderr, "s <privkeyfile>      sign the message using the private key in privkeyfile\n");
	fprintf(stderr, "V                    set verbose mode\n");
	fprintf(stderr, "H                    this help\n");
	exit(1);
}

unsigned int str_chr(char *s, int c)
{
	register char   ch;
	register char  *t;

	ch = c;
	t = s;
	for (;;) {
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
		if (!*t)
			break;
		if (*t == ch)
			break;
		++t;
	}
	return t - s;
}

void
dkim_error(int e)
{
	switch (e)
	{
	case DKIM_OUT_OF_MEMORY:
		fprintf(stderr, "memory allocation failed\n");
		break;
	case DKIM_INVALID_CONTEXT:
		fprintf(stderr, "DKIMContext structure invalid for this operation\n");
		break;
	case DKIM_NO_SENDER:
		fprintf(stderr, "Could not find From: or Sender: header in message\n");
		break;
	case DKIM_BAD_PRIVATE_KEY:
		fprintf(stderr, "Could not parse private key\n");
		break;
	case DKIM_BUFFER_TOO_SMALL:
		fprintf(stderr, "Buffer passed in is not large enough");
		break;
	}
}

/*
 * Allows you to add the headers contain the results and DKIM ADSP
 */
void writeHeader(int ret, int resDKIMSSP, int resDKIMADSP, int useSSP, int useADSP )
{
	char           *dkimStatus, *sspStatus, *adspStatus;

	dkimStatus = sspStatus = adspStatus = (char *) "";
	switch (ret)
	{
	case DKIM_SUCCESS_BUT_EXTRA:/*- 4 signature result: signature verified but it did not include all of the body */
		dkimStatus = (char *) "signature result: signature verified but it did not include all of the body";
		break;
	case DKIM_NEUTRAL:			/*- 3 verify result: no signatures verified but message is not suspicious */
		dkimStatus = (char *) "verify result: no signatures verified but message is not suspicious";
		break;
	case DKIM_PARTIAL_SUCCESS:	/*- 2 verify result: at least one but not all signatures verified */
		dkimStatus = (char *) "verify result: at least none but not all signatures verified";
		break;
	case DKIM_FINISHED_BODY:	/*- 1 process result: no more message body is needed */
		dkimStatus = (char *) "process result: no more message body is needed";
		break;
	case DKIM_SUCCESS:
		dkimStatus = (char *) "good        ";
		break;
	case DKIM_FAIL:
		dkimStatus = (char *) "failed      ";
		break;
	case DKIM_BAD_SYNTAX:
		dkimStatus = (char *) "signature error: DKIM-Signature could not parse or has bad tags/values";
		break;
	case DKIM_SIGNATURE_BAD:
		dkimStatus = (char *) "signature error: RSA verify failed";
		break;
	case DKIM_SIGNATURE_BAD_BUT_TESTING:
		dkimStatus = (char *) "signature error: RSA verify failed but testing";
		break;
	case DKIM_SIGNATURE_EXPIRED:
		dkimStatus = (char *) "signature error: x= is old";
		break;
	case DKIM_SELECTOR_INVALID:
		dkimStatus = (char *) "signature error: selector doesn't parse or contains invalid values";
		break;
	case DKIM_SELECTOR_GRANULARITY_MISMATCH:
		dkimStatus = (char *) "signature error: selector g= doesn't match i=";
		break;
	case DKIM_SELECTOR_KEY_REVOKED:
		dkimStatus = (char *) "signature error: selector p= empty";
		break;
	case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG:
		dkimStatus = (char *) "signature error: selector domain name too long to request";
		break;
	case DKIM_SELECTOR_DNS_TEMP_FAILURE:
		dkimStatus = (char *) "signature error: temporary dns failure requesting selector";
		break;
	case DKIM_SELECTOR_DNS_PERM_FAILURE:
		dkimStatus = (char *) "signature error: permanent dns failure requesting selector";
		break;
	case DKIM_SELECTOR_PUBLIC_KEY_INVALID:
		dkimStatus = (char *) "signature error: selector p= value invalid or wrong format";
		break;
	case DKIM_NO_SIGNATURES:
		dkimStatus = (char *) "process error, no sigs";
		break;
	case DKIM_NO_VALID_SIGNATURES:
		dkimStatus = (char *) "process error, no valid sigs";
		break;
	case DKIM_BODY_HASH_MISMATCH:
		dkimStatus = (char *) "sigature verify error: message body does not hash to bh value";
		break;
	case DKIM_SELECTOR_ALGORITHM_MISMATCH:
		dkimStatus = (char *) "signature error: selector h= doesn't match signature a=";
		break;
	case DKIM_STAT_INCOMPAT:
		dkimStatus = (char *) "signature error: incompatible v=";
		break;
	default:
		dkimStatus = (char *) "error";
		break;
	}
	if (useSSP && resDKIMSSP != -1)
	{
		switch(resDKIMSSP)
		{
			case DKIM_SSP_ALL:
				sspStatus = (char *) "all;";
				break;
			case DKIM_SSP_STRICT:
				sspStatus = (char *) "strict;";
				break;
			case DKIM_SSP_SCOPE:
				sspStatus = (char *) "out of scope;";
				break;
			case DKIM_SSP_TEMPFAIL:
				sspStatus = (char *) "temporary failure;";
				break;
			case DKIM_SSP_UNKNOWN:
			default:
				sspStatus = (char *) "unknown;";
				break;
		}
	}
	if (useADSP && resDKIMADSP != -1) {
		switch(resDKIMADSP)
		{
			case DKIM_ADSP_ALL:
				adspStatus = (char *) "all;";
				break;
			case DKIM_ADSP_DISCARDABLE:
				adspStatus = (char *) "discardable;";
				break;
			case DKIM_ADSP_SCOPE:
				adspStatus = (char *) "out of scope;";
				break;
			case DKIM_ADSP_TEMPFAIL:
				adspStatus = (char *) "temporary failure;";
				break;
			case DKIM_ADSP_UNKNOWN:
			default:
				adspStatus = (char *) "unknown ;";
				break;
		}
	}
	printf("DKIM-Status: %s\n", dkimStatus);
	if (useSSP && *sspStatus)
		printf("X-DKIM-SSP: %s\n", sspStatus);
	if (useADSP && *adspStatus)
		printf("X-DKIM-ADSP: %s\n", adspStatus);
}

int
ParseTagValues(char *list, char *letters[], char *values[])
{
	char           *tmp, *ptr, *key;
	int             i;

	/*- start with all args unset */
	for (i = 0; letters[i]; i++)
		values[i] = 0;
	key = 0;
	for(ptr = list;*ptr;) {
		if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\r') || (*ptr == '\n')) /*- FWS */
			*ptr++ = 0;
		if (!key)
			key = ptr;
		if (*ptr == '=') {
			*ptr = 0;
			for (i = 0;letters[i];i++) {
				if (!strcmp(letters[i], key)) {
					ptr++;
					for (;*ptr;) {
						if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\r') || (*ptr == '\n')) {
							ptr++;
							continue;
						}
						break;
					}
					values[i] = ptr;
					for(;*ptr && *ptr != ';';ptr++);
					tmp = ptr;
					if (*ptr)
						*ptr++ = 0;
					for(;tmp != values[i];tmp--) /*- RFC 4871 3.2 */ {
						if ((*tmp == ' ') || (*tmp == '\t') || (*tmp == '\r') || (*tmp == '\n')) {
							*tmp = 0;
							continue;
						}
						break;
					}
					key = 0;
					break;
				}
			}
		} else
			ptr++;
	}
	return (0);
}

int
GetSSP(char *domain, int *bTesting)
{
	char           *query, *results;
	char           *tags[] = { (char *) "dkim", (char *) "t", (char *) 0};
	char           *values[2];
	int             bIsParentSSP = 0, iSSP = DKIM_SSP_UNKNOWN;

	*bTesting = 0;
	if (!(query = (char *) DKIM_MALLOC(strlen("_ssp._domainkey.") + strlen(domain) + 1))) {
		fprintf(stderr, "malloc: %ld: %s\n", strlen("_ssp._domainkey.") + strlen(domain) + 1,
			strerror(errno));
		exit(1);
	}
	sprintf(query, "_ssp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!strcmp(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_SSP_TEMPFAIL;
	} else
	if (!strcmp(results, "e=perm;")) {
		DKIM_MFREE(results);
		results = dns_text(domain);
		if (!strcmp(results, "e=temp;")) {
			DKIM_MFREE(results);
			return DKIM_SSP_TEMPFAIL;
		} else
		if (!strcmp(results, "e=perm;")) {
			DKIM_MFREE(results);
			return DKIM_SSP_SCOPE;
		}
		bIsParentSSP = 1;
	}
	if (!ParseTagValues(results, tags, values)) {
		DKIM_MFREE(results);
		return DKIM_SSP_UNKNOWN;
	}
	DKIM_MFREE(results);
	if (values[0] != NULL) {
		if (strcasecmp(values[0], "all") == 0)
			iSSP = DKIM_SSP_ALL;
		else
		if (strcasecmp(values[0], "strict") == 0)
			iSSP = DKIM_SSP_STRICT;
	}
	// flags
	if (values[1] != NULL) {
		char           *s, *p;
		for (p = values[1], s = values[1]; *p; p++) {
			if (*p == '|')
				*p = 0;
			else
				continue;
			if (!strcmp(s, "y"))
				*bTesting = 1;
			else
			if (!strcmp(s, "s")) {
				if (bIsParentSSP) {
					/* 
					 * this is a parent's SSP record that should not apply to subdomains
					 * the message is non-suspicious
					 */
					*bTesting = 0;
					return (DKIM_SSP_UNKNOWN);
				}
			}
			s = p + 1;
		}
	}
	return iSSP; /*- No ADSP Record */
}

int
GetADSP(char *domain)
{
	char           *query, *results;
	char           *tags[] = {(char *) "dkim", (char *) 0};
	char           *values[1];

	results = dns_text(domain);
	if (!strcmp(results, "e=perm;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!strcmp(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	if (!(query = (char *) DKIM_MALLOC(strlen((char *) "_adsp._domainkey.") + strlen(domain) + 1))) {
		fprintf(stderr, "malloc: %ld: %s\n", strlen("_adsp._domainkey.") + strlen(domain) + 1,
			strerror(errno));
		exit(1);
	}
	sprintf(query, "_adsp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!strcmp(results, "e=perm;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!strcmp(results, "e=temp;")) {
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	if (!ParseTagValues(results, tags, values)) {
		DKIM_MFREE(results);
		return DKIM_ADSP_UNKNOWN;
	}
	DKIM_MFREE(results);
	if (values[0] != NULL) {
		if (strcasecmp(values[0], "all") == 0)
			return (DKIM_ADSP_ALL);
		else
		if (strcasecmp(values[0], "discardable") == 0)
			return (DKIM_ADSP_DISCARDABLE);
	}
	return DKIM_ADSP_UNKNOWN; /*- No ADSP Record */
}

int
main(int argc, char **argv)
{
	char           *PrivKey, *PrivKeyFile = NULL, *pSig = NULL, *dkimverify;
	int             i, ret, ch, nPrivKeyLen, PrivKeyFD, verbose = 0;
	int             bSign = 1, nSigCount = 0, useSSP = 0, useADSP = 0, accept3ps = 0;
	int             sCount = 0, sSize = 0, resDKIMSSP = -1, resDKIMADSP = -1;
	int             nAllowUnsignedFromHeaders = 0;
	int             nAllowUnsignedSubject = 1;
	char            Buffer[4096], szPolicy[512];
	time_t          t;
	struct stat     statbuf;
	DKIMContext     ctxt;
	DKIMSignOptions opts = { 0 };
	DKIMVerifyDetails *pDetails;
	DKIMVerifyOptions vopts = { 0 };

	if (!(program = strrchr(argv[0], '/')))
		program = argv[0];
	else
		program++;
	t = time(0);
#ifdef HAVE_EVP_SHA256
	opts.nHash = DKIM_HASH_SHA1_AND_256;
#else
	opts.nHash = DKIM_HASH_SHA1;
#endif
	opts.nCanon = DKIM_SIGN_RELAXED;
	opts.nIncludeBodyLengthTag = 0;
	opts.nIncludeQueryMethod = 0;
	opts.nIncludeTimeStamp = 0;
	opts.expireTime = t + 604800;	// expires in 1 week
	opts.nIncludeCopiedHeaders = 0;
	opts.nIncludeBodyHash = DKIM_BODYHASH_BOTH;
	strcpy(opts.szRequiredHeaders, "NonExistent");
	opts.pfnHeaderCallback = SignThisHeader;
	while (1) {
		if ((ch = getopt(argc, argv, "lqtfhHSvVp:b:c:d:i:s:x:y:z:")) == -1)
			break;
		switch (ch)
		{
		case 'l': /*- body length tag */
			vopts.nHonorBodyLengthTag = 1;
			opts.nIncludeBodyLengthTag = 1;
			break;
		case 'q': /*- query method tag */
			opts.nIncludeQueryMethod = 1;
			break;
		case 'S':
			nAllowUnsignedSubject = 0;
			break;
		case 'f':
			nAllowUnsignedFromHeaders = 1;
		case 't': /*- timestamp tag */
			opts.nIncludeTimeStamp = 1;
			break;
		case 'h':
			opts.nIncludeCopiedHeaders = 1;
			break;
		case 'H':
			usage();
			break;
		case 'v': /*- verify */
			bSign = 0;
			break;
		case 'V': /*- verbose */
			verbose = 1;
			break;
		case 'p':
			switch(*optarg)
			{
			case '1':
				accept3ps = 1;
				useSSP = 1;
				useADSP = 0;
				break;
			case '2':
				accept3ps = 1;
				useSSP = 0;
				useADSP = 1;
				break;
			case '3':
				accept3ps = 1;
				useSSP = 0;
				useADSP = 0;
				break;
			case '0':
				accept3ps = 0;
				useSSP = 0;
				useADSP = 0;
				break;
			default:
				fprintf(stderr, "%s: unrecognized practice %c.\n", program, *optarg);
				return (1);
			}
			break;
		case 'b': /*- allman or ietf draft 1 or both */
			switch (*optarg)
			{
			case '1':
				opts.nIncludeBodyHash = DKIM_BODYHASH_ALLMAN_1;
				break;
			case '2':
				opts.nIncludeBodyHash = DKIM_BODYHASH_IETF_1;
				break;
			case '3':
				opts.nIncludeBodyHash = DKIM_BODYHASH_BOTH;
				break;
			default:
				fprintf(stderr, "%s: unrecognized standard %c.\n", program, *optarg);
				return (1);
			}
			break;
		case 'c':
			switch (*optarg)
			{
			case 'r':
				opts.nCanon = DKIM_SIGN_RELAXED;
				break;
			case 's':
				opts.nCanon = DKIM_SIGN_SIMPLE;
				break;
			case 't':
				opts.nCanon = DKIM_SIGN_RELAXED_SIMPLE;
				break;
			case 'u':
				opts.nCanon = DKIM_SIGN_SIMPLE_RELAXED;
				break;
			default:
				fprintf(stderr, "%s: unrecognized canonicalization.\n", program);
				return (1);
			}
			break;
		case 'd':
			strncpy(opts.szDomain, optarg, sizeof (opts.szDomain) - 1);
			break;
		case 'i':	/*- identity */
			if (*optarg == '-')
				opts.szIdentity[0] = '\0';
			else
				strncpy(opts.szIdentity, optarg, sizeof (opts.szIdentity) - 1);
			break;
		case 's': /*- sign */
			bSign = 1;
			PrivKeyFile = optarg;
			break;
		case 'x': /*- expire time */
			if (*optarg == '-')
				opts.expireTime = 0;
			else
				opts.expireTime = t + atoi(optarg);
			break;
		case 'y':
			strncpy(opts.szSelector, optarg, sizeof (opts.szSelector) - 1);
			break;
		case 'z': /*- sign w/ sha1, sha256 or both */
#ifdef HAVE_EVP_SHA256
			switch (*optarg)
			{
			case '1':
				opts.nHash = DKIM_HASH_SHA1;
				break;
			case '2':
				opts.nHash = DKIM_HASH_SHA256;
				break;
			case '3':
				opts.nHash = DKIM_HASH_SHA1_AND_256;
				break;
			default:
				fprintf(stderr, "%s: unrecognized hash.\n", program);
				return (1);
			}
#else
			opts.nHash = DKIM_HASH_SHA1;
#endif
			break;
		} /*- switch (ch) */
	}
	if (bSign) { /*- sign */
		if (!PrivKeyFile) {
			fprintf(stderr, "Private Key not provided\n");
			usage();
			return (1);
		}
		if (!opts.szSelector[0]) {
			if ((pSig = strrchr(PrivKeyFile, '/'))) {
				pSig++;
				strcpy(opts.szSelector, pSig);
			} else
				strcpy(opts.szSelector, "private");
		}
		if ((PrivKeyFD = open(PrivKeyFile, O_RDONLY)) == -1) {
			fprintf(stderr, "%s: %s\n", PrivKeyFile, strerror(errno));
			return (1);
		}
		if (fstat(PrivKeyFD, &statbuf) == -1) {
			fprintf(stderr, "fstat: %s: %s\n", PrivKeyFile, strerror(errno));
			return (1);
		}
		if (!(PrivKey = (char *) DKIM_MALLOC(sizeof(char) * ((nPrivKeyLen = statbuf.st_size) + 1)))) {
#ifdef DARWIN
			fprintf(stderr, "malloc: %lld bytes: %s\n", statbuf.st_size + 1, strerror(errno));
#else
			fprintf(stderr, "malloc: %ld bytes: %s\n", statbuf.st_size + 1, strerror(errno));
#endif
			return (1);
		}
		if (read(PrivKeyFD, PrivKey, nPrivKeyLen) != nPrivKeyLen) {
			fprintf(stderr, "%s: read: %s\n", strerror(errno), program);
			return (1);
		}
		close(PrivKeyFD);
		PrivKey[nPrivKeyLen] = '\0';
		if (DKIMSignInit(&ctxt, &opts) != DKIM_SUCCESS) {
			fprintf(stderr, "DKIMSignInit: failed to initialize signature %s\n", PrivKeyFile);
			return (1);
		}
		for (;;) {
			if ((ret = read(0, Buffer, sizeof(Buffer) - 1)) == -1) {
				fprintf(stderr, "read: %s\n", strerror(errno));
				DKIMSignFree(&ctxt);
				return (1);
			} else
			if (!ret)
				break;
			if (DKIMSignProcess(&ctxt, Buffer, ret) == DKIM_INVALID_CONTEXT) {
				fprintf(stderr, "DKIMSignProcess: DKIMContext structure invalid for this operation\n");
				DKIMSignFree(&ctxt);
				return (1);
			}
		}
		if (DKIMSignGetSig2(&ctxt, PrivKey, &pSig) == DKIM_INVALID_CONTEXT) {
			fprintf(stderr, "DKIMSignProcess: DKIMContext structure invalid for this operation\n");
			DKIMSignFree(&ctxt);
			return (1);
		}
		if (pSig) {
			fwrite(pSig, 1, strlen(pSig), stdout);
			fwrite("\n", 1, 1, stdout);
		}
		DKIMSignFree(&ctxt);
		return 0;
	} else { /*- verify */
		if (useADSP)
			vopts.nCheckPractices = useADSP;
		else
		if (useSSP)
			vopts.nCheckPractices = useSSP;
		else
			vopts.nCheckPractices = 0;
		vopts.nAccept3ps = accept3ps;
		vopts.pfnSelectorCallback = NULL;	/*- SelectorCallback; */
		vopts.nAllowUnsignedFromHeaders = nAllowUnsignedFromHeaders;
		vopts.nSubjectRequired = nAllowUnsignedSubject;
		DKIMVerifyInit(&ctxt, &vopts);		/*- this is always successful */
		for (;;) {
			if ((i = read(0, Buffer, sizeof(Buffer) - 1)) == -1) {
				fprintf(stderr, "read: %s\n", strerror(errno));
				DKIMVerifyFree(&ctxt);
				return (1);
			} else
			if (!i)
				break;
			ret = DKIMVerifyProcess(&ctxt, Buffer, i);
			dkim_error(ret);
			if (ret > 0 && ret < DKIM_FINISHED_BODY)
				ret = DKIM_FAIL;
			if (ret)
				break;
		}
		if (!ret) {
			ret = DKIMVerifyResults(&ctxt, &sCount, &sSize);
			if (ret != DKIM_SUCCESS && ret != DKIM_3PS_SIGNATURE && ret != DKIM_NEUTRAL)
				dkim_error(ret);
			if ((ret = DKIMVerifyGetDetails(&ctxt, &nSigCount, &pDetails, szPolicy)) != DKIM_SUCCESS)
				dkim_error(ret);
			else {
				for (ret = DKIM_FAIL, i = 0; i < nSigCount; i++) {
					if (verbose)
						printf("Signature # %02d: ", i + 1);
					if (pDetails[i].nResult >= 0) {
						ret = 0;
						if (verbose)
							printf("Success\n");
						continue;
					} else {
						if (ret == DKIM_FAIL)
							ret = pDetails[i].nResult;
						if (verbose)
							printf("Failure %d\n", pDetails[i].nResult);
					}
				}
				if (!nSigCount)
					ret = DKIM_NO_SIGNATURES;
			}
		} 
		if (ret < 0 || ret == DKIM_3PS_SIGNATURE) {
			if (useADSP) {
				char           *domain;
	
				if ((domain = DKIMVerifyGetDomain(&ctxt)))
					resDKIMADSP = GetADSP(domain);
				if (sCount > 0) {
					if (resDKIMADSP == DKIM_ADSP_UNKNOWN || resDKIMADSP == DKIM_ADSP_ALL)
						ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
				} 
				/* if the message should be signed, return fail */
				if (resDKIMADSP == DKIM_ADSP_DISCARDABLE)
					ret = DKIM_FAIL;
				ret = DKIM_NEUTRAL;
			} else
			if (useSSP) {
				int             bTestingPractices = 0;
				char           *domain;

				if ((domain = DKIMVerifyGetDomain(&ctxt)))
					resDKIMSSP = GetSSP(domain, &bTestingPractices);
				if (sCount > 0) {
					if ((resDKIMSSP == DKIM_SSP_UNKNOWN || resDKIMSSP == DKIM_SSP_ALL))
						ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
				}
				// if the SSP is testing, return neutral
				if (bTestingPractices)
					return(DKIM_NEUTRAL);
				/* if the message should be signed, return fail */
				if (resDKIMSSP == DKIM_SSP_ALL || resDKIMSSP == DKIM_SSP_STRICT)
					return(DKIM_FAIL);
				ret = DKIM_NEUTRAL;
			}
		}
		DKIMVerifyFree(&ctxt);
		writeHeader(ret, resDKIMSSP, resDKIMADSP, useSSP, useADSP);
		if ((dkimverify = getenv("DKIMVERIFY"))) {
			if (ret < 0) {
				if (dkimverify[str_chr(dkimverify, 'F' - ret)])
					ret = 14; /*- return permanent error */
				else
				if (dkimverify[str_chr(dkimverify, 'f' - ret)])
					ret = 88; /*- return temporary error */
				else
					ret = 0;
			} else {
				if (dkimverify[str_chr(dkimverify, 'A' + ret)])
					ret = 14; /*- return permanent error */
				else
				if (dkimverify[str_chr(dkimverify, 'a' + ret)])
					ret = 88; /*- return temporary error */
				else
					ret = 0;
			}
		}
		return (ret);
	}
	/*- Not Reached */
	_exit(0);
}

void
getversion_dkim_c()
{
	static char    *x = (char *) "$Id: dkim.cpp,v 1.26 2020-10-01 14:14:34+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
