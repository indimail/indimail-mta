/*
 * $Log: recipients.c,v $
 * Revision 1.10  2022-04-20 23:15:43+05:30  Cprogrammer
 * replaced multiple stralloc_0 with single stralloc_catb
 *
 * Revision 1.9  2020-05-11 11:11:36+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.8  2017-10-05 08:51:20+05:30  Cprogrammer
 * use /etc/indimail/users/recipients instead of /etc/indimail/control/recipients
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
#include <unistd.h>
#include "cdb.h"
#include "auto_break.h"
#include "auto_assign.h"
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
#include "env.h"

static stralloc key = { 0 };
static stralloc domain = { 0 };
static stralloc wildhost = { 0 };
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
substdio        ssrcpt = SUBSTDIO_FDBUF(safewrite, 3, ssrcptbuf, sizeof (ssrcptbuf));

int
callapam(pam, addr)
	char           *pam;
	char           *addr;
{
	int             i;
	int             j = 0;
	int             wstat;
	int             pi[2];
	int             child;
	char            ch;
	static stralloc mailaddress = { 0 };

	char           *childargs[7] = { 0, 0, 0, 0, 0, 0, 0 };
	stralloc        pamarg = { 0 };
	stralloc        pamname = { 0 };
	stralloc        pamarg1 = { 0 };
	stralloc        pamarg2 = { 0 };
	stralloc        pamarg3 = { 0 };
	stralloc        pamarg4 = { 0 };
	stralloc        pamarg5 = { 0 };

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

	close(3);
	if (pipe(pi) == -1)
		return -3;
	if (pi[0] != 3)
		return -3;

	switch (child = fork()) {
	case -1:
		return -3;
	case 0:
		close(pi[1]);
		if (fd_copy(3, pi[0]) == -1)
			return -3;
		sig_pipedefault();
		execvp(childargs[0], childargs);
		return 111;
	}
	close(pi[0]);

/*- checkpassword compliant form: address\0\0\0 */

	mailaddress.len = 0;
	if (!stralloc_copys(&mailaddress, addr))
		return -2;
	if (!stralloc_catb(&mailaddress, "\0\0\0", 3))
		return -2;

	substdio_fdbuf(&ssrcpt, write, pi[1], ssrcptbuf, sizeof ssrcptbuf);
	if (substdio_put(&ssrcpt, mailaddress.s, mailaddress.len) == -1)
		return -3;
	if (substdio_flush(&ssrcpt) == -1)
		return -3;
	close(pi[1]);

	if (wait_pid(&wstat, child) == -1)
		return -3;
	if (wait_crashed(wstat))
		return -3;
	return wait_exitcode(wstat);
}

int
recipients_init()
{
	static stralloc controlfile = {0};
	char           *assigndir;

	assigndir = (assigndir = env_get("ASSIGNDIR")) ? assigndir : auto_assign;
	if (!stralloc_copys(&controlfile, assigndir))
		return(-1);
	if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
		return(-1);
	if (!stralloc_catb(&controlfile, "recipients", 10))
		return(-1);
	if (!stralloc_0(&controlfile))
		return(-1);
	flagrcpts = control_readfile(&rcptline, controlfile.s, 0);
	if (flagrcpts != 1)
		return flagrcpts;
	return 0;
}

