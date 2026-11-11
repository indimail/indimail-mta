/*
 * $Id: qmail-dkim.c,v 1.87 2026-11-11 19:26:59+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sgetopt.h>
#include <substdio.h>
#include <subfd.h>
#include <open.h>
#include <sig.h>
#include <scan.h>
#include <case.h>
#include <fmt.h>
#include <fd.h>
#include <alloc.h>
#include <str.h>
#include <stralloc.h>
#include <datetime.h>
#include <now.h>
#include <wait.h>
#include <env.h>
#include <getEnvConfig.h>
#include <error.h>
#include "dkim_functions.h"
#include <makeargs.h>
#include <noreturn.h>
#include <qprintf.h>
#include "control.h"
#include "auto_control.h"
#include "qmail.h"
#include "getDomainToken.h"
#include "variables.h"
#include "qmulti.h"
#include "pidopen.h"
#include "buffer_defs.h"
#include "custom_error.h"

#define strncasecmp(x,y,z) case_diffb((x), (z), (y))

extern char    *dns_text(char *); /*- TXT record, "e=temp;" or "e=perm;" on error */

static char    *
dns_lookup(const char *name)
{
	return dns_text((char *) name);
}

char            inbuf[BUFSIZE_IN];
char            outbuf[BUFSIZE_OUT];
struct substdio ssin;
struct substdio ssout;

datetime_sec    starttime;
struct datetime dt;
unsigned long   uid;
int             readfd;
DKIMSign       *sctx;
DKIMVerify     *vctx;
const char     *dkimsignoptions;

no_return void
die(int e, int what)
{
	if (!what)
		_exit(e);
	if (what == 1)
		dkim_sign_free(sctx);
	else
		dkim_verify_free(vctx);
	_exit(e);
}

no_return void
die_write()
{
	die(QQ_WRITE_ERR, 0);
}

no_return void
die_read()
{
	die(QQ_READ_ERR, 0);
}

no_return void
sigalrm(int x)
{
	(void) x;
	/*- thou shalt not clean up here */
	die(QQ_TIMEOUT, 0);
}

no_return void
sigbug(int x)
{
	(void) x;
	die(QQ_INTERNAL_BUG, 0);
}

/*
 * returns 1 if the header is to be signed.
 * EXCLUDE_DKIMSIGN is a colon separated list of header names. The names are
 * matched exactly (the old code did a prefix match, so excluding "To" also
 * excluded "Topic:").
 */
static int
SignThisHeader(const char *szHeader)
{
	const char     *excl, *p, *e;
	int             n;

	if ((!strncasecmp((char *) szHeader, "X-", 2) && strncasecmp((char *) szHeader, "X-Mailer:", 9))
			|| !strncasecmp((char *) szHeader, "Received:", 9)
			|| !strncasecmp((char *) szHeader, "Authentication-Results:", 23)
			|| !strncasecmp((char *) szHeader, "Arc-Authentication-Results:", 27)
			|| !strncasecmp(szHeader, "DKIM-Signature:", 15)
			|| !strncasecmp(szHeader, "DomainKey-Signature:", 20)
			|| !strncasecmp((char *) szHeader, "Return-Path:", 12))
		return 0;
	if (!(excl = env_get("EXCLUDE_DKIMSIGN")))
		return 1;
	for (p = excl; *p;) {
		while (*p == ':')
			p++;
		for (e = p; *e && *e != ':'; e++);
		if ((n = e - p) && !case_diffb(szHeader, n, p) && szHeader[n] == ':')
			return 0;
		p = e;
	}
	return 1;
}

void
maybe_die_dkim(int e)
{
	switch (e)
	{
	case DKIM_OUT_OF_MEMORY:
	case DKIM_BUFFER_TOO_SMALL:
		_exit (QQ_OUT_OF_MEMORY);
	case DKIM_INVALID_CONTEXT:
		custom_error("qmail-dkim", "Z", "DKIMContext structure invalid for this operation", 0, "X.3.0");
	case DKIM_NO_SENDER:
		custom_error("qmail-dkim", "Z", "Could not find From: or Sender: header in message", 0, "X.1.7");
	case DKIM_BAD_PRIVATE_KEY:
		custom_error("qmail-dkim", "D", "Could not parse private key", 0, "X.7.5");
	case DKIM_OPENSSL_TOO_OLD:
		custom_error("qmail-dkim", "Z", "OpenSSL version too old on this system", 0, "X.3.0");
	default:
		return;
	}
}
typedef const char c_char;
static c_char  *dkimsign = 0;
static c_char  *dkimverify = 0;
static c_char  *dkimpractice = 0;
static stralloc dkimoutput = { 0 };  /*- DKIM-Signature */
static stralloc dksignature = { 0 }; /*- content of private signature */
static stralloc sigdomains = { 0 };  /*- domains which must have signatures */
static stralloc nsigdomains = { 0 }; /*- domains which do not have signatures */
static stralloc dkimopts = { 0 };
static stralloc dkimkeys = { 0 };
static stralloc bouncehost = { 0 };

