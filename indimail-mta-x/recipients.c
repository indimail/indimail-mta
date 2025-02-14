/*
 * $Id: recipients.c,v 1.13 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <cdb.h>
#include <case.h>
#include <uint32.h>
#include <byte.h>
#include <str.h>
#include <open.h>
#include <error.h>
#include <constmap.h>
#include <stralloc.h>
#include <sig.h>
#include <wait.h>
#include <fd.h>
#include <substdio.h>
#include <makeargs.h>
#include "auto_break.h"
#include "control.h"
#include "recipients.h"

#define FDAUTH 3

static stralloc key = { 0 };
static stralloc domain = { 0 };
static stralloc address = { 0 };
static stralloc rcptline = { 0 };
static stralloc vkey = { 0 };
static stralloc verp = { 0 };

static int      flagrcpts = 0;
static int      fdrcps;

/*- return -3: problem with PAM */
/*- return -2: out of memory */
/*- return -1: error reading control file */
/*- return  0: address not found; fatal */
/*- return  1: CDB lookup */
/*- return  2: PAM lookup */
/*- return  3: Wildcarded domain */
/*- return  4: Pass-thru */
/*- return 10: none existing control file; pass-thru */

extern ssize_t  safewrite(int, const char *, size_t);

char            ssrcptbuf[512];
substdio        ssrcpt = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) safewrite, FDAUTH, ssrcptbuf, sizeof (ssrcptbuf));

/*- pluggable address verification module */
int
pavm(char *pavm, char *addr)
{
	int             wstat, child;
	int             pi[2];
	static stralloc mailaddress = { 0 };
	char          **childargs;

	if (pipe(pi) == -1)
		return -3;

	switch (child = fork())
	{
	case -1:
		return -3;
	case 0:
		sig_pipedefault();
		close(pi[1]);
		if (fd_move(FDAUTH, pi[0]) == -1)
			_exit(-3);
		if (!(childargs = makeargs(pavm)))
			_exit(-2);
		execvp(childargs[0], childargs);
		_exit(-3);
	}
	close(pi[0]);

	/*- checkpassword compliant form: address\0\0\0 */
	mailaddress.len = 0;
	if (!stralloc_copys(&mailaddress, addr) ||
			!stralloc_catb(&mailaddress, "\0\0\0", 3))
		return -2;

	substdio_fdbuf(&ssrcpt, (ssize_t (*)(int,  char *, size_t)) write, pi[1], ssrcptbuf, sizeof ssrcptbuf);
	if (substdio_put(&ssrcpt, mailaddress.s, mailaddress.len) == -1 ||
			substdio_flush(&ssrcpt) == -1)
		return -3;
	close(pi[1]);
	if (wait_pid(&wstat, child) == -1 ||
			wait_crashed(wstat))
		return -3;
	return wait_exitcode(wstat);
}

int
recipients_init()
{
	if ((flagrcpts = control_readfile(&rcptline, "recipients", 0)) != 1)
		return flagrcpts;
	return 0;
}

static int
parse_recips(char *rhost, int rlen, char *addr, char *rkey, int klen, char *vaddr, char *v_key, int vlen)
{
	int             len, r, i, j, k, seenhost = 0;
	uint32          dlen;
	char           *p;

	for (len = 0, p = rcptline.s;len < rcptline.len;) {
		i = str_len(p) + 1;
		seenhost = 0;
		if (p[0] == '!') {
			if (!str_diffn(p + 1, rhost, rlen))
				return 3; /*- wildcard */
			if (!str_diffn(p, "!*", 2)) /*- pass-thru */
				return 4;
			p += i;
			len += i;
			continue;
		}
		j = byte_chr(p, i - 1, ':'); /*- cdb */
		k = byte_chr(p, i - 1, '|'); /*- pavm */
		if (j == (i - 1))
			j = 0; /*- ':' not present */
		if (k == (i - 1))
			k = 0; /*- '|' not present */
		if (!j && !k) {
			if ((fdrcps = open_read(p)) != -1) { /*- legacy cdb */
				r = cdb_seek(fdrcps, rkey, klen - 2, &dlen);
				if (vlen > 0 && r == 0)
					r = cdb_seek(fdrcps, v_key, vlen - 2, &dlen);
				close(fdrcps);
				if (r)
					return 1;
			}
			p += i;
			len += i;
			continue;
		}
		if (j > 0 || k > 0) {
			if (j > 0)
				p[j] = '\0'; /*- remove ':' */
			if (k > 0)
				p[k] = '\0'; /*- remove '|' */
			if (!str_diffn(p, "@", 1) &&
					!str_diffn(p + 1, rhost, rlen)) /*- exact */
				seenhost = 1;
			if (j > 0)
				p[j] = ':'; /*- restore ':' */
			if (k > 0)
				p[k] = '|'; /*- restore '|' */
		}
		if (!seenhost && p[0] != '@') { /*- sub domain */
			if (j > 0 && rlen >= j) { /*- cdb */
				p[j] = '\0'; /*- remove ':' */
				if (rhost[rlen - j - 2] == '.' && !str_diffn(p, rhost + rlen - j - 1, j + 1))
					seenhost = 2;
				p[j] = ':'; /*- restore ':' */
			}
			if (k > 0 && rlen >= k) { /*- pavm */
				p[k] = '\0'; /*- remove '|' */
				if (rhost[rlen - k - 2] == '.' && !str_diffn(p, rhost + rlen - k - 1, k + 1))
					seenhost = 3;
				p[k] = '|'; /*- restore '|' */
			}
		}

		if (j > 0 && j < i) { /*- cdb */
			if (seenhost || !str_diffn(p, "*:", 2)) {
				if ((fdrcps = open_read(p + j + 1)) != -1) {
					r = cdb_seek(fdrcps, rkey, klen - 2, &dlen);
					if (vlen > 0 && r == 0)
						r = cdb_seek(fdrcps, v_key, vlen - 2, &dlen);
					close(fdrcps);
					if (r)
						return 1; /*- CDB lookup succeeded */
				}
			}
		}

		if (k > 0 && k < i) { /*- pavm */
			if (seenhost || !str_diffn(p, "*|", 2)) {
				r = pavm(p + k + 1, addr);
				if (vlen > 0 && r != 0)
					r = pavm(p + k + 1, vaddr);
				if (r == 0)
					return 2; /*- PAM lookup succeeded */
				if (r == 111)
					return r;
			}
		}

		p += i;
		len += i;
	}
	return 0;
}

