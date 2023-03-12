/*
 * RCS log at bottom
 * $Id: recipients.c,v 1.10 2023-03-13 00:09:59+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include "cdb.h"
#include "auto_break.h"
#include "case.h"
#include "uint32.h"
#include "byte.h"
#include "str.h"
#include "open.h"
#include "error.h"
#include "control.h"
#include "constmap.h"
#include "stralloc.h"
#include "recipients.h"
#include "sig.h"
#include "wait.h"
#include "substdio.h"
#include "fd.h"

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

extern ssize_t  safewrite(int, char *, int);

char            ssrcptbuf[512];
substdio        ssrcpt = SUBSTDIO_FDBUF(safewrite, FDAUTH, ssrcptbuf, sizeof (ssrcptbuf));

int
callapam(char *pam, char *addr)
{
	int             i, j = 0, wstat, child;
	int             pi[2];
	char            ch;
	static stralloc mailaddress = { 0 };
	char           *childargs[7] = { 0, 0, 0, 0, 0, 0, 0 };
	stralloc        pamarg = {0}, pamname = {0}, pamarg1 = {0},
					pamarg2 = {0}, pamarg3 = {0}, pamarg4 = {0},
					pamarg5 = {0};

	for (i = 0; (ch = pam[i]); i++) {
		if (j < 6) {
			if (ch != ' ')
				if (!stralloc_append(&pamarg, &ch))
					return -2;
			if (ch == ' ' || ch == '\n' || i == str_len(pam) - 1) {
				if (!stralloc_0(&pamarg))
					return -2;
				switch (j) {
				case 0:
					if (!stralloc_copy(&pamname, &pamarg))
						return -2;
					childargs[0] = pamname.s;
				case 1:
					if (!stralloc_copy(&pamarg1, &pamarg))
						return -2;
					childargs[1] = pamarg1.s;
				case 2:
					if (!stralloc_copy(&pamarg2, &pamarg))
						return -2;
					childargs[2] = pamarg2.s;
				case 3:
					if (!stralloc_copy(&pamarg3, &pamarg))
						return -2;
					childargs[3] = pamarg3.s;
				case 4:
					if (!stralloc_copy(&pamarg4, &pamarg))
						return -2;
					childargs[4] = pamarg4.s;
				case 5:
					if (!stralloc_copy(&pamarg5, &pamarg))
						return -2;
					childargs[5] = pamarg5.s;
				}
				j++;
				pamarg.len = 0;
			}
		}
	}
	childargs[j] = 0;
	close(FDAUTH);
	if (pipe(pi) == -1)
		return -3;
	if (pi[0] != FDAUTH)
		return -3;

	switch (child = fork())
	{
	case -1:
		return -3;
	case 0:
		close(pi[1]);
		if (fd_copy(FDAUTH, pi[0]) == -1)
			return -3;
		sig_pipedefault();
		execvp(childargs[0], childargs);
		return 111;
	}
	close(pi[0]);

	/*- checkpassword compliant form: address\0\0\0 */
	mailaddress.len = 0;
	if (!stralloc_copys(&mailaddress, addr) ||
			!stralloc_catb(&mailaddress, "\0\0\0", 3))
		return -2;

	substdio_fdbuf(&ssrcpt, write, pi[1], ssrcptbuf, sizeof ssrcptbuf);
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
		k = byte_chr(p, i - 1, '|'); /*- pam */
		if (j == (i - 1))
			j = 0;
		if (k == (i - 1))
			k = 0;
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
				p[j] = '\0';
			if (k > 0)
				p[k] = '\0';
			if (!str_diffn(p, "@", 1) &&
					!str_diffn(p + 1, rhost, rlen)) /*- exact */
				seenhost = 1;
			if (j > 0)
				p[j] = ':';
			if (k > 0)
				p[k] = '|';
		}
		if (!seenhost && p[0] != '@') { /*- sub domain */
			if (j > 0 && rlen >= j) {
				p[j] = '\0';
				if (rhost[rlen - j - 2] == '.' && !str_diffn(p, rhost + rlen - j - 1, j + 1))
					seenhost = 2;
				p[j] = ':';
			}
			if (k > 0 && rlen >= k) {
				p[k] = '\0';
				if (rhost[rlen - k - 2] == '.' && !str_diffn(p, rhost + rlen - k - 1, k + 1))
					seenhost = 3;
				p[k] = '|';
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

		if (k > 0 && k < i) { /*- pam */
			if (seenhost || !str_diffn(p, "*|", 2)) {
				r = callapam(p + k + 1, addr);
				if (vlen > 0 && r != 0)
					r = callapam(p + k + 1, vaddr);
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
	static char    *x = "$Id: recipients.c,v 1.10 2023-03-13 00:09:59+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: recipients.c,v $
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