void
restore_gid()
{
	if (getegid() != getgid() && setgid(getgid()) == -1)
		custom_error("qmail-dkim", "Z", "unable to restore gid.", 0, "X.3.0");
}

const char     *
replace_pct(const char *keyfn, const char *domain, int pos, int *replace)
{
	const char     *p;
	char           *t, *s;
	int             i, d, r, len;
	static stralloc tmp = {0};

	if (!domain) {
		tmp.len = 0;
		for (p = keyfn, len = 0; *p; p++) {
			if (*p == '%') {
				if (tmp.len && *(p - 1) == '/' && *(p + 1) == '/') /*- replace // with single / */
					tmp.len--;
			} else
			if (!stralloc_append(&tmp, p))
				die(QQ_OUT_OF_MEMORY, 1);
		}
		if (!stralloc_0(&tmp))
			die(QQ_OUT_OF_MEMORY, 1);
		return tmp.s;
	}
	if (!keyfn[pos + 1]) { /*- file has % as the last component (implies selector is %) */
		len = pos + (d = fmt_str(0, domain));
		r = 0;
		if (replace)
			*replace = 1;
	} else
		len = pos + (d = fmt_str(0, domain)) + (r = fmt_str(0, keyfn + pos + 1));
	if (!(t = (char *) alloc((len + 1) * sizeof(char))))
		die(QQ_OUT_OF_MEMORY, 1);
	s = t;
	s += fmt_strn(t, keyfn, pos);
	s += fmt_strn(t + pos, domain, d);
	if (keyfn[pos + 1])
		s += fmt_strn(t + pos + d, keyfn + pos + 1, r);
	*s = 0;
	i = str_rchr(t, '%');
	if (t[i]) {
		p = replace_pct(t, domain, i, replace);
		alloc_free(t);
		return p;
	} else {
		if (!stralloc_copyb(&tmp, t, len + 1))
			die(QQ_OUT_OF_MEMORY, 1);
		alloc_free(t);
		return tmp.s;
	}
}

int
dkim_setoptions(DKIMSignOptions *opts, const char *signOptions)
{
	int             ch, argc;
	char          **argv;

	sgoptind = 1;
	sgoptpos = 0;
	opts->nCanon = DKIM_SIGN_RELAXED;					/*- c */
	opts->nIncludeBodyLengthTag = 0;					/*- l */
	opts->nIncludeQueryMethod = 0;						/*- q */
	opts->nIncludeTimeStamp = 0;						/*- t */
	opts->nIncludeCopiedHeaders = 0;					/*- h */
	opts->szIdentity[0] = '\0';
	opts->expireTime = starttime + 604800;	// expires in 1 week
	opts->nHash = DKIM_HASH_SHA256;
	if (!signOptions)
		return (0);
	if (!stralloc_copys(&dkimopts, "dkim ") ||
			!stralloc_cats(&dkimopts, signOptions) ||
			!stralloc_0(&dkimopts))
		die(QQ_OUT_OF_MEMORY, 0);
	if (!(argv = makeargs(dkimopts.s)))
		die(QQ_OUT_OF_MEMORY, 0);
	for (argc = 0;argv[argc];argc++);
	while ((ch = sgopt(argc, argv, "b:c:li:qthx:z:")) != sgoptdone) {
		switch (ch)
		{
		case 'b':
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
				free_makeargs(argv);
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
		case 'z': /*- sign rsa-sha1, rsa-sha256, rsa-sha1+rsa-sha256 or ed25519 */
			switch (*optarg)
			{
			case '1':
				opts->nHash = DKIM_HASH_SHA1;
				break;
			case '2':
				opts->nHash = DKIM_HASH_SHA256;
				break;
			case '3':
				opts->nHash = DKIM_HASH_SHA1_AND_SHA256;
				break;
			case '4':
				opts->nHash = DKIM_HASH_ED25519;
				break;
			default:
				free_makeargs(argv);
				return (1);
			}
			break;
		default:
			free_makeargs(argv);
			return (1);
		} /*- switch (ch) */
	} /*- while (1) */
	free_makeargs(argv);
	return (0);
}

