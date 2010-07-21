/*
 * $Log: qmail-dk.c,v $
 * Revision 1.29  2010-07-21 08:59:14+05:30  Cprogrammer
 * use CONTROLDIR environment variable instead of a hardcoded control directory
 *
 * Revision 1.28  2009-08-13 18:36:39+05:30  Cprogrammer
 * for verification continue in case of DK_SYNTAX errors
 *
 * Revision 1.27  2009-04-22 13:42:47+05:30  Cprogrammer
 * made fd for custom error configurable through env variable ERROR_FD
 *
 * Revision 1.26  2009-04-21 09:05:14+05:30  Cprogrammer
 * return relevant error message for reading private key
 *
 * Revision 1.25  2009-04-21 08:56:03+05:30  Cprogrammer
 * return temp errors for temporary failures
 *
 * Revision 1.24  2009-04-21 08:15:39+05:30  Cprogrammer
 * moved stralloc variable outside block
 *
 * Revision 1.23  2009-04-20 22:20:40+05:30  Cprogrammer
 * added DKSIGNOPTIONS
 *
 * Revision 1.22  2009-04-07 11:38:02+05:30  Cprogrammer
 * use TMPDIR env variable for tmp directory
 *
 * Revision 1.21  2009-04-05 12:52:10+05:30  Cprogrammer
 * added preprocessor warning
 *
 * Revision 1.20  2009-03-31 08:21:12+05:30  Cprogrammer
 * set dksign when RELAYCLIENT is defined when both dksign, dkverify are undefined
 *
 * Revision 1.19  2009-03-28 22:26:35+05:30  Cprogrammer
 * use DKSIGN,DKVERIFY env variables if RELAYCLIENT is not set
 *
 * Revision 1.18  2009-03-28 22:02:32+05:30  Cprogrammer
 * removed extra white space
 *
 * Revision 1.17  2009-03-28 11:34:51+05:30  Cprogrammer
 * BUG fix. corrected setting of dksign, dkverify variables
 *
 * Revision 1.16  2009-03-22 16:58:13+05:30  Cprogrammer
 * report custom errors to qmail-queue through custom error interface
 *
 * Revision 1.15  2009-03-21 15:15:56+05:30  Cprogrammer
 * improved logic
 *
 * Revision 1.14  2009-03-20 22:35:24+05:30  Cprogrammer
 * fix for multi-line headers
 *
 * Revision 1.13  2009-03-19 08:28:12+05:30  Cprogrammer
 * added EXCLUDEHEADERS
 *
 * Revision 1.12  2009-03-14 17:11:32+05:30  Cprogrammer
 * added DK_STAT_GRANULARITY
 *
 * Revision 1.11  2009-03-14 08:52:54+05:30  Cprogrammer
 * look for domainkey in control/domainkeys
 *
 * Revision 1.10  2005-08-23 17:33:37+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.9  2005-04-01 21:42:04+05:30  Cprogrammer
 * added DK_STAT_SYNTAX
 * changed error codes
 *
 * Revision 1.8  2004-11-02 09:15:53+05:30  Cprogrammer
 * commented out writing of Comments: header
 *
 * Revision 1.7  2004-10-24 21:32:00+05:30  Cprogrammer
 * removed unecessary header files
 *
 * Revision 1.6  2004-10-22 20:28:18+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:36:45+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-10-22 14:44:26+05:30  Cprogrammer
 * use control_readnativefile to avoid skipping signure with '#' char
 *
 * Revision 1.3  2004-10-20 20:08:53+05:30  Cprogrammer
 * libdomainkeys-0.62
 *
 * Revision 1.2  2004-09-23 22:55:32+05:30  Cprogrammer
 * removed uneccessary header files
 *
 * Revision 1.1  2004-08-28 01:02:16+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef DOMAIN_KEYS
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sgetopt.h"
#include "substdio.h"
#include "open.h"
#include "qmail.h"
#include "sig.h"
#include "fmt.h"
#include "fd.h"
#include "alloc.h"
#include "str.h"
#include "getln.h"
#include "case.h"
#include "stralloc.h"
#include "datetime.h"
#include "now.h"
#include "wait.h"
#include "auto_qmail.h"
#include "env.h"
#include "scan.h"
#include "mess822.h"
#include "control.h"
#include "variables.h"
#include "domainkeys.h"

#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */
#define ADDR 1003
#define ADVICE_BUF 2048

char            inbuf[2048];
struct substdio ssin;
char            outbuf[256];
struct substdio ssout;
struct substdio sserr;
char            errbuf[256];

