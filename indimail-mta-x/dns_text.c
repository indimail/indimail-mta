/*
 * $Log: dns_text.c,v $
 * Revision 1.12  2023-09-23 21:22:02+05:30  Cprogrammer
 * use ansic proto for functions
 *
 * Revision 1.11  2022-10-03 12:27:48+05:30  Cprogrammer
 * added dependency on stralloc.h
 *
 * Revision 1.10  2020-09-16 18:58:02+05:30  Cprogrammer
 * fix for FreeBSD
 *
 * Revision 1.9  2017-05-16 12:34:12+05:30  Cprogrammer
 * refactored dns_text() function
 *
 * Revision 1.8  2017-05-10 14:59:59+05:30  Cprogrammer
 * increase responselen to 1024 for long text records
 *
 * Revision 1.7  2014-01-29 14:00:24+05:30  Cprogrammer
 * fix for OS x
 *
 * Revision 1.6  2013-08-12 11:55:52+05:30  Cprogrammer
 * made dk_strdup() visible for both dk and dkim
 *
 * Revision 1.5  2009-06-11 15:20:57+05:30  Cprogrammer
 * port for DARWIN
 *
 * Revision 1.4  2009-04-05 12:51:41+05:30  Cprogrammer
 * made dns_text portable for dk, domainkeys
 *
 * Revision 1.3  2009-04-04 22:49:37+05:30  Cprogrammer
 * changed DKIM_MALLOC definition
 *
 * Revision 1.2  2009-04-04 00:33:27+05:30  Cprogrammer
 * added dk_strdup()
 *
 * Revision 1.1  2009-03-27 16:49:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stralloc.h>
#include <str.h>
#include <error.h>
#include <alloc.h>
#include "dns.h"
#include <openssl/evp.h>

char           *
dk_strdup(const char *s)
{
	char           *new = OPENSSL_malloc(str_len((char *) s) + 1);
	if (new != 0)
		str_copy(new, (char *) s);
	return new;
}

static struct
{
	unsigned char  *buf;
} response;
static int      responsebuflen = 0;
static int      responselen;
static unsigned char *responseend;
static unsigned char *responsepos;
static u_long   saveresoptions;
static int      (*lookup) () = res_query;
static int      numanswers;
static char     name[MAXDNAME];

static int
resolve(char *domain, int type)
{
	int             n;
	int             i;

	errno = 0;
	if (!responsebuflen) {
		if ((response.buf = (unsigned char *) alloc(PACKETSZ + 1)))
			responsebuflen = PACKETSZ + 1;
		else
			return DNS_MEM;
	}
	responselen = lookup(domain, C_IN, type, response.buf, responsebuflen);
	if ((responselen >= responsebuflen) || (responselen > 0 && (((HEADER *) response.buf)->tc))) {
		if (responsebuflen < 65536) {
			if (alloc_re((char *) &response.buf, responsebuflen, 65536))
				responsebuflen = 65536;
			else
				return DNS_MEM;
		}
		saveresoptions = _res.options;
		_res.options |= RES_USEVC;
		responselen = lookup(domain, C_IN, type, response.buf, responsebuflen);
		_res.options = saveresoptions;
	}
	if (responselen <= 0) {
		if (errno == ECONNREFUSED)
			return DNS_SOFT;
		if (h_errno == TRY_AGAIN)
			return DNS_SOFT;
		return DNS_HARD;
	}
	responseend = response.buf + responselen;
	responsepos = response.buf + sizeof(HEADER);
	n = ntohs(((HEADER *) response.buf)->qdcount);
	while (n-- > 0) {
		if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
			return DNS_SOFT;
		responsepos += i;
		i = responseend - responsepos;
		if (i < QFIXEDSZ)
			return DNS_SOFT;
		responsepos += QFIXEDSZ;
	}
	numanswers = ntohs(((HEADER *) response.buf)->ancount);
	return 0;
}

static stralloc txt = { 0 };
static stralloc result = { 0 };

static unsigned short
getshort(unsigned char *c)
{
	unsigned short  u;
	u = c[0];
	return (u << 8) + c[1];
}

static int
findtxt(int wanttype)
{
	unsigned short  rrtype;
	unsigned short  rrdlen;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;

	if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
		return DNS_SOFT;
	responsepos += i;

	if ((i = responseend - responsepos) < 4 + 3 * 2)
		return DNS_SOFT;

	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;

	if (rrtype == wanttype) {
		unsigned short  txtpos;
		unsigned char   txtlen;

		txt.len = 0;
		for (txtpos = 0; txtpos < rrdlen; txtpos += txtlen) {
			txtlen = responsepos[txtpos++];
			if (txtlen > rrdlen - txtpos)
				txtlen = rrdlen - txtpos;
			if (!stralloc_catb(&txt, (char *) &responsepos[txtpos], txtlen))
				return DNS_MEM;
		}

		responsepos += rrdlen;
		return 1;
	}

	responsepos += rrdlen;
	return 0;
}

static int
dns_txtplus(char *domain)
{
	int             r;

	switch (resolve(domain, T_TXT)) {
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	while ((r = findtxt(T_TXT)) != 2) {
		if (r == DNS_SOFT)
			return DNS_SOFT;
		if (r == 1) {
			if (!stralloc_cat(&result, &txt))
				return DNS_MEM;
		}
	}
	if (!stralloc_0(&result))
		return DNS_MEM;
	if (result.len)
		return (0);
	return DNS_HARD;
}

/*
 * we always return a null-terminated string which has been malloc'ed.  The string
 * is always in the tag=value form.  If a temporary or permanent error occurs,
 * the string will be exactly "e=perm;" or "e=temp;".
 * Note that it never returns NULL.
 */
char           *
dns_text(char *dn)
{
	int             r;
	
	switch (r = dns_txtplus(dn))
	{
	case DNS_MEM:
	case DNS_SOFT:
		return dk_strdup("e=temp;");
	case DNS_HARD:
		return dk_strdup("e=perm;");
	}
	return dk_strdup(result.s);
}

void
getversion_dns_text_c()
{
	const char     *x = "$Id: dns_text.c,v 1.12 2023-09-23 21:22:02+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
// vim: shiftwidth=2:tabstop=4:softtabstop=4