int
recipients(char *buf, int len)
{
	int             at, i, r;

	if (flagrcpts != 1)
		return 10;
	address.len = 0; /*- multiple recipients */
	domain.len = 0;
	at = byte_rchr(buf, len, '@');
	if (at < len) { /*- address has domain component */
		if (!stralloc_copyb(&domain, buf + at + 1, len - at - 1) ||
				!stralloc_copyb(&address, buf, len))
			return -2;
	} else {
		if (!stralloc_copyb(&address, buf, len) ||
				!stralloc_append(&address, "@") ||
				!stralloc_copys(&domain, "localhost") ||
				!stralloc_cat(&address, &domain))
			return -2;
	}
	if (!stralloc_0(&address) ||
			!stralloc_0(&domain))
		return -2;
	if (!stralloc_copys(&key, ":") ||
			!stralloc_cat(&key, &address) ||
			!stralloc_0(&key))
		return -2; /* \0\0 terminated */
	case_lowerb(key.s, key.len);
	case_lowerb(domain.s, domain.len);

	for (i = 0; i < at; i++) {
    	if (buf[i] == *auto_break || buf[i] == '=' || buf[i] == '+') { /* SRS delimiter */
			if (!stralloc_copyb(&verp, buf, i + 1) ||
					!stralloc_append(&verp, "@") ||
					!stralloc_cat(&verp, &domain) ||
					!stralloc_copys(&vkey, ":") ||
					!stralloc_cat(&vkey, &verp) ||
					!stralloc_0(&vkey))
				return -2;		/* \0\0 terminated */
			case_lowerb(vkey.s, vkey.len);
			break;
		}
	}

	if ((r = parse_recips(domain.s, domain.len, address.s, key.s, key.len, verp.s, vkey.s, vkey.len)))
		return r;
	return 0;
}

void
getversion_recipients_c()
{
	const char     *x = "$Id: recipients.c,v 1.13 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
	x = sccsidmakeargsh;
	x++;
}

/*
 * $Log: recipients.c,v $
 * Revision 1.13  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.12  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.11  2023-08-14 00:56:38+05:30  Cprogrammer
 * allow any number of arguments for pavm
 *
 * Revision 1.10  2023-03-13 00:09:59+05:30  Cprogrammer
 * fixed bug with string comparisions
 *
 * Revision 1.9  2023-03-12 19:10:57+05:30  Cprogrammer
 * refactored recipients extension
 *
 * Revision 1.8  2022-10-31 23:17:19+05:30  Cprogrammer
 * refactored code to collaps stralloc, substdio statements.
 * fix for SRS addresses
 *
 * Revision 1.7  2011-01-14 22:19:52+05:30  Cprogrammer
 * upgrade to verion 0.7 of EH's recipients extension
 *
 * Revision 1.6  2009-09-01 21:24:52+05:30  Cprogrammer
 * use break character from auto_break.c
 *
 * Revision 1.5  2009-09-01 20:38:15+05:30  Cprogrammer
 * update to RECIPIENTS extension 0.6 by Erwin Hoffmann
 *
 * Revision 1.4  2009-04-27 21:00:45+05:30  Cprogrammer
 * fix for formatting
 *
 * Revision 1.3  2009-04-10 13:31:20+05:30  Cprogrammer
 * upgrade to RECIPIENTS extension (0.5.19) (by Erwin Hoffman - http://www.fehcom.de/qmail/qmail.html##recipients)
 *
 * Revision 1.2  2004-10-22 20:29:56+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-22 23:27:38+05:30  Cprogrammer
 * Initial revision
 *
 */
