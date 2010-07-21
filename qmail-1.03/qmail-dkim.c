/*
 * $Log: qmail-dkim.c,v $
 * Revision 1.23  2010-07-21 08:59:57+05:30  Cprogrammer
 * use CONTROLDIR environment variable instead of a hardcoded control directory
 *
 * Revision 1.22  2009-04-22 13:42:51+05:30  Cprogrammer
 * made fd for custom error configurable through env variable ERROR_FD
 *
 * Revision 1.21  2009-04-21 09:05:48+05:30  Cprogrammer
 * return relevant error message for reading private key
 *
 * Revision 1.20  2009-04-21 08:55:41+05:30  Cprogrammer
 * return temporary error for temp failures
 *
 * Revision 1.19  2009-04-20 22:19:01+05:30  Cprogrammer
 * made dkimopts global
 *
 * Revision 1.18  2009-04-16 13:48:32+05:30  Cprogrammer
 * added dkim_setoptions() to set all DKIM options
 *
 * Revision 1.17  2009-04-07 11:36:56+05:30  Cprogrammer
 * use TMPDIR env variable for tmp directory
 *
 * Revision 1.16  2009-04-05 12:52:17+05:30  Cprogrammer
 * added preprocessor warning
 *
 * Revision 1.15  2009-04-04 00:33:44+05:30  Cprogrammer
 * removed dk_strdup()
 *
 * Revision 1.14  2009-03-31 08:21:58+05:30  Cprogrammer
 * set dkimsign when RELAYCLIENT is defined when both dkimsign and dkimverify are undefined
 *
 * Revision 1.13  2009-03-30 22:25:54+05:30  Cprogrammer
 * made DKIM messages friendlier
 *
 * Revision 1.12  2009-03-30 14:47:59+05:30  Cprogrammer
 * added descriptive text for original dkim error
 *
 * Revision 1.11  2009-03-29 19:20:43+05:30  Cprogrammer
 * added nosignaturedomains
 *
 * Revision 1.10  2009-03-28 22:27:02+05:30  Cprogrammer
 * use DKIMSIGN, DKIMVERIFY if RELAYCLIENT is not set
 *
 * Revision 1.9  2009-03-28 22:03:05+05:30  Cprogrammer
 * fixed DKIM return codes
 *
 * Revision 1.8  2009-03-28 13:37:37+05:30  Cprogrammer
 * call DKIMVerifyGetDetails() always
 *
 * Revision 1.7  2009-03-28 11:39:23+05:30  Cprogrammer
 * set automatic setting of dkimsign, dkimverify variables based on RELAYCLIENT
 *
 * Revision 1.6  2009-03-28 11:35:58+05:30  Cprogrammer
 * added ADSP/SSP
 *
 * Revision 1.5  2009-03-22 17:39:38+05:30  Cprogrammer
 * set identity using basename of signature or environment variable DKIMIDENTITY
 *
 * Revision 1.4  2009-03-22 16:58:38+05:30  Cprogrammer
 * fixed bug with verification
 * report custom errors to qmail-queue through custom error interface
 *
 * Revision 1.3  2009-03-21 12:34:38+05:30  Cprogrammer
 * use hasdkim.h for conditional compilation of dkim
 *
 * Revision 1.2  2009-03-20 22:35:57+05:30  Cprogrammer
 * set error to DKIM_NO_SIGNATURE when DKIM-Signature is not present
 *
 * Revision 1.1  2009-03-18 13:54:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hasdkim.h"
#ifdef HASDKIM
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sgetopt.h"
#include "substdio.h"
#include "open.h"
#include "qmail.h"
#include "sig.h"
#include "scan.h"
#include "case.h"
#include "fmt.h"
#include "fd.h"
#include "alloc.h"
#include "str.h"
#include "stralloc.h"
#include "datetime.h"
#include "now.h"
#include "wait.h"
#include "auto_qmail.h"
#include "env.h"
#include "control.h"
#include "dkim.h"
#include "variables.h"

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR 1003
#define HAVE_EVP_SHA256
#define strncasecmp(x,y,z) case_diffb((x), (z), (y))
#define strcasecmp(x,y)    case_diffs((x), (y))

char            inbuf[2048];
char            outbuf[256];
char            errbuf[256];
struct substdio ssin;
struct substdio ssout;
struct substdio sserr;

datetime_sec    starttime;
struct datetime dt;
unsigned long   mypid;
unsigned long   uid;
char           *pidfn;
int             messfd;
int             readfd;

char          **MakeArgs(char *);
void            FreeMakeArgs(char **);

void
die(e)
	int             e;
{
	_exit(e);
}

void
die_write()
{
	die(53);
}

void
die_read()
{
	die(54);
}

void
sigalrm()
{
	/*- thou shalt not clean up here */
	die(52);
}