static void
write_signature(const char *domain, DKIMSignOptions *opts, size_t selector_size)
{
	char           *pSig;
	const char     *keyfn, *ptr, *selector;
	unsigned int    i;
	int             r_selector, pct_found;
	static stralloc tmp = { 0 };

	if (env_get("NODKIMKEYS")) {
		i = 0;
		keyfn = dkimsign;
	} else {
		if ((int) (i = control_readfile(&dkimkeys, "dkimkeys", 0)) == -1)
			custom_error("qmail-dkim", "Z", "unable to read dkimkeys.", 0, "X.3.0");
		else
		if (!i || !(keyfn = getDomainToken(domain, &dkimkeys))) {
			i = 0;
			keyfn = dkimsign;
		} else { /*- has reading dkimkeys altered DKIMSIGNOPTIONS? */
			ptr = env_get("DKIMSIGN");
			/*- don't allow DKIMSIGN to be altered by dkimkeys */
			if (str_diff(ptr, keyfn) && !env_put2("DKIMSIGN", keyfn))
				die(QQ_OUT_OF_MEMORY, 1);
			ptr = env_get("DKIMSIGNOPTIONS");
			if (!dkimsignoptions || str_diff(dkimsignoptions, ptr)) {
				dkimsignoptions = ptr;
				if (dkim_setoptions(opts, dkimsignoptions))
					custom_error("qmail-dkim", "Z", "Invalid DKIMSIGNOPTIONS", 0, "X.3.0");
			}
		}
	}

	if (keyfn[0] != '/') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&tmp, controldir) ||
				!stralloc_append(&tmp, "/") ||
				!stralloc_cats(&tmp, keyfn))
			die(QQ_OUT_OF_MEMORY, 1);
	} else
	if (!stralloc_copys(&tmp, keyfn))
		die(QQ_OUT_OF_MEMORY, 1);
	if (!stralloc_0(&tmp))
		die(QQ_OUT_OF_MEMORY, 1);
	/*
	 * it is possible that selector was chosen based
	 * on DKIMSIGN.
	 * If the selector is % or keyfn has been overriden
	 * by dkimkeys control file, we need to change
	 * the selector
	 */
	r_selector = i ? 1 : 0; /*- for replacing selector later */
	i = str_rchr(tmp.s, '%');
	if (tmp.s[i]) {
		pct_found = 1;
		keyfn = replace_pct(tmp.s, domain, i, &r_selector);
		if (access(keyfn, F_OK)) {
			if (errno != error_noent && errno != error_notdir)
				custom_error("qmail-dkim", "Z", "unable to read private key.", 0, "X.3.0");
			keyfn = replace_pct(tmp.s, 0, 0, 0);
		}
	} else {
		pct_found = 0;
		keyfn = tmp.s;
	}
	switch (control_readnativefile(&dksignature, keyfn, 1))
	{
	case 0: /*- missing signature file */
		dkim_sign_free(sctx);
		sctx = 0;
		/*
		 * You may have multiple domains, but may chose to sign
		 * only for few domains which have the key present. Do not
		 * treat domains with missing key as an error.
		 */
		if (pct_found)
			return;
		custom_error("qmail-dkim", "Z", "unable to read private key.", 0, "X.3.0");
	case 1:
		restore_gid();
		break;
	default:
		dkim_sign_free(sctx);
		sctx = 0;
		if (errno == error_isdir && pct_found)
			return;
		custom_error("qmail-dkim", "Z", "unable to read private key.", 0, "X.3.0");
	}
	for (i = 0; i < dksignature.len; i++) {
		if (dksignature.s[i] == '\0')
			dksignature.s[i] = '\n';
	}
	if (!stralloc_0(&dksignature))
		die(QQ_OUT_OF_MEMORY, 1);
	if (r_selector) { /*- replace selector */
		selector = ptr = keyfn;
		while (*ptr) {
			if (*ptr == '/' && *(ptr + 1))
				selector = ptr + 1;
			ptr++;
		}
		str_copyb(opts->szSelector, selector, selector_size);
	}
	/*
	 * the message has been read once. dkim_sign_get() signs from the
	 * saved hashes with whatever options are in 'opts' right now.
	 * pSig holds complete DKIM-Signature header(s), each ending in LF.
	 */
	i = dkim_sign_get(sctx, opts, dksignature.s, &pSig);
	maybe_die_dkim(i);
	if (i) { /*- some other error, treat as temporary */
		dkim_sign_free(sctx);
		custom_error("qmail-dkim", "Z", dkim_status_text(i, 0), 0, "X.3.0");
	}
	if (pSig) {
		if (!stralloc_cats(&dkimoutput, pSig))
			die(QQ_OUT_OF_MEMORY, 1);
		dkim_free(pSig);
	}
	dkim_sign_free(sctx);
	sctx = 0;
}

void
dkimverify_exit(int dkimRet, const char *status, const char *code)
{
	if (dkimRet < 0) {
		if (dkimverify[str_chr(dkimverify, 'F' - dkimRet)])
			custom_error("qmail-dkim", "D", status, 0, code);
		if (dkimverify[str_chr(dkimverify, 'f' - dkimRet)])
			custom_error("qmail-dkim", "Z", status, 0, code);
	} else {
		if (dkimverify[str_chr(dkimverify, 'A' + dkimRet)])
			custom_error("qmail-dkim", "D", status, 0, code);
		if (dkimverify[str_chr(dkimverify, 'a' + dkimRet)])
			custom_error("qmail-dkim", "Z", status, 0, code);
	}
}

