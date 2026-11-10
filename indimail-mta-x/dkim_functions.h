/*-
 * $Id: dkim_functions.h,v 1.2 2026-11-10 15:12:27+05:30 Cprogrammer Exp mbhangui $
 *
 * dkim_functions.h - DKIM (RFC 6376 / RFC 8463) signing and verification in C
 *
 * Replaces libdkim (C++). Needs OpenSSL >= 1.1.1 (Ed25519).
 *
 * Signing:
 *   s = dkim_sign_new(&opts);
 *   dkim_sign_process(s, buf, len);            -- as many times as you like
 *   dkim_sign_get(s, &opts1, pem1, &sig1);     -- once per signature wanted
 *   dkim_sign_get(s, &opts2, pem2, &sig2);     -- message is NOT re-read
 *   dkim_sign_free(s);
 *
 * Verification:
 *   v = dkim_verify_new(&vopts);
 *   dkim_verify_process(v, buf, len);
 *   dkim_verify_finish(v, &n, &details);       -- one detail per signature
 *   ret = dkim_verify_status(n, details);
 *   dkim_verify_free(v);
 */
#ifndef DKIM_FUNCTIONS_H
#define DKIM_FUNCTIONS_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-
 * Return codes. The values and the letters used by the DKIMVERIFY
 * environment variable (A = 0, B = 1 ... for >= 0; G = -1, H = -2 ...
 * for < 0) are unchanged from libdkim.
 */
#define DKIM_SUCCESS                       0 /*- A */
#define DKIM_FINISHED_BODY                 1 /*- B */
#define DKIM_PARTIAL_SUCCESS               2 /*- C */
#define DKIM_NEUTRAL                       3 /*- D */
#define DKIM_SUCCESS_BUT_EXTRA             4 /*- E signature OK but l= did not cover whole body */
#define DKIM_3PS_SIGNATURE                 5 /*- F verified, but d= is not the author's domain */
#define DKIM_FAIL                         -1 /*- G */
#define DKIM_BAD_SYNTAX                   -2 /*- H */
#define DKIM_SIGNATURE_BAD                -3 /*- I */
#define DKIM_SIGNATURE_BAD_BUT_TESTING    -4 /*- J */
#define DKIM_SIGNATURE_EXPIRED            -5 /*- K */
#define DKIM_SELECTOR_INVALID             -6 /*- L */
#define DKIM_SELECTOR_GRANULARITY_MISMATCH -7 /*- M */
#define DKIM_SELECTOR_KEY_REVOKED         -8 /*- N */
#define DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG -9 /*- O */
#define DKIM_SELECTOR_DNS_TEMP_FAILURE   -10 /*- P */
#define DKIM_SELECTOR_DNS_PERM_FAILURE   -11 /*- Q */
#define DKIM_SELECTOR_PUBLIC_KEY_INVALID -12 /*- R */
#define DKIM_NO_SIGNATURES               -13 /*- S */
#define DKIM_NO_VALID_SIGNATURES         -14 /*- T */
#define DKIM_BODY_HASH_MISMATCH          -15 /*- U */
#define DKIM_SELECTOR_ALGORITHM_MISMATCH -16 /*- V */
#define DKIM_STAT_INCOMPAT               -17 /*- W */
#define DKIM_UNSIGNED_FROM               -18 /*- X */
#define DKIM_BAD_IDENTITY                -19 /*- Y */
/* errors that have no DKIMVERIFY letter */
#define DKIM_OUT_OF_MEMORY               -20
#define DKIM_INVALID_CONTEXT             -21
#define DKIM_NO_SENDER                   -22
#define DKIM_BAD_PRIVATE_KEY             -23
#define DKIM_BUFFER_TOO_SMALL            -24
#define DKIM_EVP_SIGN_FAILURE            -25
#define DKIM_EVP_DIGEST_FAILURE          -26
#define DKIM_OPENSSL_TOO_OLD             -27 /*- OpenSSL < 1.1.1 on this system: no DKIM possible */
#define DKIM_MAX_ERROR                  -100 /*- "no error recorded" marker */

/* canonicalization: header/body */
#define DKIM_SIGN_SIMPLE                   0 /*- simple/simple   */
#define DKIM_SIGN_RELAXED                  1 /*- relaxed/relaxed */
#define DKIM_SIGN_RELAXED_SIMPLE           2 /*- relaxed header, simple body */
#define DKIM_SIGN_SIMPLE_RELAXED           3 /*- simple header, relaxed body */

/* algorithms */
#define DKIM_HASH_SHA1                     1 /*- rsa-sha1 */
#define DKIM_HASH_SHA256                   2 /*- rsa-sha256 */
#define DKIM_HASH_SHA1_AND_SHA256          3 /*- two signatures, same key */
#define DKIM_HASH_ED25519                  4 /*- ed25519-sha256 (RFC 8463) */