void
sigbug()
{
	die(81);
}

void
custom_error(char *flag, char *status, char *code)
{
	char           *c;

	if (substdio_put(&sserr, flag, 1) == -1)
		die_write();
	if (substdio_put(&sserr, "qmail-dkim: ", 12) == -1)
		die_write();
	if (substdio_puts(&sserr, status) == -1)
		die_write();
	if (code)
	{
		if (substdio_put(&sserr, " (#", 3) == -1)
			die_write();
		c = (*flag == 'Z') ? "4" : "5";
		if (substdio_put(&sserr, c, 1) == -1)
			die_write();
		if (substdio_put(&sserr, code + 1, 4) == -1)
			die_write();
		if (substdio_put(&sserr, ")", 1) == -1)
			die_write();
	}
	if (substdio_flush(&sserr) == -1)
		die_write();
	return;
}

int DKIM_CALL
SignThisHeader(const char *szHeader)
{
	if (strncasecmp((char *) szHeader, "X-", 2) == 0
		|| strncasecmp((char *) szHeader, "Received:", 9) == 0
		|| strncasecmp((char *) szHeader, "Authentication-Results:", 23) == 0
		|| strncasecmp((char *) szHeader, "Return-Path:", 12) == 0)
	{
		return 0;
	}
	return 1;
}

void
maybe_die_dkim(e)
	int             e;
{
	switch (e)
	{
	case DKIM_OUT_OF_MEMORY:
	case DKIM_BUFFER_TOO_SMALL:
		_exit (51);
	case DKIM_INVALID_CONTEXT:
		custom_error("Z", "DKIMContext structure invalid for this operation (#4.3.0)", 0);
		_exit(88);
	case DKIM_NO_SENDER:
		custom_error("Z", "Could not find From: or Sender: header in message (#5.1.7)", 0);
		_exit(88);
	case DKIM_BAD_PRIVATE_KEY:
		custom_error("D", "Could not parse private key (#5.7.5)", 0);
		_exit(88);
	default:
		return;
	}
}

unsigned int
pidfmt(s, seq)
	char           *s;
	unsigned long   seq;
{
	unsigned int    i;
	unsigned int    len;
	char           *tmpdir;

	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	len = 0;
	i = fmt_str(s, tmpdir);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "/qmail-dkim.");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, starttime);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, seq);
	len += i;
	if (s)
		s += i;
	++len;
	if (s)
		*s++ = 0;

	return len;
}

void
pidopen()
{
	unsigned int    len;
	unsigned long   seq;

	seq = 1;
	len = pidfmt((char *) 0, seq);
	if (!(pidfn = alloc(len)))
		die(51);
	for (seq = 1; seq < 10; ++seq)
	{
		if (pidfmt((char *) 0, seq) > len)
			die(81); /*- paranoia */
		pidfmt(pidfn, seq);
		if ((messfd = open_excl(pidfn)) != -1)
			return;
	}
	die(63);
}

char            tmp[FMT_ULONG];
DKIMContext     ctxt;
char           *dkimsign = 0;
char           *dkimverify = 0;
char           *dkimadspverify = 0, *dkimpractice =  "FGHIJKLMNPQRSTUVW";
stralloc        dkimoutput = { 0 };  /*- DKIM-Signature */
stralloc        dksignature = { 0 }; /*- content of private signature */
stralloc        sigdomains = { 0 };  /*- domains which must have signatures */
stralloc        nsigdomains = { 0 }; /*- domains which do not have signatures */
stralloc        dkimopts = { 0 };
char           *dkimqueue = 0;

static void
write_signature(char *domain, char *keyfn)
{
	char           *pSig;
	int             i;
	static stralloc keyfnfrom = { 0 };

	i = str_chr(keyfn, '%');
	if (keyfn[i])
	{
		if (!stralloc_copyb(&keyfnfrom, keyfn, i))
			die(51);
		if (!stralloc_cats(&keyfnfrom, domain))
			die(51);
		if (!stralloc_cats(&keyfnfrom, keyfn + i + 1))
			die(51);
	} else
	if (!stralloc_copys(&keyfnfrom, keyfn))
		die(51);
	if (!stralloc_0(&keyfnfrom))
		die(51);
	switch (control_readnativefile(&dksignature, keyfnfrom.s, 1))
	{
	case 0: /*- missing signature file */
		if (keyfn[i])
			return;
		die(35);
	case 1:
		break;
	default:
		custom_error("Z", "Unable to read private key. (#4.3.0)", 0);
		_exit(88);
	}
	for (i = 0; i < dksignature.len; i++)
	{
		if (dksignature.s[i] == '\0')
			dksignature.s[i] = '\n';
	}
	if (!stralloc_0(&dksignature))
		die(51);
	i = DKIMSignGetSig2(&ctxt, dksignature.s, &pSig);
	maybe_die_dkim(i);
	if (pSig)
	{
		if (!stralloc_catb(&dkimoutput, pSig, str_len(pSig)))
			die(51);
		if (!stralloc_cats(&dkimoutput, "\n"))
			die(51);
	}
	DKIMSignFree(&ctxt);
}

