/*
 * $Log: dns_text.c,v $
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
#ifdef DARWIN
#include <nameser8_compat.h>
#endif
#include <arpa/nameser.h>
#include <resolv.h>
#include "byte.h"
#ifdef DOMAIN_KEYS
#include "domainkeys.h"
#else
#include "str.h"
#include <openssl/evp.h>

#define DKIM_MALLOC(s)     OPENSSL_malloc(s)

char           *
dk_strdup(const char *s)
{
	char           *new = DKIM_MALLOC(str_len((char *) s) + 1);
	if (new != 0)
		str_copy(new, (char *) s);
	return new;
}
#endif

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
			return dk_strdup("e=temp;");
		else
			return dk_strdup("e=perm;");
	}
	qdcount = getshort(response + 4);	/* http://crynwr.com/rfc1035/rfc1035.html#4.1.1. */
	ancount = getshort(response + 6);
	eom = response + responselen;
	cp = response + HFIXEDSZ;
	while (qdcount-- > 0 && cp < eom)
	{
		rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME);
		if (rc < 0)
			return dk_strdup("e=perm;");
		cp += rc + QFIXEDSZ;
	}
	while (ancount-- > 0 && cp < eom)
	{
		if ((rc = dn_expand(response, eom, cp, (char *) buf, MAXDNAME)) < 0)
			return dk_strdup("e=perm;");
		cp += rc;
		if (cp + RRFIXEDSZ >= eom)
			return dk_strdup("e=perm;");
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
				return dk_strdup("e=perm;");
			if (cp + cnt > eom)
				return dk_strdup("e=perm;");
			byte_copy((char *) bufptr, cnt, (char *) cp);
			rdlength -= cnt + 1;
			bufptr += cnt;
			cp += cnt;
			*bufptr = '\0';
		}
		return (char *) dk_strdup((char *) buf);
	}
	return dk_strdup("e=perm;");
}

void
getversion_dns_text_c()
{
	static char    *x = "$Id: dns_text.c,v 1.5 2009-06-11 15:20:57+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
// vim: shiftwidth=2:tabstop=4:softtabstop=4
