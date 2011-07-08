/*
 * $Log: dnsbl.c,v $
 * Revision 1.1  2011-07-08 13:45:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "smtp_plugin.h"
#include "stralloc.h"
#include "dns.h"
#include "env.h"
#include "control.h"

extern int      authenticated;
extern char    *relayclient;
int             dnsblok, flagdnsbl = 0;
stralloc        dnsblhost = { 0 }, dnsbl_mesg = { 0 }, dnsbllist = { 0 };
char           *skipdnsbl, *dnsblFn;

void
out_of_mem(char **mesg)
{
	*mesg = "451 Requested action aborted: out of memory (#4.3.0)\r\n";
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
 * dnsbl patch
 * Author fabio.busatto@sikurezza.org
 */

int
dnsblcheck(char **mesg, char *remoteip)
{
	char           *ch;
	static stralloc dnsblbyte = { 0 }, dnsblrev = { 0 };
	static ipalloc  dnsblip = { 0 };
	char            x[IPFMT];
	unsigned int    len;

	if (!dnsblok)
	{
		flagdnsbl = 1;
		return (0);
	}
	ch = remoteip;
	if (!stralloc_copys(&dnsblrev, ""))
	{
		out_of_mem(mesg);
		return (1);
	}
	for (;;) {
		if (!stralloc_copys(&dnsblbyte, ""))
		{
			out_of_mem(mesg);
			return (1);
		}
		while (ch[0] && (ch[0] != '.')) {
			if (!stralloc_append(&dnsblbyte, ch))
			{
				out_of_mem(mesg);
				return (1);
			}
			ch++;
		}
		if (!stralloc_append(&dnsblbyte, "."))
		{
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_cat(&dnsblbyte, &dnsblrev))
		{
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_copy(&dnsblrev, &dnsblbyte))
		{
			out_of_mem(mesg);
			return (1);
		}
		if (!ch[0])
			break;
		ch++;
	}
	ch = dnsbllist.s;
	while (ch < (dnsbllist.s + dnsbllist.len)) {
		if (!stralloc_copy(&dnsblhost, &dnsblrev))
		{
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_cats(&dnsblhost, ch))
		{
			out_of_mem(mesg);
			return (1);
		}
		if (!stralloc_0(&dnsblhost))
		{
			out_of_mem(mesg);
			return (1);
		}
		if (!dns_ip(&dnsblip, &dnsblhost))
		{
			len = ip_fmt(x, &dnsblip.ix->addr.ip);
			flagdnsbl = 1;
			return 1;
		} while (*ch++);
	}
	flagdnsbl = 1;
	return 0;
}

static int
from_plug_1(char *remoteip, char *from, char **mesg)
{
	char           *x;

	if ((skipdnsbl = env_get("SKIPDNSBL")))
		return (0);
	if ((dnsblok = control_readfile(&dnsbllist, dnsblFn = ((x = env_get("DNSBLLIST")) && *x ? x : "dnsbllist"), 0)) == -1)
	{
		*mesg = "451 Requested action aborted: unable to read controls (#4.3.0)\r\n";
		return (1);
	}
	if ((!authenticated && !relayclient && !flagdnsbl) && dnsblcheck(mesg, remoteip))
	{
		die_dnsbl(mesg);
		return (1);
	}
	return (0);
}

PLUGIN         *
plugin_init()
{
	static PLUGIN   plug;
	PLUGIN         *ptr;

	ptr = &plug;
	ptr->mail_func = from_plug_1;
	ptr->rcpt_func = 0;
	ptr->data_func = 0;
	return &plug;
}

void
getversion_dnsbl_c()
{
	static char    *x = "$Id: dnsbl.c,v 1.1 2011-07-08 13:45:44+05:30 Cprogrammer Exp mbhangui $";
	x++;
}