void
writeHeaderNexit(int ret, int origRet)
{
	const char     *dkimStatus, *code = 0;
	char            strnum[FMT_ULONG];

	dkimStatus = dkim_status_text(ret, &code);
	if (!stralloc_copys(&dkimoutput, "DKIM-Status: ") ||
			!stralloc_cats(&dkimoutput, dkimStatus))
		die(QQ_OUT_OF_MEMORY, 0);
	if (origRet != DKIM_MAX_ERROR && ret != origRet) {
		if (!stralloc_cats(&dkimoutput, "\n\t(old=") ||
				!stralloc_cats(&dkimoutput, dkim_code_name(origRet)) ||
				!stralloc_cats(&dkimoutput, ":"))
			die(QQ_OUT_OF_MEMORY, 0);
		if (origRet < 0) {
			if (!stralloc_cats(&dkimoutput, "-"))
				die(QQ_OUT_OF_MEMORY, 0);
			strnum[fmt_ulong(strnum, 0 - origRet)] = 0;
		} else
			strnum[fmt_ulong(strnum, origRet)] = 0;
		if (!stralloc_cats(&dkimoutput, strnum) ||
				!stralloc_cats(&dkimoutput, ")"))
			die(QQ_OUT_OF_MEMORY, 0);
	}
	if (!stralloc_cats(&dkimoutput, "\n"))
		die(QQ_OUT_OF_MEMORY, 0);
	dkimverify_exit(ret, dkimStatus, code);
	return;
}

/*
 * Is the signaturedomains / nosignaturedomains logic to be applied to this
 * verification result? Controlled by the letters in DKIMPRACTICE; if that is
 * not set the letters of DKIMVERIFY are used. An empty string means "any
 * failure". (ADSP and SSP have been removed, this is what remains of
 * SIGN_PRACTICE=local.)
 */
int
checkPractice(int dkimRet)
{
	char           *ptr;

	if (!(ptr = env_get("DKIMPRACTICE")))
		dkimpractice = dkimverify; /*- DKIMVERIFY env variable */
	else
		dkimpractice = ptr;
	if (!dkimpractice)
		return (0);
	if (!*dkimpractice) {
		if (dkimRet < 0 || dkimRet == DKIM_3PS_SIGNATURE)
			return (1);
		return (0);
	}
	if (dkimRet < 0) {
		if (dkimpractice[str_chr(dkimpractice, 'F' - dkimRet)])
			return (1);
		if (dkimpractice[str_chr(dkimpractice, 'f' - dkimRet)])
			return (1);
	} else {
		if (dkimpractice[str_chr(dkimpractice, 'A' + dkimRet)])
			return (1);
		if (dkimpractice[str_chr(dkimpractice, 'a' + dkimRet)])
			return (1);
	}
	return (0);
}

