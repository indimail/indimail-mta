/*
 * $Log: dns.cpp,v $
 * Revision 1.4  2009-06-11 13:58:39+05:30  Cprogrammer
 * port for DARWIN
 *
 * Revision 1.3  2009-03-27 19:22:45+05:30  Cprogrammer
 * dns functions
 *
 */
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#ifdef DARWIN
#include <nameser8_compat.h>
#endif
#include <arpa/nameser.h>
#include <resolv.h>
#include <string.h>
#include "dns.h"

static unsigned short
getshort(unsigned char *cp)
{
	return (cp[0] << 8) | cp[1];
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
	u_char          response[PACKETSZ + 1];	/* response */
	int             responselen;			/* buffer length */
	int             rc;						/* misc variables */
	int             ancount, qdcount;		/* answer count and query count */
	u_short         type, rdlength;			/* fields of records returned */
	u_char         *eom, *cp;
	u_char          buf[PACKETSZ + 1];		/* we're storing a TXT record here, not just a DNAME */
	u_char         *bufptr;

	responselen = res_query(dn, C_IN, T_TXT, response, sizeof (response));
	if (responselen < 0) {
		if (h_errno == TRY_AGAIN)
			return strdup("e=temp;");
		else
			return strdup("e=perm;");
	}
	qdcount = getshort(response + 4);	/* http://crynwr.com/rfc1035/rfc1035.html#4.1.1. */
	ancount = getshort(response + 6);
	eom = response + responselen;
	cp = response + HFIXEDSZ;
	while (qdcount-- > 0 && cp < eom)
	{
		rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME);
		if (rc < 0)
			return strdup("e=perm;");
		cp += rc + QFIXEDSZ;
	}
	while (ancount-- > 0 && cp < eom)
	{
		if ((rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME)) < 0)
			return strdup("e=perm;");
		cp += rc;
		if (cp + RRFIXEDSZ >= eom)
			return strdup("e=perm;");
		type = getshort(cp + 0);	/* http://crynwr.com/rfc1035/rfc1035.html#4.1.3. */
		rdlength = getshort(cp + 8);
		cp += RRFIXEDSZ;
		if (type != T_TXT)
		{
			cp += rdlength;
			continue;
		}
		bufptr = buf;
		while (rdlength && cp < eom)
		{
			unsigned int    cnt;

			cnt = *cp++;		/* http://crynwr.com/rfc1035/rfc1035.html#3.3.14. */
			if (bufptr - buf + cnt + 1 >= PACKETSZ)
				return strdup("e=perm;");
			if (cp + cnt > eom)
				return strdup("e=perm;");
			memcpy((char *) bufptr, (char *) cp, cnt);
			rdlength -= cnt + 1;
			bufptr += cnt;
			cp += cnt;
			*bufptr = '\0';
		}
		return (char *) strdup((char *) buf);
	}
	return strdup("e=perm;");
}

int
DNSGetTXT(const char *domain, char *buffer, int maxlen)
{
	char           *results;
	int             len;

	results = dns_text((char *) domain);
	if (!strcmp(results, "e=perm;"))
	{
		free(results);
		return DNSRESP_PERM_FAIL;
	} else
	if (!strcmp(results, "e=temp;"))
	{
		free(results);
		return DNSRESP_TEMP_FAIL;
	}
	if (strlen(results) > maxlen - 1)
	{
		free(results);
		return DNSRESP_DOMAIN_NAME_TOO_LONG;
	}
	strncpy(buffer, results, maxlen);
	free(results);
	return DNSRESP_SUCCESS;
}

void
getversion_dkimdns_cpp()
{
	static char    *x = (char *) "$Id: dns.cpp,v 1.4 2009-06-11 13:58:39+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