#include <openssl/evp.h>
#define DKIM_MALLOC(s)     OPENSSL_malloc(s)
#define DKIM_MFREE(s)      OPENSSL_free(s); s = NULL;
char           *dns_text(char *);

int
ParseTagValues(char *list, char *letters[], char *values[])
{
	char           *tmp, *ptr, *key;
	int             i;

	/*- start with all args unset */
	for (i = 0; letters[i]; i++)
		values[i] = 0;
	key = 0;
	for(ptr = list;*ptr;)
	{
		if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\r') || (*ptr == '\n')) /*- FWS */
			*ptr++ = 0;
		if (!key)
			key = ptr;
		if (*ptr == '=')
		{
			*ptr = 0;
			for (i = 0;letters[i];i++)
			{
				if (!str_diff(letters[i], key))
				{
					ptr++;
					for (;*ptr;)
					{
						if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\r') || (*ptr == '\n'))
						{
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
					for(;tmp != values[i];tmp--) /*- RFC 4871 3.2 */
					{
						if ((*tmp == ' ') || (*tmp == '\t') || (*tmp == '\r') || (*tmp == '\n'))
						{
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
checkSSP(char *domain, int *bTesting)
{
	char           *query, *results;
	char           *tags[] = { "dkim", "t", 0};
	char           *values[2];
	int             bIsParentSSP = 0, iSSP = DKIM_SSP_UNKNOWN;

	*bTesting = 0;
	if (!(query = DKIM_MALLOC(str_len("_ssp._domainkey.") + str_len(domain) + 1)))
		die(51);
	sprintf(query, "_ssp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!str_diff(results, "e=temp;"))
	{
		DKIM_MFREE(results);
		return DKIM_SSP_TEMPFAIL;
	} else
	if (!str_diff(results, "e=perm;"))
	{
		DKIM_MFREE(results);
		results = dns_text(domain);
		if (!str_diff(results, "e=temp;"))
		{
			DKIM_MFREE(results);
			return DKIM_SSP_TEMPFAIL;
		} else
		if (!str_diff(results, "e=perm;"))
		{
			DKIM_MFREE(results);
			return DKIM_SSP_SCOPE;
		}
		bIsParentSSP = 1;
	}
	if (!ParseTagValues(results, tags, values))
		return DKIM_SSP_UNKNOWN;
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
		for (p = values[1], s = values[1]; *p; p++)
		{
			if (*p == '|')
				*p = 0;
			else
				continue;
			if (!str_diff(s, "y"))
				*bTesting = 1;
			else
			if (!str_diff(s, "s")) {
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
	return iSSP;
}

int
checkADSP(char *domain)
{
	char           *query, *results;
	char           *tags[] = { "dkim", 0};
	char           *values[1];

	results = dns_text(domain);
	if (!str_diff(results, "e=perm;"))
	{
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	}
	else
	if (!str_diff(results, "e=temp;"))
	{
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	if (!(query = DKIM_MALLOC(str_len("_adsp._domainkey.") + str_len(domain) + 1)))
		die(51);
	sprintf(query, "_adsp._domainkey.%s", domain);
	results = dns_text(query);
	DKIM_MFREE(query);
	if (!str_diff(results, "e=perm;"))
	{
		DKIM_MFREE(results);
		return DKIM_ADSP_SCOPE;
	} else
	if (!str_diff(results, "e=temp;"))
	{
		DKIM_MFREE(results);
		return DKIM_ADSP_TEMPFAIL;
	}
	DKIM_MFREE(results);
	if (!ParseTagValues(results, tags, values))
		return DKIM_SSP_UNKNOWN;
	if (values[0] != NULL) {
		if (strcasecmp(values[0], "all") == 0)
			return (DKIM_ADSP_ALL);
		else
		if (strcasecmp(values[0], "discardable") == 0)
			return (DKIM_ADSP_DISCARDABLE);
	}
	return DKIM_ADSP_UNKNOWN; /*- No ADSP Record */
}

void
dkimverify_exit(int dkimRet, char *status, char *code)
{
	if (dkimRet < 0)
	{
		if (dkimverify[str_chr(dkimverify, 'F' - dkimRet)])
		{
			custom_error("D", status, code);
			die(88);
		}
		if (dkimverify[str_chr(dkimverify, 'f' - dkimRet)])
		{
			custom_error("Z", status, code);
			die(88);
		}
	} else
	{
		if (dkimverify[str_chr(dkimverify, 'A' + dkimRet)])
		{
			custom_error("D", status, code);
			die(88);
		}
		if (dkimverify[str_chr(dkimverify, 'a' + dkimRet)])
		{
			custom_error("Z", status, code);
			die(88);
		}
	}
}

void
writeHeaderNexit(int ret, int origRet, int resDKIMSSP, int resDKIMADSP, int useSSP, int useADSP)
{
	char           *dkimStatus = 0, *sspStatus = 0, *adspStatus = 0, *code = 0, *orig = 0;
	char            strnum[FMT_ULONG];

	switch (ret)
	{
	case DKIM_SUCCESS:			/*- 0 */ /*- A */
		dkimStatus = "good        ";
		code = "X.7.0";
		break;
	case DKIM_FINISHED_BODY:	/*- 1 process result: no more message body is needed */
		dkimStatus = "process result: no more message body is needed";
		code = "X.7.0";
		break;
	case DKIM_PARTIAL_SUCCESS:	/*- 2 verify result: at least one but not all signatures verified */
		dkimStatus = "verify result: at least none but not all signatures verified";
		code = "X.7.0";
		break;
	case DKIM_NEUTRAL:			/*- 3 verify result: no signatures verified but message is not suspicious */
		dkimStatus = "verify result: no signatures verified but message is not suspicious";
		code = "X.7.0";
		break;
	case DKIM_SUCCESS_BUT_EXTRA:/*- 4 signature result: signature verified but it did not include all of the body */
		dkimStatus = "signature result: signature verified but it did not include all of the body";
		code = "X.7.0";
		break;
	case DKIM_FAIL:				/*- -1 */ /*- F */
		dkimStatus = "DKIM Signature verification failed";
		code = "X.7.0";
		break;
	case DKIM_BAD_SYNTAX:		/*- -2 */ /*- G */
		dkimStatus = "signature error: DKIM-Signature could not parse or has bad tags/values";
		code = "X.7.5";
		break;
	case DKIM_SIGNATURE_BAD:	/*- -3 */
		dkimStatus = "signature error: RSA verify failed";
		code = "X.7.5";
		break;
	case DKIM_SIGNATURE_BAD_BUT_TESTING:
		dkimStatus = "signature error: RSA verify failed but testing";
		code = "X.7.5";
		break;
	case DKIM_SIGNATURE_EXPIRED:
		dkimStatus = "signature error: x= is old";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_INVALID:
		dkimStatus = "signature error: selector doesn't parse or contains invalid values";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_GRANULARITY_MISMATCH:
		dkimStatus = "signature error: selector g= doesn't match i=";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_KEY_REVOKED:
		dkimStatus = "signature error: selector p= empty";
		code = "X.7.5";
		break;
	case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG:
		dkimStatus = "signature error: selector domain name too long to request";
		code = "X.7.0";
		break;
	case DKIM_SELECTOR_DNS_TEMP_FAILURE:
		dkimStatus = "signature error: temporary dns failure requesting selector";
		code = "X.7.0";
		break;
	case DKIM_SELECTOR_DNS_PERM_FAILURE:
		dkimStatus = "signature error: permanent dns failure requesting selector";
		code = "X.7.0";
		break;
	case DKIM_SELECTOR_PUBLIC_KEY_INVALID:
		dkimStatus = "signature error: selector p= value invalid or wrong format";
		code = "X.7.5";
		break;
	case DKIM_NO_SIGNATURES:
		dkimStatus = "no signatures";
		code = "X.7.5";
		break;
	case DKIM_NO_VALID_SIGNATURES:
		dkimStatus = "no valid signatures";
		code = "X.7.5";
		break;
	case DKIM_BODY_HASH_MISMATCH:
		dkimStatus = "sigature verify error: message body does not hash to bh value";
		code = "X.7.7";
		break;
	case DKIM_SELECTOR_ALGORITHM_MISMATCH:
		dkimStatus = "signature error: selector h= doesn't match signature a=";
		code = "X.7.7";
		break;
	case DKIM_STAT_INCOMPAT:
		dkimStatus = "signature error: incompatible v=";
		code = "X.7.6";
		break;
	default:
		dkimStatus = "error";
		code = "X.3.0";
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
	if (useADSP && resDKIMADSP != -1)
	{
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
	if (!stralloc_copys(&dkimoutput, "DKIM-Status: "))
		die(51);
	if (!stralloc_cats(&dkimoutput, dkimStatus))
		die(51);
	if (origRet != DKIM_MAX_ERROR && ret != origRet)
	{
		if (!stralloc_cats(&dkimoutput, "\n\t(old="))
			die(51);
		switch (origRet)
		{
		case DKIM_SUCCESS:			/*- 0 */ /*- A */
			orig = "SUCCESS";
			break;
		case DKIM_FINISHED_BODY:	/*- 1 process result: no more message body is needed */
			orig = "FINISHED BODY";
			break;
		case DKIM_PARTIAL_SUCCESS:	/*- 2 verify result: at least one but not all signatures verified */
			orig = "PARTIAL SUCCESS";
			break;
		case DKIM_NEUTRAL:			/*- 3 verify result: no signatures verified but message is not suspicious */
			orig = "NEUTRAL";
			break;
		case DKIM_SUCCESS_BUT_EXTRA:/*- 4 signature result: signature verified but it did not include all of the body */
			orig = "SUCCESS(BUT EXTRA)";
			break;
		case DKIM_FAIL:				/*- -1 */ /*- F */
			orig = "FAIL";
			break;
		case DKIM_BAD_SYNTAX:		/*- -2 */ /*- G */
			orig = "BAD SYNTAX";
			break;
		case DKIM_SIGNATURE_BAD:	/*- -3 */
			orig = "SIGNATURE BAD";
			break;
		case DKIM_SIGNATURE_BAD_BUT_TESTING:
			orig = "SIGNATURE BAD (TESTING)";
			break;
		case DKIM_SIGNATURE_EXPIRED:
			orig = "SIGNATURE EXPIRED";
			break;
		case DKIM_SELECTOR_INVALID:
			orig = "SELECTOR INVALID";
			break;
		case DKIM_SELECTOR_GRANULARITY_MISMATCH:
			orig = "SELECTOR GRANULARITY MISMATCH";
			break;
		case DKIM_SELECTOR_KEY_REVOKED:
			orig = "SELECTOR KEY REVOKED";
			break;
		case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG:
			orig = "DOMAIN NAME TOO LONG";
			break;
		case DKIM_SELECTOR_DNS_TEMP_FAILURE:
			orig = "DNS TEMP FAILURE";
			break;
		case DKIM_SELECTOR_DNS_PERM_FAILURE:
			orig = "DNS PERM FAILURE";
			break;
		case DKIM_SELECTOR_PUBLIC_KEY_INVALID:
			orig = "PUBLIC KEY INVALID";
			break;
		case DKIM_NO_SIGNATURES:
			orig = "NO SIGNATURES";
			break;
		case DKIM_NO_VALID_SIGNATURES:
			orig = "NO VALID SIGNATURES";
			break;
		case DKIM_BODY_HASH_MISMATCH:
			orig = "BODY HASH MISMATCH";
			break;
		case DKIM_SELECTOR_ALGORITHM_MISMATCH:
			orig = "ALGORITHM MISMATCH";
			break;
		case DKIM_STAT_INCOMPAT:
			orig = "STAT INCOMPAT";
			break;
		default:
			orig = "Unkown error";
			break;
		}
		if (!stralloc_cats(&dkimoutput, orig))
			die(51);
		if (!stralloc_cats(&dkimoutput, ":"))
			die(51);
		if (origRet < 0)
		{
			if (!stralloc_cats(&dkimoutput, "-"))
				die(51);
			strnum[fmt_ulong(strnum, 0 - origRet)] = 0;
		} else
			strnum[fmt_ulong(strnum, origRet)] = 0;
		if (!stralloc_cats(&dkimoutput, strnum))
			die(51);
		if (!stralloc_cats(&dkimoutput, ")"))
			die(51);
	}
	if (!stralloc_cats(&dkimoutput, "\n"))
		die(51);
	if (useSSP && sspStatus) {
		if (!stralloc_cats(&dkimoutput, "X-DKIM-SSP: "))
			die(51);
		if (!stralloc_cats(&dkimoutput, sspStatus))
			die(51);
		if (!stralloc_cats(&dkimoutput, "\n"))
			die(51);
	}
	if (useADSP && adspStatus) {
		if (!stralloc_cats(&dkimoutput, "X-DKIM-ADSP: "))
			die(51);
		if (!stralloc_cats(&dkimoutput, adspStatus))
			die(51);
		if (!stralloc_cats(&dkimoutput, "\n"))
			die(51);
	}
	dkimverify_exit(ret, dkimStatus, code);
	return;
}

int
checkPractice(int dkimRet)
{
	char           *ptr;

	if (!(ptr = env_get("DKIMPRACTICE")))
		return (0);
	else
		dkimpractice = ptr;
	if (!*ptr)
	{
		if (dkimRet < 0 || dkimRet == DKIM_3PS_SIGNATURE)
			return (1);
		return (0);
	}
	if (dkimRet < 0)
	{
		if (dkimpractice[str_chr(dkimpractice, 'F' - dkimRet)])
			return (1);
		if (dkimpractice[str_chr(dkimpractice, 'f' - dkimRet)])
			return (1);
	} else
	{
		if (dkimpractice[str_chr(dkimpractice, 'A' + dkimRet)])
			return (1);
		if (dkimpractice[str_chr(dkimpractice, 'a' + dkimRet)])
			return (1);
	}
	return (0);
}

static char    *binqqargs[2] = { "bin/qmail-multi", 0 };

int
dkim_setoptions(DKIMSignOptions *opts, char *signOptions)
{
	int             ch, argc;
	char          **argv;

	opts->nIncludeBodyHash = DKIM_BODYHASH_IETF_1;
	opts->nCanon = DKIM_SIGN_RELAXED;					/*- c */
	opts->nIncludeBodyLengthTag = 0;					/*- l */
	opts->nIncludeQueryMethod = 0;						/*- q */
	opts->nIncludeTimeStamp = 0;						/*- t */
	opts->nIncludeCopiedHeaders = 0;					/*- h */
	opts->szIdentity[0] = '\0';
	opts->expireTime = starttime + 604800;	// expires in 1 week
	opts->nHash = DKIM_HASH_SHA1;
	str_copy(opts->szRequiredHeaders, "NonExistent");
	if (!signOptions)
		return (0);
	if (!stralloc_copys(&dkimopts, "dkim "))
		die(51);
	if (!stralloc_cats(&dkimopts, signOptions))
		die(51);
	if (!stralloc_0(&dkimopts))
		die(51);
	if (!(argv = MakeArgs(dkimopts.s)))
		die(51);
	for (argc = 0;argv[argc];argc++);
#ifdef HAVE_EVP_SHA256
	while ((ch = sgopt(argc, argv, "b:c:li:qthx:z:")) != sgoptdone)
#else
	while ((ch = sgopt(argc, argv, "b:c:li:qthx:")) != sgoptdone)
#endif
	{
		switch (ch)
		{
		case 'b':
			switch (*optarg)
			{
			case '1':
				opts->nIncludeBodyHash = DKIM_BODYHASH_ALLMAN_1;
				break;
			case '2':
				opts->nIncludeBodyHash = DKIM_BODYHASH_IETF_1;
				break;
			case '3':
				opts->nIncludeBodyHash = DKIM_BODYHASH_BOTH;
				break;
			default:
				FreeMakeArgs(argv);
				return (1);
			}
			break;
		case 'c':
			switch (*optarg)
			{
			case 'r':
				opts->nCanon = DKIM_SIGN_RELAXED;
				break;
			case 's':
				opts->nCanon = DKIM_SIGN_SIMPLE;
				break;
			case 't':
				opts->nCanon = DKIM_SIGN_RELAXED_SIMPLE;
				break;
			case 'u':
				opts->nCanon = DKIM_SIGN_SIMPLE_RELAXED;
				break;
			default:
				FreeMakeArgs(argv);
				return (1);
			}
			break;
		case 'l': /*- body length tag */
			opts->nIncludeBodyLengthTag = 1;
			break;
		case 'q': /*- query method tag */
			opts->nIncludeQueryMethod = 1;
			break;
		case 't': /*- timestamp tag */
			opts->nIncludeTimeStamp = 1;
			break;
		case 'h':
			opts->nIncludeCopiedHeaders = 1;
			break;
		case 'i':	/*- identity */
			if (*optarg == '-') /* do not use i= tag */
				opts->szIdentity[0] = '\0';
			else
				str_copyb(opts->szIdentity, optarg, sizeof(opts->szIdentity) - 1);
			break;
		case 'x': /*- expire time */
			if (*optarg == '-')
				opts->expireTime = 0;
			else
				opts->expireTime = starttime + atoi(optarg);
			break;
#ifdef HAVE_EVP_SHA256
		case 'z': /*- sign w/ sha1, sha256 or both */
			switch (*optarg)
			{
			case '1':
				opts->nHash = DKIM_HASH_SHA1;
				break;
			case '2':
				opts->nHash = DKIM_HASH_SHA256;
				break;
			case '3':
				opts->nHash = DKIM_HASH_SHA1_AND_256;
				break;
			default:
				FreeMakeArgs(argv);
				return (1);
			}
			break;
#endif
		default:
			FreeMakeArgs(argv);
			return (1);
		} /*- switch (ch) */
	} /*- while (1) */
	FreeMakeArgs(argv);
	return (0);
}

int
main(int argc, char *argv[])
{
	int             errfd, pim[2];
	int             wstat;
	int             resDKIMSSP = -1, resDKIMADSP = -1, useSSP = 0, useADSP = 0, accept3ps = 0;
	int             sCount = 0, sSize = 0;
	int             ret = 0, origRet = DKIM_MAX_ERROR, i, nSigCount = 0, len, token_len;
	unsigned long   pid;
	char           *selector, *p;
	stralloc        dkimfn = {0};
	DKIMSignOptions opts = { 0 };
	DKIMVerifyDetails *pDetails;
	DKIMVerifyOptions vopts = { 0 };

	starttime = now();
	sig_blocknone();
	umask(033);
	if (chdir(auto_qmail) == -1)
		die(61);
	if (!dkimsign)
		dkimsign = env_get("DKIMSIGN");
	if (!dkimverify)
		dkimverify = env_get("DKIMVERIFY");
	if (!dkimsign && !dkimverify && (p = env_get("RELAYCLIENT")))
	{
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&dkimfn, controldir))
			die(51);
		if (!stralloc_cats(&dkimfn, "/domainkeys/%/default"))
			die(51);
		if (!stralloc_0(&dkimfn))
			die(51);
		dkimsign = dkimfn.s;
	}
	dkimqueue = env_get("DKIMQUEUE");
	if (dkimqueue && *dkimqueue)
		binqqargs[0] = dkimqueue;
	if (!(p = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(p, &errfd);
	substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	if (dkimsign)
	{
		/* selector */
		p = dkimsign;
		selector = p;
		while (*p)
		{
			if (*p == '/' && *(p + 1))
				selector = p + 1;
			p++;
		}
		str_copyb(opts.szSelector, selector, sizeof(opts.szSelector) - 1);

		if (dkim_setoptions(&opts, env_get("DKIMSIGNOPTIONS")))
		{
			custom_error("Z", "Invalid DKIMSIGNOPTIONS (#4.3.0)", 0);
			_exit(88);
		}
		p = env_get("DKIMIDENTITY");
		if (p && *p)
			str_copyb(opts.szIdentity, p, sizeof(opts.szIdentity) - 1);
		p = env_get("DKIMEXPIRE");
		if (p && *p)
			opts.expireTime = starttime + atol(p);
		else
		if (p)
			opts.expireTime = 0;
		opts.pfnHeaderCallback = SignThisHeader;
		if (DKIMSignInit(&ctxt, &opts) != DKIM_SUCCESS) /*- failed to initialize signature */
		{
			custom_error("Z", "dkim initialization failed (#4.3.0)", 0);
			_exit(88);
		}
	} else
	{
		char           *x;

		if (!(x = env_get("SIGN_PRACTICE")))
			x = "adsp";
		if (!str_diffn("adsp", x, 4))
		{
			useADSP = 1;
			accept3ps = 1;
		}
		else
		if (!str_diffn("ssp", x, 3))
		{
			useSSP = 1;
			accept3ps = 1;
		} else
		if (!str_diffn("local", x, 5))
		{
			useSSP = 0;
			useADSP = 0;
			accept3ps = 1;
		}
		if (useADSP)
			vopts.nCheckPractices = useADSP;
		else
		if (useSSP)
			vopts.nCheckPractices = useSSP;
		else
			vopts.nCheckPractices = 0;
		vopts.nAccept3ps = accept3ps;
		vopts.pfnSelectorCallback = NULL;	/*- SelectorCallback; */
		DKIMVerifyInit(&ctxt, &vopts);		/*- this is always successful */
	}
	/*- Initialization */
	mypid = getpid();
	uid = getuid();
	datetime_tai(&dt, starttime);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	pidopen(); /*- fd = messfd */
	if ((readfd = open_read(pidfn)) == -1)
		die(63);
	if (unlink(pidfn) == -1)
		die(63);
	substdio_fdbuf(&ssout, write, messfd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf));
	for (;;)
	{
		register int    n;
		register char  *x;
		int             i, j;

		if ((n = substdio_feed(&ssin)) < 0)
			die_read();
		if (!n)
			break;
		x = substdio_PEEK(&ssin);
		if (dkimsign || dkimverify)
		{
			for (i = 0; i < n; i++)
			{
				if (x[i] == '\n')
				{
					if (dkimsign)
						j = DKIMSignProcess(&ctxt, "\r\n", 2);
					else
						j = DKIMVerifyProcess(&ctxt, "\r\n", 2);
				} else
				{
					if (dkimsign)
						j = DKIMSignProcess(&ctxt, x + i, 1);
					else
						j = DKIMVerifyProcess(&ctxt, x + i, 1);
				}
				maybe_die_dkim(j);
				if (j > 0 && j < DKIM_FINISHED_BODY)
					break;
				if (!ret && j)
					ret = j;
			}
		} /*- for(;;) */
		if (substdio_put(&ssout, x, n) == -1)
			die_write();
		substdio_SEEK(&ssin, n);
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	if (dkimsign || dkimverify)
	{
		if (dkimsign)
			write_signature(DKIMSignGetDomain(&ctxt), dkimsign);
		else
		if (dkimverify)
		{
			char            szPolicy[512];

			if (!ret)
			{
				if ((ret = DKIMVerifyResults(&ctxt, &sCount, &sSize)) != DKIM_SUCCESS)
					maybe_die_dkim(ret);
				if ((ret = DKIMVerifyGetDetails(&ctxt, &nSigCount, &pDetails, szPolicy)) != DKIM_SUCCESS)
					maybe_die_dkim(ret);
				else
				for (ret = DKIM_SUCCESS,i = 0; i < nSigCount; i++) {
					if (pDetails[i].nResult < 0)
					{
						ret = pDetails[i].nResult;
						break; /*- don't know if it is right to break */
					}
				}
				if (!nSigCount)
					ret = DKIM_NO_SIGNATURES;
			}
			if (checkPractice(ret)) {
				char           *domain;

				origRet = ret;
				if ((domain = DKIMVerifyGetDomain(&ctxt)))
				{
					if (!(p = env_get("SIGNATUREDOMAINS")))
					{
						if (control_readfile(&sigdomains, "signaturedomains", 0) == -1)
							die(55);
					} else
					if (!stralloc_copys(&sigdomains, p))
						die(51);
					for (len = 0, p = sigdomains.s;len < sigdomains.len;)
					{
						len += ((token_len = str_len(p)) + 1); /*- next domain */
						if (!case_diffb(p, token_len, domain))
						{
							ret = DKIM_FAIL;
							useADSP = 0;
							useSSP = 0;
							break;
						}
						p = sigdomains.s + len;
					}
					if (!(p = env_get("NOSIGNATUREDOMAINS")))
					{
						if (control_readfile(&nsigdomains, "nosignaturedomains", 0) == -1)
							die(55);
					} else
					if (!stralloc_copys(&nsigdomains, p))
						die(51);
					for (len = 0, p = nsigdomains.s;len < nsigdomains.len;)
					{
						len += ((token_len = str_len(p)) + 1); /*- next domain */
						if (*p == '*' || !case_diffb(p, token_len, domain))
						{
							ret = DKIM_NEUTRAL;
							useADSP = 0;
							useSSP = 0;
							break;
						}
						p = nsigdomains.s + len;
					}
				}
				if (!domain || !*domain)
					; /*- do nothing ? */
				else
				if (useADSP)
				{
					resDKIMADSP = checkADSP(domain);
					if (sCount > 0) {
						if (resDKIMADSP == DKIM_ADSP_UNKNOWN || resDKIMADSP == DKIM_ADSP_ALL)
							ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
					} 
					/* if the message should be signed, return fail */
					if (resDKIMADSP == DKIM_ADSP_DISCARDABLE)
						ret = DKIM_FAIL;
					else
						ret = DKIM_NEUTRAL;
				} else
				if (useSSP)
				{
					int             bTestingPractices = 0;
					char           *domain;
	
					if ((domain = DKIMVerifyGetDomain(&ctxt)))
						resDKIMSSP = checkSSP(domain, &bTestingPractices);
					if (sCount > 0) {
						if ((resDKIMSSP == DKIM_SSP_UNKNOWN || resDKIMSSP == DKIM_SSP_ALL))
							ret = (sCount == sSize ? DKIM_SUCCESS : DKIM_PARTIAL_SUCCESS);
					}
					// if the SSP is testing, return neutral
					if (bTestingPractices)
						ret = DKIM_NEUTRAL;
					/* if the message should be signed, return fail */
					if (resDKIMSSP == DKIM_SSP_ALL || resDKIMSSP == DKIM_SSP_STRICT)
						ret = DKIM_FAIL;
					else
						ret = DKIM_NEUTRAL;
				}
			}
			DKIMVerifyFree(&ctxt);
			writeHeaderNexit(ret, origRet, resDKIMSSP, resDKIMADSP, useSSP, useADSP);
		} /*- if (dkimverify) */
	}
	if (pipe(pim) == -1)
		die(59);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		die(58);
	case 0:
		close(pim[1]);
		if (fd_move(0, pim[0]) == -1)
			die(120);
		execv(*binqqargs, binqqargs);
		die(120);
	}
	close(pim[0]);
	substdio_fdbuf(&ssin, read, readfd, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, pim[1], outbuf, sizeof(outbuf));
	if (substdio_bput(&ssout, dkimoutput.s, dkimoutput.len) == -1) /*- write DKIM signature */
		die_write();
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2:
		die_read();
	case -3:
		die_write();
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	close(pim[1]);
	if (wait_pid(&wstat, pid) != pid)
		die(57);
	if (wait_crashed(wstat))
		die(57);
	die(wait_exitcode(wstat));
	/*- Not Reached */
	exit(0);
}
#else
#warning "not compiled with -DHASDKIM"
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
	substdio_puts(&sserr, "not compiled with -DHASDKIM\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif

void
getversion_qmail_dkim_c()
{
	static char    *x = "$Id: qmail-dkim.c,v 1.23 2010-07-21 08:59:57+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