int
main(int argc, char *argv[])
{
	int             pim[2];
	int             wstat;
	int             verbose = 0;
	unsigned int    len, token_len;
	int             ret = 0, origRet = DKIM_MAX_ERROR, i, nSigCount = 0;
	pid_t           pid;
	unsigned long   death;
	const char     *selector = NULL, *ptr;
	stralloc        dkimfn = {0};
	DKIMVerifyDetails *pDetails;
	DKIMSignOptions   sopts;
	DKIMVerifyOptions vopts;

	exit_if_old(QQ_SYSTEM_MISCONFIG);
	memset(&sopts, 0, sizeof(sopts));
	memset(&vopts, 0, sizeof(vopts));
	starttime = now();
	sig_blocknone();
	umask(033);
	dkimsign = env_get("DKIMSIGN");
	dkimverify = env_get("DKIMVERIFY");
	ptr = (env_get("RELAYCLIENT") || env_get("AUTHINFO")) ? "" : 0;
	if (dkimverify && ptr && env_get("RELAYCLIENT_NODKIMVERIFY")) {
		restore_gid();
		return (qmulti("DKIMQUEUE", argc, argv));
	}
	if (!dkimsign && !dkimverify && ptr) {
		if (!(dkimsign = env_get("DKIMKEY"))) {
			if (!stralloc_copys(&dkimfn, "domainkeys/%/default") ||
					!stralloc_0(&dkimfn))
				die(QQ_OUT_OF_MEMORY, 0);
			dkimsign = dkimfn.s;
		}
	}
	if (dkimsign) {
		if (!(ptr = env_get("BOUNCEDOMAIN"))) {
			if ((ret = control_readfile(&bouncehost, "bouncehost", 1)) == -1)
				custom_error("qmail-dkim", "Z", "unable to read bouncehost.", 0, "X.3.0");
			else
			if (ret && !env_put2("BOUNCEDOMAIN", bouncehost.s))
				die(QQ_OUT_OF_MEMORY, 0);
		}
		/* selector */
		ptr = dkimsign;
		selector = ptr;
		while (*ptr) {
			if (*ptr == '/' && *(ptr + 1))
				selector = ptr + 1;
			ptr++;
		}
		str_copyb(sopts.szSelector, selector, sizeof(sopts.szSelector) - 1);

		if (dkim_setoptions(&sopts, dkimsignoptions = env_get("DKIMSIGNOPTIONS")))
			custom_error("qmail-dkim", "Z", "Invalid DKIMSIGNOPTIONS", 0, "X.3.0");
		ptr = env_get("DKIMIDENTITY");
		if (ptr && *ptr)
			str_copyb(sopts.szIdentity, ptr, sizeof(sopts.szIdentity) - 1);
		ptr = env_get("DKIMEXPIRE");
		if (ptr && *ptr)
			sopts.expireTime = starttime + atol(ptr);
		else
		if (ptr)
			sopts.expireTime = 0;
		sopts.pfnHeaderCallback = SignThisHeader;
		/*
		 * BOUNCEDOMAIN (or bouncehost/me control file) is used for d= when no
		 * domain can be found in the message. It may be user@domain or domain.
		 */
		if ((ptr = env_get("BOUNCEDOMAIN")) && *ptr) {
			const char     *at = ptr + str_chr(ptr, '@');

			at = *at ? at + 1 : ptr;
			str_copyb(sopts.szDefaultDomain, at, sizeof(sopts.szDefaultDomain) - 1);
		}
		if (!(sctx = dkim_sign_new(&sopts))) /*- failed to initialize signature */
			custom_error("qmail-dkim", "Z", "dkim initialization failed", 0, "X.3.0");
	} else {
		restore_gid();
		if (!dkimverify)
			dkimverify = "";
		/*
		 * SELECTOR_DATA: use this text instead of a DNS lookup for the public key
		 */
		vopts.pszSelectorOverride = env_get("SELECTOR_DATA");
		vopts.pfnDnsCallback = dns_lookup;
		if (env_get("UNSIGNED_FROM")) /*- accept signatures that do not cover every From: header */
			vopts.nAllowUnsignedFromHeaders = 1;
		vopts.nSubjectRequired = env_get("UNSIGNED_SUBJECT") ? 0 : 1;
		/*
		 * the man page says: set HONOR_BODYLENGTHTAG to honor l=. The old code
		 * did the opposite (honored l= unless the variable was set).
		 */
		vopts.nHonorBodyLengthTag = env_get("HONOR_BODYLENGTHTAG") ? 1 : 0;
		if (!(vctx = dkim_verify_new(&vopts)))
			custom_error("qmail-dkim", "Z", "dkim initialization failed", 0, "X.3.0");
	}
	/*- Initialization */
	uid = getuid();
	datetime_tai(&dt, starttime);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	getEnvConfiguLong(&death, "DEATH", DEATH);
	alarm(death);
	if (!(ptr = env_get("TMPDIR")))
		ptr = "/tmp";
	if ((ret = pidopen(starttime, ptr))) /*- set pidfn and open with fd = messfd */
		die(ret, 0);
	if ((readfd = open_read(pidfn)) == -1)
		die(QQ_PID_FILE, dkimsign ? 1 : 2);
	if (unlink(pidfn) == -1)
		die(QQ_PID_FILE, dkimsign ? 1 : 2);
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, messfd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, 0, inbuf, sizeof(inbuf)); /*- message content */
	for (ret = 0;;) {
		register int    n;
		register char  *x;

		if ((n = substdio_feed(&ssin)) < 0)
			die_read();
		if (!n)
			break;
		x = substdio_PEEK(&ssin);
		if (!ret) {
			ret = dkimsign ? dkim_sign_process(sctx, x, n) : dkim_verify_process(vctx, x, n);
			maybe_die_dkim(ret); /*- exits on out of memory */
			if (ret) /*- any other error: stop feeding the DKIM code, still pass the mail on */
				ret = DKIM_FAIL;
		}
		if (substdio_put(&ssout, x, n) == -1)
			die_write();
		substdio_SEEK(&ssin, n);
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	if (dkimsign || dkimverify) {
		if (dkimsign) {
			const char     *t = 0;

			if (ret || !(t = dkim_sign_domain(sctx))) { /*- ret != 0: DKIM code gave up on this message */
				dkim_sign_free(sctx);
				sctx = 0;
				maybe_die_dkim(DKIM_NO_SENDER);
			}
			write_signature(t, &sopts, sizeof(sopts.szSelector) - 1); /*- frees sctx */
		} else
		if (dkimverify) {
			restore_gid();
			if (!ret) {
				if (env_get("VERBOSE"))
					verbose = 1;
				if ((ret = dkim_verify_finish(vctx, &nSigCount, &pDetails)) != DKIM_SUCCESS)
					maybe_die_dkim(ret); /*- exits on out of memory */
				else {
					/*
					 * one result per DKIM-Signature header. One good signature is
					 * enough; otherwise the first failure is reported.
					 */
					if (verbose) {
						for (i = 0; i < nSigCount; i++) {
							subprintf(subfderr, "Signature # %02d: ", i + 1);
							if (pDetails[i].nResult >= 0)
								subprintf(subfderr, "Success\n");
							else
								subprintf(subfderr, "Failure %d\n", pDetails[i].nResult);
						}
						substdio_flush(subfderr);
					}
					ret = dkim_verify_status(nSigCount, pDetails);
				}
			}
			/*- what to do if DKIM Verification fails */
			if (checkPractice(ret)) {
				const char     *domain;
				int             skip_nosignature_domain = 0;

				origRet = ret;
				if ((domain = dkim_verify_author_domain(vctx))) {
					if (!(ptr = env_get("SIGNATUREDOMAINS"))) {
						if (control_readfile(&sigdomains, "signaturedomains", 0) == -1)
							custom_error("qmail-dkim", "Z", "unable to read signaturedomains.", 0, "X.3.0");
					} else
					if (!stralloc_copys(&sigdomains, ptr))
						die(QQ_OUT_OF_MEMORY, 2);
					for (len = 0, ptr = sigdomains.s;len < sigdomains.len;) {
						len += ((token_len = str_len(ptr)) + 1); /*- next domain */
						if (!case_diffb(ptr, token_len, domain)) {
							ret = origRet;
							skip_nosignature_domain = 1;
							break;
						}
						ptr = sigdomains.s + len;
					}
					/*
					 * not in signaturedomains: if the domain is listed in nosignaturedomains
					 * (or the list has *) a message without any DKIM-Signature is neutral.
					 * A message that has a signature which fails is still a failure.
					 */
					if (!skip_nosignature_domain && ret == DKIM_NO_SIGNATURES) {
						if (!(ptr = env_get("NOSIGNATUREDOMAINS"))) {
							if (control_readfile(&nsigdomains, "nosignaturedomains", 0) == -1)
								custom_error("qmail-dkim", "Z", "unable to read nosignaturedomains.", 0, "X.3.0");
						} else
						if (!stralloc_copys(&nsigdomains, ptr))
							die(QQ_OUT_OF_MEMORY, 2);
						for (len = 0, ptr = nsigdomains.s;len < nsigdomains.len;) {
							len += ((token_len = str_len(ptr)) + 1); /*- next domain */
							if (*ptr == '*' || !case_diffb(ptr, token_len, domain)) {
								ret = DKIM_NEUTRAL;
								break;
							}
							ptr = nsigdomains.s + len;
						}
					}
				}
			}
			dkim_verify_free(vctx);
			vctx = 0;
			writeHeaderNexit(ret, origRet);
		} /*- if (dkimverify) */
	}
	if (pipe(pim) == -1)
		die(QQ_PIPE_SOCKET, 0);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		die(QQ_FORK_ERR, 0);
	case 0:
		close(pim[1]);
		if (fd_move(0, pim[0]) == -1)
			die(QQ_DUP_ERR, 0);
		restore_gid();
		if (!(ptr = env_get("DKIMSIGNEXTRA")))
			return (qmulti("DKIMQUEUE", argc, argv));
		else {
			if (!env_put2("NODKIMKEYS", "qmail-dkim")) /* prevent recursive call of qmail-dkim */
				die(QQ_OUT_OF_MEMORY, 1);
			else
			if (!env_put2("DKIMSIGN", ptr))
				die(QQ_OUT_OF_MEMORY, 1);
			else
			if ((ptr = env_get("DKIMSIGNOPTIONSEXTRA")) && !env_put2("DKIMSIGNOPTIONS", ptr))
				die(QQ_OUT_OF_MEMORY, 1);
			if (!env_unset("DKIMSIGNEXTRA") || !env_unset("DKIMSIGNOPTIONSEXTRA"))
				die(QQ_OUT_OF_MEMORY, 1);
			execv(argv[0], argv);
			_exit(111);
		}
	}
	close(pim[0]);
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, readfd, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, pim[1], outbuf, sizeof(outbuf));
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
		die(QQ_WAITPID_SURPRISE, 0);
	if (wait_crashed(wstat))
		die(QQ_CRASHED, 0);
	die(wait_exitcode(wstat), 0);
}

