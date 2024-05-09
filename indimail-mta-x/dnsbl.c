/*
 * $Log: dnsbl.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2022-06-01 13:00:54+05:30  Cprogrammer
 * skip loopback address from dnsbl check
 *
 * Revision 1.6  2017-12-27 00:32:44+05:30  Cprogrammer
 * fixed RCS id
 *
 * Revision 1.5  2017-12-26 15:18:08+05:30  Cprogrammer
 * use env variable to avoid using undefined variables when used as plugin
 *
 * Revision 1.4  2015-08-26 14:11:52+05:30  Cprogrammer
 * replaced ip_fmt() with ip4_fm()
 *
 * Revision 1.3  2015-04-14 20:02:01+05:30  Cprogrammer
 * skip dnsbl if ip address is unknown
 *
 * Revision 1.2  2011-07-29 09:28:12+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.1  2011-07-15 19:34:52+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include "smtp_plugin.h"
#include "stralloc.h"
#include "dns.h"
#include "env.h"
#include "str.h"
#include "control.h"

int             dnsblok, skipdnsbl = 0;
stralloc        dnsblhost = { 0 }, dnsbl_mesg = { 0 }, dnsbllist = { 0 };
const char     *dnsblFn;

void
out_of_mem(char **mesg)
{
	*mesg = (char *) "451 Requested action aborted: out of memory (#4.3.0)\r\n";
	return;
}

void
die_dnsbl(char **mesg)
{
	if (!stralloc_copys(&dnsbl_mesg,
		"421 your ip is currently blacklisted, try to auth first ("))
		out_of_mem(mesg);
	if (!stralloc_catb(&dnsbl_mesg, dnsblhost.s, dnsblhost.len - 1))
		out_of_mem(mesg);
	if (!stralloc_cats(&dnsbl_mesg, ") #5.7.1\r\n"))
		out_of_mem(mesg);
	if (!stralloc_0(&dnsbl_mesg))
		out_of_mem(mesg);
	*mesg = dnsbl_mesg.s;
	return;
}

/*-
 * dnsbl function
 * adapted from
 * http://qmail-dnsbl.sourceforge.net/
 * Author fabio.busatto@sikurezza.org
 */

int
dnsblcheck(char **mesg, const char *remoteip)
{
	const char     *ch;
	static stralloc dnsblbyte = { 0 }, dnsblrev = { 0 };
	static ipalloc  dnsblip = { 0 };
	char            x[IPFMT];

	if (!dnsblok) {
		skipdnsbl = 1;
		return (0);
	}
	ch = remoteip;
	if (!stralloc_copys(&dnsblrev, "")) {
		out_of_mem(mesg);
		return (1);
	}
	for (;;) {
		if (!stralloc_copys(&dnsblbyte, "")) {
			out_of_mem(mesg);
			return (1);
		}
		while (ch[0] && (ch[0] != '.')) {
			if (!stralloc_append(&dnsblbyte, ch)) {
				out_of_mem(mesg);
				return (1);
			}
			ch++;
		}
		if (!stralloc_append(&dnsblbyte, ".")) {
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_cat(&dnsblbyte, &dnsblrev)) {
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_copy(&dnsblrev, &dnsblbyte)) {
			out_of_mem(mesg);
			return (1);
		}
		if (!ch[0])
			break;
		ch++;
	}
	ch = dnsbllist.s;
	while (ch < (dnsbllist.s + dnsbllist.len)) {
		if (!stralloc_copy(&dnsblhost, &dnsblrev)) {
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_cats(&dnsblhost, ch)) {
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_0(&dnsblhost)) {
			out_of_mem(mesg);
			return (1);
		}
		if (!dns_ip(&dnsblip, &dnsblhost)) {
			ip4_fmt(x, &dnsblip.ix->addr.ip);
			skipdnsbl = 1;
			return 1;
		}
		while (*ch++);
	}
	skipdnsbl = 1;
	return 0;
}

static int
mailfrom_hook(const char *remoteip, const char *from, char **mesg)
{
	char           *x, *_relayclient;
	int             _authenticated;

	if (env_get("SKIPDNSBL")) {
		skipdnsbl = 1;
		return (0);
	}
	if (!str_diffn(remoteip, "unknown", 7) ||
			!str_diffn(remoteip, "::ffff:127.0.0.1", 17) ||
			!str_diffn(remoteip, "127.0.0.1", 10))
		return (0);
	if ((dnsblok = control_readfile(&dnsbllist,
			dnsblFn = ((x = env_get("DNSBLLIST")) && *x ? x : "dnsbllist"), 0)) == -1) {
		*mesg = (char *) "451 Requested action aborted: unable to read controls (#4.3.0)\r\n";
		return (1);
	}
	_relayclient = env_get("RELAYCLIENT");
	if (!(x = env_get("AUTHENTICATED")))
		_authenticated = 0;
	else
		_authenticated = *x == '1' ? 1 : 0;
	if ((!_authenticated && !_relayclient && !skipdnsbl) && dnsblcheck(mesg, remoteip)) {
		die_dnsbl(mesg);
		return (1);
	}
	return (0);
}

/*-
 * plugin_init() will be the function called by qmail-smtpd using
 * dlsym()
 */
PLUGIN         *
plugin_init()
{
	static PLUGIN   plug;
	PLUGIN         *ptr;

	ptr = &plug;
	ptr->mail_func = mailfrom_hook;
	ptr->rcpt_func = 0;
	ptr->data_func = 0;
	return &plug;
}

void
getversion_dnsbl_c()
{
	const char     *x = "$Id: dnsbl.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
	x++;
}
