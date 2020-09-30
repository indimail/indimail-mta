/*
 * $Log: dns.cpp,v $
 * Revision 1.12  2020-09-30 20:46:38+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.11  2019-05-21 22:37:42+05:30  Cprogrammer
 * fixed pointers after realloc
 *
 * Revision 1.10  2017-09-05 11:01:06+05:30  Cprogrammer
 * removed unused variables
 *
 * Revision 1.9  2017-09-01 12:46:27+05:30  Cprogrammer
 * fixed double free() of dnresult
 *
 * Revision 1.8  2017-08-09 22:06:33+05:30  Cprogrammer
 * fixed resolve() function
 *
 * Revision 1.7  2017-05-16 12:40:23+05:30  Cprogrammer
 * refactored dns_text() function
 *
 * Revision 1.6  2017-05-10 14:58:06+05:30  Cprogrammer
 * increase responselen to 1024 for long text records
 *
 * Revision 1.5  2017-05-10 12:27:49+05:30  Cprogrammer
 * use packetsize > 512 to avoid dkim failures for sites having long txt records (hotmail.com)
 *
 * Revision 1.4  2009-06-11 13:58:39+05:30  Cprogrammer
 * port for DARWIN
 *
 * Revision 1.3  2009-03-27 19:22:45+05:30  Cprogrammer
 * dns functions
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#ifdef NAMESER8_COMPAT_H
#include <nameser8_compat.h>
#endif
#ifdef ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#include <resolv.h>
#include <string.h>
#include "dns.h"

static unsigned short
getshort(unsigned char *cp)
{
	return (cp[0] << 8) | cp[1];
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
static int      numanswers;
static char     name[MAXDNAME];

static int
resolve(char *domain, int type)
{
	int             n, i;
	unsigned char  *ptr;

	errno = 0;
	if (!responsebuflen) {
		if ((response.buf = (unsigned char *) malloc(PACKETSZ + 1)))
			responsebuflen = PACKETSZ + 1;
		else
			return DNS_MEM;
	}
	responselen = res_query(domain, C_IN, type, response.buf, responsebuflen);
	if ((responselen >= responsebuflen) || (responselen > 0 && (((HEADER *) response.buf)->tc))) {
		if (responsebuflen < 65536) {
			if ((ptr = (unsigned char *) realloc((void *) response.buf, 65536))) {
				response.buf = ptr;
				responsebuflen = 65536;
			} else {
				free(response.buf);
				responsebuflen = 0;
				return DNS_MEM;
			}
		}
		saveresoptions = _res.options;
		_res.options |= RES_USEVC;
		responselen = res_query(domain, C_IN, type, response.buf, responsebuflen);
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

void
byte_copy(register char *to, register unsigned int n, register char *from)
{
	for (;;) {
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
		if (!n)
			return;
		*to++ = *from++;
		--n;
	}
}

static char     *txt, *dnresult;
static int      txtlen, dnresultlen;

static int
findtxt(int wanttype, int *txt_strlen)
{
	unsigned short  rrtype, rrdlen;
	char           *ptr;
	int             i;

	if (numanswers <= 0)
		return 2;
	--numanswers;
	if (responsepos == responseend)
		return DNS_SOFT;

	if ((i = dn_expand(response.buf, responseend, responsepos, name, MAXDNAME)) < 0)
		return DNS_SOFT;
	responsepos += i;

	if ((i = responseend - responsepos) < 4 + 3 * 3)
		return DNS_SOFT;

	rrtype = getshort(responsepos);
	rrdlen = getshort(responsepos + 8);
	responsepos += 10;

	*txt_strlen = 0;
	if (!txtlen) {
		if (!(txt = (char *) malloc(PACKETSZ * 2 * sizeof(char))))
			return DNS_MEM;
		txtlen = PACKETSZ * 2;
	}
	if (rrtype == wanttype) {
		unsigned short  txtpos;
		unsigned char   n;

		*txt = 0;
		for (txtpos = 0; txtpos < rrdlen; txtpos += n) {
			n = responsepos[txtpos++];
			if (n > rrdlen - txtpos)
				n = rrdlen - txtpos;
			if ((*txt_strlen + n + 1) > txtlen) {
				if (!(ptr = (char *) realloc(txt, (*txt_strlen + n) * 2))) {
					free(txt);
					txtlen = 0;
					if (dnresultlen) {
						free(dnresult);
						dnresultlen = 0;
					}
					return DNS_MEM;
				}
				txt = ptr;
				txtlen = (*txt_strlen + n) * 2;
			}
			byte_copy(txt + *txt_strlen, n, (char *) &responsepos[txtpos]);
			*txt_strlen += n;
		}
		responsepos += rrdlen;
		txt[*txt_strlen] = 0;
		return 1;
	}
	responsepos += rrdlen;
	return 0;
}

static int
dns_txtplus(char *domain)
{
	int             r, len, total;
	char           *ptr;

	switch (resolve(domain, T_TXT))
	{
	case DNS_MEM:
		return DNS_MEM;
	case DNS_SOFT:
		return DNS_SOFT;
	case DNS_HARD:
		return DNS_HARD;
	}
	total = 0;
	if (!dnresultlen) {
		if (!(dnresult = (char *) malloc ((2 * PACKETSZ) * sizeof(char))))
			return DNS_MEM;
		dnresultlen = 2 * PACKETSZ;
	}
	while ((r = findtxt(T_TXT, &len)) != 2) {
		if (r == DNS_SOFT) {
			if (txtlen) {
				free(txt);
				txtlen = 0;
			}
			return DNS_SOFT;
		}
		if (r == 1) {
			if ((total + len + 1) >= dnresultlen) {
				if (!(ptr = (char *) realloc(dnresult, (total + len) * 2))) {
					free(dnresult);
					dnresultlen = 0;
					if (txtlen) {
						free(txt);
						txtlen = 0;
					}
					return DNS_MEM;
				}
				dnresult = ptr;
				dnresultlen = (total + len) * 2;
			}
			byte_copy(dnresult + total, len, txt);
			total += len;
		}
	}
	if (txtlen) {
		free(txt);
		txtlen = 0;
	}
	if (total) {
		dnresult[total] = 0;
		return (0);
	}
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
		if (responsebuflen) {
			free(response.buf);
			responsebuflen = 0;
		}
		return strdup("e=temp;");
	case DNS_HARD:
		if (responsebuflen) {
			free(response.buf);
			responsebuflen = 0;
		}
		return strdup("e=perm;");
	}
	if (responsebuflen) {
		free(response.buf);
		responsebuflen = 0;
	}
	return dnresult;
}


int
DNSGetTXT(const char *domain, char *buffer, int maxlen)
{
	char           *results;
	int             len;

	results = dns_text((char *) domain);
	if (!strcmp(results, "e=perm;")) {
		free(results);
		dnresultlen = 0;
		return DNSRESP_PERM_FAIL;
	} else
	if (!strcmp(results, "e=temp;")) {
		free(results);
		dnresultlen = 0;
		return DNSRESP_TEMP_FAIL;
	}
	if ((len = strlen(results)) > maxlen - 1) {
		free(results);
		dnresultlen = 0;
		return DNSRESP_DOMAIN_NAME_TOO_LONG;
	}
	byte_copy(buffer, len, results);
	buffer[len] = 0;
	free(results);
	dnresultlen = 0;
	return DNSRESP_SUCCESS;
}

void
getversion_dkimdns_cpp()
{
	static char    *x = (char *) "$Id: dns.cpp,v 1.12 2020-09-30 20:46:38+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