#ifndef lint
void
getversion_qmail_dkim_c()
{
	const char     *x = "$Id: qmail-dkim.c,v 1.87 2026-11-11 19:26:59+05:30 Cprogrammer Exp mbhangui $";

	(void) x;
}
#endif

/*
 * $Log: qmail-dkim.c,v $
 * Revision 1.87  2026-11-11 19:26:59+05:30  Cprogrammer
 * Fix warnings from -Wextra
 *
 * Revision 1.86  2026-11-10 15:12:38+05:30  Cprogrammer
 * fix compile (albeit with stub) on older openssl versions
 *
 * Revision 1.85  2026-11-09 23:23:58+05:30  Cprogrammer
 * compile qmail-dkim using dns_functions.c instead of linking with libdkim2
 *
 * Revision 1.85
 * replaced libdkim (C++) with dkim_functions.c (C)
 * removed SSP/ADSP and SIGN_PRACTICE; dkimpractice now selects when
 * signaturedomains/nosignaturedomains are applied
 * sign once, call dkim_sign_get() once per signature
 * verify each DKIM-Signature independently; one good signature is enough
 * HONOR_BODYLENGTHTAG semantics fixed to match the man page
 * EXCLUDE_DKIMSIGN matches header names exactly
 *
 * Revision 1.84  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.83  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.82  2024-01-23 01:22:34+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.81  2024-01-10 23:01:23+05:30  Cprogrammer
 * reset sgoptind, sgoptpos for repeated calls to subgetopt
 *
 * Revision 1.80  2024-01-10 10:05:58+05:30  Cprogrammer
 * use bouncehost/me control file if BOUNCEDOMAIN is not set
 * set DKIMSIGN to private key from dkimkeys control file
 *
 * Revision 1.79  2024-01-09 19:18:44+05:30  Cprogrammer
 * added DKIM_BAD_IDENTITY with letter Y
 *
 * Revision 1.78  2024-01-06 21:33:37+05:30  Cprogrammer
 * use new error code DKIM_BAD_IDENTITY for invalid identity domain (i= tag)
 *
 * Revision 1.77  2023-12-25 09:30:05+05:30  Cprogrammer
 * made DEATH configurable
 *
 * Revision 1.76  2023-11-20 11:03:04+05:30  Cprogrammer
 * Added env variable EXCLUDE_DKIMSIGN to exclude headers from DKIM signing
 * exclude Arc-Authentication-Results header from DKIM signing
 *
 * Revision 1.75  2023-02-17 11:49:48+05:30  Cprogrammer
 * added env variable NODKIMKEYS to disable reading of dkimkeys control file
 * disable dkimkeys when doing DKIMSIGNEXTRA
 *
 * Revision 1.74  2023-02-12 18:37:37+05:30  Cprogrammer
 * use VERBOSE variable to turn on debug for signature verification status on fd 2
 *
 * Revision 1.73  2023-02-12 13:20:06+05:30  Cprogrammer
 * replaced exit code 70 with QQ_PID_FILE
 *
 * Revision 1.72  2023-02-01 18:15:33+05:30  Cprogrammer
 * use dkimkeys for setting env variables facilitating multi-signature generation with mixed encryption methods
 *
 * Revision 1.71  2023-01-30 10:42:16+05:30  Cprogrammer
 * set pfnSelectorCallback to dns_bypass if SELECTOR_DATA is set
 *
 * Revision 1.70  2023-01-29 22:37:42+05:30  Cprogrammer
 * fixed ed25519 signature
 * added DKIMSIGNEXTRA, DKIMSIGNOPTIONSEXTRA env variables to insert additional signature
 *
 * Revision 1.69  2023-01-26 22:59:19+05:30  Cprogrammer
 * removed setting redundant -b option
 * update verification message to include ED25519 failure
 *
 * Revision 1.68  2022-10-17 19:44:32+05:30  Cprogrammer
 * use exit codes defines from qmail.h
 *
 * Revision 1.67  2022-10-17 12:28:50+05:30  Cprogrammer
 * replace all '%' character with domain name
 *
 * Revision 1.66  2022-10-03 12:28:16+05:30  Cprogrammer
 * fixed return exit codes
 *
 * Revision 1.65  2022-10-02 22:21:48+05:30  Cprogrammer
 * fixed 'Private key file does not exist' for DKIMSIGN with '%'
 *
 * Revision 1.64  2022-04-03 18:44:21+05:30  Cprogrammer
 * refactored qmail_open() error codes
 *
 * Revision 1.63  2022-03-08 22:59:35+05:30  Cprogrammer
 * use custom_error() from custom_error.c
 *
 * Revision 1.62  2021-09-12 14:17:30+05:30  Cprogrammer
 * restore gid after reading private key file
 *
 * Revision 1.61  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.60  2021-08-28 23:16:06+05:30  Cprogrammer
 * control file dkimkeys for domain specific private key, selector
 *
 * Revision 1.59  2021-06-15 22:15:14+05:30  Cprogrammer
 * pass tmpdir argument to pidopen
 *
 * Revision 1.58  2021-06-15 11:53:44+05:30  Cprogrammer
 * moved pidopen out to its own file
 *
 * Revision 1.57  2021-06-09 21:14:33+05:30  Cprogrammer
 * use qmulti() instead of exec of qmail-multi
 *
 * Revision 1.56  2021-05-26 10:44:21+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.55  2020-05-11 11:06:35+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.54  2020-04-01 16:14:36+05:30  Cprogrammer
 * added header for makeargs() function
 *
 * Revision 1.53  2019-06-14 21:26:37+05:30  Cprogrammer
 * added env variable HONOR_BODYLENGTHTAG to honor body length tag during verification
 *
 * Revision 1.52  2019-02-18 22:18:12+05:30  Cprogrammer
 * allow DKIMVERIFY env variable in place of DKIMPRACTICE when SIGN_PRACTICE="local"
 *
 * Revision 1.51  2019-02-17 11:38:51+05:30  Cprogrammer
 * set original DKIM error for SIGN_PRACTICE=local
 *
 * Revision 1.50  2019-02-15 21:25:04+05:30  Cprogrammer
 * skip nosignaturedomains if domain is present in signaturedomains
 *
 * Revision 1.49  2018-08-08 23:58:01+05:30  Cprogrammer
 * issue success if at lease one one good signature is found
 *
 * Revision 1.48  2017-09-05 12:37:16+05:30  Cprogrammer
 * added missing DKIM_MFREE()
 *
 * Revision 1.47  2016-06-03 09:57:59+05:30  Cprogrammer
 * moved qmail-multi to sbin
 *
 * Revision 1.46  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.45  2016-03-01 18:48:02+05:30  Cprogrammer
 * added env variable UNSIGNED_SUBJECT to verify dkim without subject field
 *
 * Revision 1.44  2015-12-15 16:05:58+05:30  Cprogrammer
 * increased buffer size for long header issue
 *
 * Revision 1.43  2014-01-22 22:45:01+05:30  Cprogrammer
 * treat AUTHINFO environment like RELAYCLIENT environment variable
 *
 * Revision 1.42  2013-10-01 17:11:24+05:30  Cprogrammer
 * fixed QMAILQUEUE recursion
 *
 * Revision 1.41  2013-09-16 22:16:35+05:30  Cprogrammer
 * corrected logic for RELAYCLIENT_NODKIMVERIFY
 *
 * Revision 1.40  2013-09-13 16:34:35+05:30  Cprogrammer
 * turn off verification if RELAYCLIENT, DKIMVERIFY and RELAYCLIENT_NODKIMVERIFY is set
 *
 * Revision 1.39  2013-08-18 15:53:30+05:30  Cprogrammer
 * revert back to default verification mode if both dksign, dkverify are not set
 *
 * Revision 1.38  2013-08-17 15:00:33+05:30  Cprogrammer
 * BUG - corrected location of private key when % sign is removed
 *
 * Revision 1.37  2013-01-24 22:37:22+05:30  Cprogrammer
 * BUG (fix by Piotr Gronek) - DKIM_FREE(results) called before call to ParseTagValues()
 * alternate code for DKIMSIGN selector file name
 *
 * Revision 1.36  2012-08-16 08:01:46+05:30  Cprogrammer
 * do not skip X-Mailer headers
 *
 * Revision 1.35  2011-11-10 14:32:08+05:30  Cprogrammer
 * BUG ssout to be assigned only after pidopen
 *
 * Revision 1.34  2011-11-07 09:35:59+05:30  Cprogrammer
 * set ssout, sserr, ssin before executing other functions
 *
 * Revision 1.33  2011-07-29 09:29:17+05:30  Cprogrammer
 * fixed key file name
 *
 * Revision 1.32  2011-07-28 19:36:36+05:30  Cprogrammer
 * BUG - fixed opening of private key with absolute path
 *
 * Revision 1.31  2011-07-22 14:40:05+05:30  Cprogrammer
 * fixed checking of private key file
 *
 * Revision 1.30  2011-06-04 14:49:48+05:30  Cprogrammer
 * remove '%' sign from private key if key not found
 *
 * Revision 1.29  2011-06-04 14:22:29+05:30  Cprogrammer
 * added DKIM_UNSIGNED_FROM error code for dkimpractice
 *
 * Revision 1.28  2011-06-04 14:07:41+05:30  Cprogrammer
 * added DKIM_UNSIGNED_FROM
 *
 * Revision 1.27  2011-02-10 23:39:59+05:30  Cprogrammer
 * use DKIMKEY to override defult control/domainkeys/%/default
 *
 * Revision 1.26  2011-02-06 10:13:50+05:30  Cprogrammer
 * BUG - signature was wrongly freed before being accessed.
 *
 * Revision 1.25  2011-02-05 09:47:47+05:30  Cprogrammer
 * fixed SIGSEGV occuring for messages without body
 *
 * Revision 1.24  2010-11-02 18:45:14+05:30  Cprogrammer
 * Improve DKIM signing/verification speed
 *
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