int
recipients_parse(char *rhost, int rlen, char *addr, char *rkey, int klen, char *vaddr, char *v_key, int vlen)
{
	int             i;
	int             r;
	int             j = 0;
	int             k = 0;
	uint32          dlen;
	static stralloc line = { 0 };
	int             seenhost = 0;

	wildhost.len = 0;
	line.len = 0;

	if (!stralloc_copys(&wildhost, "!"))
		return -2;
	if (!stralloc_cats(&wildhost, rhost))
		return -2;
	if (!stralloc_0(&wildhost))
		return -2;

	for (i = 0; i < rcptline.len; ++i) {
		if (!stralloc_append(&line, &rcptline.s[i]))
			return -2;
		if (rcptline.s[i] == '\0') {
			if (!stralloc_0(&line))
				return -2;

			j = byte_chr(line.s, line.len, ':');
			k = byte_chr(line.s, line.len, '|');
			if (!str_diffn(line.s, wildhost.s, wildhost.len - 1))
				return 3;	/*- wilddomain */
			if (j > 0 || k > 0)
				if (!str_diffn(line.s, "@", 1))	/*- exact */
					if (!str_diffn(line.s + 1, rhost, rlen - 1))
						seenhost = 1;
			if (!seenhost) {	/*- domain */
				if (j > 0 && rlen >= j)
					if (!str_diffn(line.s, rhost + rlen - j - 1, j - 1))
						seenhost = 2;
				if (k > 0 && rlen >= k)
					if (!str_diffn(line.s, rhost + rlen - k - 1, k - 1))
						seenhost = 3;
			}
			if (!seenhost)	/*- pass-thru */
				if (!str_diffn(line.s, "!*", 2))
					return 4;

			if (k > 0 && k < line.len)	/*- pam */
				if (seenhost || !str_diffn(line.s, "*", 1)) {
					r = callapam(line.s + k + 1, addr);
					if (vlen > 0 && r != 0)
						r = callapam(line.s + k + 1, vaddr);
					if (r == 0)
						return 2;
					if (r == 111)
						return r;
				}

			if (j > 0 && j < line.len)	/*- cdb */
				if (seenhost || !str_diffn(line.s, "*", 1)) {
					fdrcps = open_read(line.s + j + 1);
					if (fdrcps != -1) {
						r = cdb_seek(fdrcps, rkey, klen - 2, &dlen);
						if (vlen > 0 && r == 0)
							r = cdb_seek(fdrcps, v_key, vlen - 2, &dlen);
						close(fdrcps);
						if (r)
							return 1;
					}
				}

			if (!seenhost) {
				fdrcps = open_read(line.s);	/*- legacy cdb */
				if (fdrcps != -1) {
					r = cdb_seek(fdrcps, rkey, klen - 2, &dlen);
					if (vlen > 0 && r == 0)
						r = cdb_seek(fdrcps, v_key, vlen - 2, &dlen);
					close(fdrcps);
					if (r)
						return 1;
				}
			}

			line.len = 0;
		}
	}
	return 0;
}

int
recipients(buf, len)
	char           *buf;
	int             len;
{
	int             at;
	int             i;
	int             r;

	if (flagrcpts != 1)
		return 10;
	address.len = 0;			/*- multiple recipients */
	domain.len = 0;
	at = byte_rchr(buf, len, '@');
	if (at < len) {
		if (!stralloc_copyb(&domain, buf + at + 1, len - at - 1))
			return -2;
		if (!stralloc_copyb(&address, buf, len))
			return -2;
	} else {
		if (!stralloc_copyb(&address, buf, len))
			return -2;
		if (!stralloc_append(&address, "@"))
			return -2;
		if (!stralloc_copys(&domain, "localhost"))
			return -2;
		if (!stralloc_cat(&address, &domain))
			return -2;
	}
	if (!stralloc_0(&address))
		return -2;
	if (!stralloc_0(&domain))
		return -2;
	key.len = 0;
	if (!stralloc_copys(&key, ":"))
		return -2;
	if (!stralloc_cat(&key, &address))
		return -2;
	if (!stralloc_0(&key))
		return -2;				/* \0\0 terminated */
	case_lowerb(key.s, key.len);
	case_lowerb(domain.s, domain.len);
	vkey.len = 0;
	verp.len = 0;
	for (i = 0; i < at; i++) {
		if (buf[i] == *auto_break) {
			if (!stralloc_copyb(&verp, buf, i + 1))
				return -2;
			if (!stralloc_append(&verp, "@"))
				return -2;
			if (!stralloc_cat(&verp, &domain))
				return -2;
			if (!stralloc_copys(&vkey, ":"))
				return -2;
			if (!stralloc_cat(&vkey, &verp))
				return -2;
			if (!stralloc_0(&vkey))
				return -2;		/* \0\0 terminated */
			case_lowerb(vkey.s, vkey.len);
			break;
		}
	}

	r = recipients_parse(domain.s, domain.len, address.s, key.s, key.len, verp.s, vkey.s, vkey.len);
	if (r)
		return r;
	return 0;
}

void
getversion_recipients_c()
{
	static char    *x = "$Id: recipients.c,v 1.10 2022-04-20 23:15:43+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