datetime_sec    starttime;
struct datetime dt;
unsigned long   mypid;
unsigned long   uid;
char           *pidfn;
int             messfd;
int             readfd;
char           *dksign = 0;
char           *dkverify = 0;

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
	if (substdio_put(&sserr, "qmail-dk: ", 10) == -1)
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

void
maybe_die_dk(e)
	DK_STAT         e;
{
	switch (e)
	{
	case DK_STAT_NORESOURCE:
		_exit(51);
	case DK_STAT_INTERNAL:
		_exit(81);
	case DK_STAT_ARGS:
		custom_error("Z", "Arguments are not usable. (#4.3.5)", 0);
		_exit(88);
	case DK_STAT_SYNTAX:
		if (!dksign && dkverify)
			return;
		custom_error("Z", "Message is not valid syntax. Signature could not be created/checked (#4.6.0)", 0);
		_exit(88);
	case DK_STAT_CANTVRFY:
		custom_error("Z", "Cannot get domainkeys to verify signature (#5.7.5)", 0);
		_exit(88);
	case DK_STAT_BADKEY:
		if (env_get("DKVERIFY"))
			custom_error("Z", "Unusable public key for verifying (#5.7.5)", 0);
		else
			custom_error("Z", "Unusable private key for signing (#5.7.15", 0);
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
	i = fmt_str(s, "/qmail-dk.");
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
DK_LIB         *dklib;
DK             *dk;
DK_STAT         st;
stralloc        dkoutput = { 0 };    /*- Domainkey-Signature */
stralloc        dksignature = { 0 }; /*- content of private signature */
stralloc        dkopts = { 0 };
char           *dkqueue = 0;
char           *dkexcludeheaders;

static void
write_signature(DK *dk, char *dk_selector, char *keyfn,
	int advicelen, int opth, char *canon)
{
	unsigned char   advice[ADVICE_BUF];
	char           *selector, *from, *tmp;
	static stralloc keyfnfrom = { 0 };
	int             i;

	from = dk_from(dk);
	i = str_chr(keyfn, '%');
	if (keyfn[i])
	{
		if (!stralloc_copyb(&keyfnfrom, keyfn, i))
			die(51);
		if (!stralloc_cats(&keyfnfrom, from))
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
	case 0: /*- file not present */
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
	st = dk_getsig(dk, dksignature.s, advice, advicelen);
	maybe_die_dk(st);
	if (!dk_selector)
	{
		selector = keyfn;
		while (*keyfn)
		{
			if (*keyfn == '/')
				selector = keyfn + 1;
			keyfn++;
		}
	} else
		selector = dk_selector;
	if (!stralloc_cats(&dkoutput,
#if 0
		"Comment: DomainKeys? See http://antispam.yahoo.com/domainkeys\n"
#endif
		"DomainKey-Signature: a=rsa-sha1; q=dns; c="))
		die(51);
	if (!stralloc_cats(&dkoutput, canon))
		die(51);
	if (!stralloc_cats(&dkoutput, ";\n"))
		die(51);
	if (!stralloc_cats(&dkoutput, "    s="))
		die(51);
	if (!stralloc_cats(&dkoutput, selector))
		die(51);
	if (!stralloc_cats(&dkoutput, "; d="))
		die(51);
	if (from)
	{
		if (!stralloc_cats(&dkoutput, from))
			die(51);
	} else
	if (!stralloc_cats(&dkoutput, "unknown"))
		die(51);
	if (!stralloc_cats(&dkoutput, ";\n"))
		die(51);
	if (!stralloc_cats(&dkoutput, "    b="))
		die(51);
	if (!stralloc_cats(&dkoutput, (char *) advice))
		die(51);
	if (dkexcludeheaders || opth)
	{
		if ((i = dk_headers(dk, NULL)) > 0)
		{
			if (!(tmp = alloc(i)))
				die(51);
			if (!dk_headers(dk, tmp))
				die(51);
			if (!stralloc_cats(&dkoutput, ";\n    h="))
				die(51);
			if (!stralloc_cats(&dkoutput, tmp))
				die(51);
			alloc_free(tmp);
		}
	}
	if (!stralloc_cats(&dkoutput, ";\n"))
		die(51);
}

int
find_header(stralloc *line)
{
	static stralloc headers = { 0 };
	int             n, i, j;

	for (n = 0; n < line->len; ++n)
	{
		if (line->s[n] == ':')
			break;
	}
	if (n == line->len)
		return -1;
	if (!headers.len)
	{
		if (!stralloc_copys(&headers, ""))
			die(51);
		if (dkexcludeheaders)
		{
			if (!stralloc_cats(&headers, dkexcludeheaders))
				die(51);
			if (!stralloc_append(&headers, ":"))
				die(51);
		}
	}
	if (!headers.len)
		return 0;
	for (i = j = 0; i < headers.len; ++i)
	{
		if (headers.s[i] != ':')
			continue;
		if (i - j == n && !case_diffb(headers.s + j, n, line->s))
			return 1;
		j = i + 1;
	}
	return 0;
}

int
dk_setoptions(char **selector, int *advicelen, int *opth, int *optr, int *optc,
	char **canon, char *signOptions)
{
	char          **argv;
	int             ch, argc;

	*opth = 0;
	*optr = 0;
	*optc = DK_CANON_NOFWS;
	*canon = "nofws";
	*selector = 0;
	if (!signOptions)
		return (0);
	if (!stralloc_copys(&dkopts, "qmail-dk "))
		die(51);
	if (!stralloc_cats(&dkopts, signOptions))
		die(51);
	if (!stralloc_0(&dkopts))
		die(51);
	if (!(argv = MakeArgs(dkopts.s)))
		die(51);
	for (argc = 0;argv[argc];argc++);
	while ((ch = sgopt(argc, argv, "hrb:c:s:")) != sgoptdone)
	{
		switch (ch)
		{
		case 'h':
			*opth = 1;
			break;
		case 'r':
			*optr = 1;
			*opth = 1;
			break;
		case 'b':
			*advicelen = atoi(optarg);
			if (*advicelen > ADVICE_BUF);
				*advicelen = ADVICE_BUF;
			break;
		case 'c':
			if (!str_diffn("simple\0", optarg, 7))
			{
				*optc = DK_CANON_SIMPLE;
				*canon = "simple";
			}
			break;
		case 's':
			*selector = optarg;
			break;
		default:
			FreeMakeArgs(argv);
			return (1);
		}
	} /*- while ((ch = sgopt(argc, argv, "hrb:c:s:")) != sgoptdone) */
	FreeMakeArgs(argv);
	return (0);
}

static char    *binqqargs[2] = { "bin/qmail-multi", 0 };
int
main(int argc, char *argv[])
{
	int             errfd, pim[2];
	int             wstat, match, opth = 0, optr = 0, optc = DK_CANON_NOFWS,
					advicelen = ADVICE_BUF;
	char           *x, *relayclient, *canon = "nofws", *selector = 0; 
	stralloc        line = {0}, dkfn = {0};
	unsigned long   pid;

	sig_blocknone();
	umask(033);
	if (chdir(auto_qmail) == -1)
		die(61);
	if (!dksign)
		dksign = env_get("DKSIGN");
	if (!dkverify)
		dkverify = env_get("DKVERIFY");
	if (!dksign && !dkverify && (relayclient = env_get("RELAYCLIENT")))
	{
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&dkfn, controldir))
			die(51);
		if (!stralloc_cats(&dkfn, "/domainkeys/%/default"))
			die(51);
		if (!stralloc_0(&dkfn))
			die(51);
		dksign = dkfn.s;
	}
	dkqueue = env_get("DKQUEUE");
	if (dkqueue && *dkqueue)
		binqqargs[0] = dkqueue;
	if (dksign || dkverify)
	{
		if (!(dklib = dk_init(&st)))
		{
			maybe_die_dk(st);
			custom_error("Z", "dk initialization failed (#4.3.0)", 0);
			_exit(88);
		}
	}
	/*- Initialization */
	if (dksign)
	{
		if (dk_setoptions(&selector, &advicelen, &opth, &optr, &optc, &canon, env_get("DKSIGNOPTIONS")))
		{
			custom_error("Z", "Invalid DKSIGNOPTIONS (#4.3.0)", 0);
			_exit(88);
		}
		if (!(dk = dk_sign(dklib, &st, optc)))
		{
			maybe_die_dk(st);
			custom_error("Z", "dk_sign failed (#4.3.0)", 0);
			_exit(88);
		}
		if (optr && dk_setopts(dk, DKOPT_RDUPE) != DK_STAT_OK)
		{
			custom_error("Z", "DKOPT_RDUPE failed (#4.3.0)", 0);
			_exit(88);
		}
	} else
	if (dkverify)
	{
		if (!(dk = dk_verify(dklib, &st)))
		{
			maybe_die_dk(st);
			custom_error("Z", "dk_verify failed (#4.3.0)", 0);
			_exit(88);
		}
	}
	mypid = getpid();
	uid = getuid();
	starttime = now();
	datetime_tai(&dt, starttime);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	pidopen();
	if ((readfd = open_read(pidfn)) == -1)
		die(63);
	if (unlink(pidfn) == -1)
		die(63);
	substdio_fdbuf(&ssout, write, messfd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, read, 0, inbuf, sizeof(inbuf));
	if (!(x = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(x, &errfd);
	substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	dkexcludeheaders = env_get("DKEXCLUDEHEADERS");
	if (dkexcludeheaders)
	{
		int             hdr_continue, in_header = 1;

		hdr_continue = 0;
		for (;;)
		{
	
			if (getln(&ssin, &line, &match, '\n') == -1)
				die_read();
			if (!match && line.len == 0)
				break;
			if (substdio_put(&ssout, line.s, line.len) == -1)
				die_write();
			if (!dksign && !dkverify)
				continue;
			if (in_header && !mess822_ok(&line))
				in_header = 0;
			if (in_header)
			{
				if (line.s[0] == ' ' || line.s[0] == '\t')
				{
					if (hdr_continue)
						continue;
				} else
				if (find_header(&line) == 1) {
					hdr_continue = 1;
					continue;
				} else
					hdr_continue = 0;
			}
			if (match)
			{
				st = dk_message(dk, (unsigned char *) line.s, line.len - 1);
				maybe_die_dk(st);
				st = dk_message(dk, (unsigned char *) "\r\n", 2);
			} else
				st = dk_message(dk, (unsigned char *) line.s, line.len);
			maybe_die_dk(st);
		}
	} else
	for (;;)
	{
		register int    n;
		register char  *x;
		int             i;

		if ((n = substdio_feed(&ssin)) < 0)
			die_read();
		if (!n)
			break;
		x = substdio_PEEK(&ssin);
		if (dksign || dkverify)
		{
			for (i = 0; i < n; i++)
			{
				if (x[i] == '\n')
					st = dk_message(dk, (unsigned char *) "\r\n", 2);
				else
					st = dk_message(dk, (unsigned char *) x + i, 1);
				maybe_die_dk(st);
			}
		}
		if (substdio_put(&ssout, x, n) == -1)
			die_write();
		substdio_SEEK(&ssin, n);
	}
	if (substdio_flush(&ssout) == -1)
		die_write();
	if (dksign || dkverify)
	{
		st = dk_eom(dk, (void *) 0);
		maybe_die_dk(st);
		if (dksign)
			write_signature(dk, selector, dksign, advicelen, opth, canon);
		else
		if (dkverify)
		{
			char           *status = 0, *code = 0;

			if (!stralloc_copys(&dkoutput, "DomainKey-Status: "))
				die(51);
			switch (st)
			{
			case DK_STAT_OK:
				status = "good        ";
				break;
			case DK_STAT_BADSIG:
				status = "bad         ";
				code = "X.7.5";
				break;
			case DK_STAT_NOSIG:
				status = "no signature";
				code = "X.7.5";
				break;
			case DK_STAT_NOKEY:
			case DK_STAT_CANTVRFY:
				status = "no key      ";
				code = "X.7.0";
				break;
			case DK_STAT_BADKEY:
				status = "bad key     ";
				code = "X.7.5";
				break;
			case DK_STAT_INTERNAL:
				status = "bad format  ";
				code = "X.3.0";
				break;
			case DK_STAT_ARGS:
				status = "bad format  ";
				code = "X.3.5";
				break;
			case DK_STAT_SYNTAX:
				status = "bad format  ";
				code = "X.6.0";
				break;
			case DK_STAT_NORESOURCE:
				status = "no resources";
				code = "X.3.0";
				break;
			case DK_STAT_REVOKED:
				status = "revoked     ";
				code = "X.7.5";
				break;
			case DK_STAT_GRANULARITY:
				status = "bad sender  ";
				code = "X.7.5";
				break;
			}
			if (!stralloc_cats(&dkoutput, status))
				die(51);
			if (!stralloc_cats(&dkoutput, "\n"))
				die(51);
			if (dkverify[str_chr(dkverify, 'A' + st)])
			{
				custom_error("D", status, code); /*- return permanent error */
				die(88);
			}
			if (dkverify[str_chr(dkverify, 'a' + st)])
			{
				custom_error("Z", status, code); /*- return temporary error */
				die(88);
			}
		}
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
	if (substdio_bput(&ssout, dkoutput.s, dkoutput.len) == -1) /*- write DK signature */
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
#warning "not compiled with -DDOMAIN_KEYS"
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
	substdio_puts(&sserr, "not compiled with -DDOMAIN_KEYS\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif

void
getversion_qmail_dk_c()
{
	static char    *x = "$Id: qmail-dk.c,v 1.29 2010-07-21 08:59:14+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