#define DKIM_MAX_VERIFY_SIGNATURES        10 /*- signatures beyond this are ignored */

/*-
 * Return 1 if the header is to be signed, 0 if not. 'header' is the whole
 * header field as found in the message ("Name: value", folded lines
 * joined with CRLF). From: is always signed whatever this returns.
 */
typedef int (*dkim_header_cb)(const char *header);

/*-
 * Returns a TXT record string (allocated, to be released with
 * OPENSSL_free()) for the name, or the literals "e=temp;" and "e=perm;"
 * for temporary and permanent DNS failures. indimail's dns_text() has
 * exactly this behaviour.
 */
typedef char *(*dkim_dns_cb)(const char *name);

typedef struct {
	/* used by dkim_sign_new() */
	dkim_header_cb  pfnHeaderCallback;     /*- may be NULL (sign everything) */
	char            szDomain[256];         /*- d=, overrides domain found in the message */
	char            szDefaultDomain[256];  /*- d= if none could be found in the message */
	/* used by dkim_sign_get(), one set per signature */
	int             nCanon;
	int             nHash;
	int             nIncludeBodyLengthTag; /*- l= */
	int             nIncludeQueryMethod;   /*- q= */
	int             nIncludeTimeStamp;     /*- t= */
	int             nIncludeCopiedHeaders; /*- z= */
	time_t          expireTime;            /*- x=, absolute; 0 = none */
	char            szSelector[256];       /*- s= */
	char            szIdentity[256];       /*- i=, empty = none */
} DKIMSignOptions;

typedef struct {
	int             nAllowUnsignedFromHeaders; /*- 0: every From: header must be in h= */
	int             nSubjectRequired;          /*- 1: Subject: must be in h= when present */
	int             nHonorBodyLengthTag;       /*- 1: hash only l= bytes of body */
	int             verbose;
	dkim_dns_cb     pfnDnsCallback;            /*- required unless pszSelectorOverride is set */
	const char     *pszSelectorOverride;       /*- use this key record instead of DNS */
} DKIMVerifyOptions;

typedef struct {
	int             nResult;                   /*- >= 0 verified, < 0 failed */
	char           *szDomain;                  /*- d= (NULL if unparsable) */
	char           *szSelector;                /*- s= */
	const char     *szAlgorithm;               /*- a= */
} DKIMVerifyDetails;

typedef struct DKIMSign   DKIMSign;
typedef struct DKIMVerify DKIMVerify;

/*- signing */
DKIMSign    *dkim_sign_new(const DKIMSignOptions *opts);
int          dkim_sign_process(DKIMSign *s, const char *buf, size_t len);
const char  *dkim_sign_domain(DKIMSign *s);   /*- d= that will be used; NULL if none */
/*-
 * *sig receives one or two (nHash == DKIM_HASH_SHA1_AND_SHA256) complete
 * "DKIM-Signature: ...\n" headers, each line terminated by LF.
 * Release with dkim_free(). Can be called any number of times.
 */
int          dkim_sign_get(DKIMSign *s, const DKIMSignOptions *opts, const char *privkey_pem, char **sig);
void         dkim_sign_free(DKIMSign *s);

/*- verifying */
DKIMVerify  *dkim_verify_new(const DKIMVerifyOptions *opts);
int          dkim_verify_process(DKIMVerify *v, const char *buf, size_t len);
int          dkim_verify_finish(DKIMVerify *v, int *nsigs, DKIMVerifyDetails **details);
const char  *dkim_verify_author_domain(DKIMVerify *v); /*- domain of first From: header */
void         dkim_verify_free(DKIMVerify *v);
/*-
 * One verified signature is enough: DKIM_SUCCESS. Otherwise the first
 * failure; DKIM_NO_SIGNATURES if there were none.
 */
int          dkim_verify_status(int nsigs, const DKIMVerifyDetails *details);

/*-
 * Built against OpenSSL < 1.1.1 (no Ed25519) this file still compiles, but
 * every function above fails with DKIM_OPENSSL_TOO_OLD (the ones that return
 * pointers return NULL). exit_if_old() lets a program check once, at start
 * up: it does _exit(code) when the library was built with an old OpenSSL
 * and returns 0 otherwise.
 */
int          exit_if_old(int code);

/*- helpers */
const char  *dkim_status_text(int code, const char **enhanced_code); /*- DKIM-Status: text */
const char  *dkim_code_name(int code);                                /*- "SIGNATURE BAD" etc */
void         dkim_free(void *p);

#ifdef __cplusplus
}
#endif
#endif

/*-
 * $Log: dkim_functions.h,v $
 * Revision 1.2  2026-11-10 15:12:27+05:30  Cprogrammer
 * fix compile (albeit with stub) on older openssl versions
 *
 * Revision 1.1  2026-11-09 23:21:24+05:30  Cprogrammer
 * Initial revision
 *
 */
