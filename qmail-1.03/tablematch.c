/*
 * $Log: tablematch.c,v $
 * Revision 1.6  2009-08-29 15:26:23+05:30  Cprogrammer
 * added flag to decide use of gethostbyname()
 *
 * Revision 1.5  2008-02-05 15:32:39+05:30  Cprogrammer
 * Corrected working of PARANOID
 *
 * Revision 1.4  2004-10-22 20:31:23+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-05-24 19:53:56+05:30  Cprogrammer
 * corrected logic for rejecting hosts (combination of domain and IP match)
 *
 * Revision 1.2  2004-05-23 22:27:24+05:30  Cprogrammer
 * added PARANOID mode
 * reject for IP match and no domain match and vice versa
 *
 * Revision 1.1  2004-05-19 11:48:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#include "str.h"
#include "env.h"
#include "scan.h"
#include "control.h"
#include "tablematch.h"
#include <netdb.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static char    *
my_strchr(char *str, char ch)
{
	int             i;

	i = str_chr(str, ch);
	if (!str[i])
		return ((char *) 0);
	return (str + i);
}

/*-
 * Function: matchinet
 *
 * Description:
 *     Checks to see if the internet addres matches our token
 *
 * Returns:
 *     1 if it matches, 0 if doesn't
-*/

int
matchinet(char *ip, char *token, char flag)
{
	char            field1[8];
	char            field2[8];
	int             lnum, hnum, idx1, idx2, match, tmp;
	char           *ptr, *ptr1, *ptr2, *cptr;
	struct in_addr  inp;
	struct hostent *hp;

	/*- Exact match / match all */
	if (!str_diff(token, ip))
		return (1);
	/*
	 * If our token is a valid internet address then we don't need to
	 * check any further
	 */
	if (inet_aton(token, &inp))
		return (0);
	else
	for (match = idx1 = 0, ptr1 = token, ptr2 = ip; idx1 < 4 && match == idx1; idx1++)
	{
		/*- IP Address in control file */
		for (cptr = field1; *ptr1 && *ptr1 != '.'; *cptr++ = *ptr1++);
		*cptr = 0;
		ptr1++;

		/*
		 * IP Address of client 
		 */
		for (cptr = field2; *ptr2 && *ptr2 != '.'; *cptr++ = *ptr2++);
		*cptr = 0;
		ptr2++;
		/*-
		 * Network address wildcard match (i.e. "192.86.28.*")
		 */
		if (!str_diff(field1, "*"))
		{
			match++;
			continue;
		}
		/*
		 * Network address wildcard match (i.e.
		 * "192.86.2?.12?")
		 */
		for (; (ptr = my_strchr(field1, '?'));)
		{
			lnum = ptr - field1;
			*ptr = field2[lnum];
		}
		if (!str_diff(field1, field2))
		{
			match++;
			continue;
		}
		/*
		 * Range match (i.e. "190-193.86.22.11") 
		 */
		if ((ptr = my_strchr(field1, '-')))
		{
			*ptr = 0;
			ptr++;
			scan_int(field1, &lnum);
			if (!*ptr)
				hnum = 256;
			else
				scan_int(ptr, &hnum);
			scan_int(field2, &tmp);
			for (idx2 = lnum; idx2 <= hnum; idx2++)
			{
				if (idx2 == tmp)
				{
					match++;
					break;
				}
			}
			if (idx2 <= hnum && idx2 == tmp)
				continue;
		}
	} /*- for (match = idx1 = 0, ptr1 = token, ptr2 = ip; idx1 < 4 && match == idx1; idx1++) */
	if (match == 4)
		return (1);
	if (flag)
	{
		/*
		 * If our token is a host name then translate it and compare internet
		 * addresses
		 */
		if (!(hp = gethostbyname(token)))
			return (0);
		if (!str_diff(ip, inet_ntoa(*((struct in_addr *) hp->h_addr_list[0]))))
			return (1);
	}
	return (0);
}

/*
 * *:202.133.22.1
 * yahoo.com:202.144.77.5
 * abc.com:202.10-33.54.5
 * indi.com:*
 * - All domains allwed from 202.133.22.1 (no domain restriction for
 *   202.133.22.1)
 * - yahoo.com allowed from 202.144.77.5
 * - abc.com is allowed from a ip range 202.10.54.5 to 202.33.54.5
 * - indi.com is allowed from any ip address (no IP restriction for indi.com)
 */
int
tablematch(char *filename, char *ip, char *domain)
{
	int             len, count, nullflag, dmatch, imatch, domain_match, ip_match;
	char           *ptr, *cptr;
	static stralloc hostacc = { 0 };

	if ((ptr = env_get("HOSTACCESS")))
	{
		len = str_len(ptr);
		if (*(ptr + len - 1) == '\n')
			len--;
		for (;*ptr;ptr++)
		{
			if (*ptr == '\n')
				*ptr = 0;
		}
		if (!stralloc_copyb(&hostacc, ptr, len))
			return(-2);
	} else
	{
		if ((count = control_readfile(&hostacc, filename, 0)) == -1)
			return (-1);
		if (!count) /*- allow access if control file is absent */
			return (1);
	}
	for (domain_match = ip_match = len = 0, ptr = hostacc.s; len < hostacc.len;)
	{
		len += (str_len(ptr) + 1);
		for (cptr = ptr; *cptr && *cptr != ':'; cptr++);
		if (*cptr == ':') /*- cptr + 1 is the IP address token */
			*cptr = 0;
		else
			continue;
		if (!str_diff(ptr, "<>") && !*domain)
			nullflag = 1;
		else
			nullflag = 0;
		/*
		 * If a sender domain comes from specific IPs then
		 * It also means that yahoo.com cannot come from those
		 * specific IPs.
		 */
		dmatch = imatch = 0;
		if (nullflag || (*domain && (!str_diff("*", ptr) || !str_diff(domain + 1, ptr))))
		{
			dmatch = 1;
			domain_match = str_diff(ptr, "*") ? 1 : 0; /* matched on domain & not on wildcard */
		}
		if (!str_diff(cptr + 1, "*") || !str_diff(cptr + 1, "*.*.*.*") || matchinet(ip, cptr + 1, 0))
		{
			imatch = 1;
			ip_match = (str_diff(cptr + 1, "*") && str_diff(cptr + 1, "*.*.*.*")) ? 1 : 0;
		}
		if (imatch && dmatch)
			return (1);
		ptr = hostacc.s + len;
	}
	/*
	 * Matched a domain without IP match - deny access
	 * if PARANOID is set - deny access
	 */
	if (domain_match && env_get("PARANOID"))
		return (0);
	/*
	 * IP matched but domain did not match - deny access unless
	 * DOMAIN_MASQUERADE is set
	 */
	if (ip_match && !env_get("DOMAIN_MASQUERADE"))
		return (0);
	/*- Got no match - neither domain and neither IP */
	return (1);
}

void
getversion_tablematch_c()
{
	static char    *x = "$Id: tablematch.c,v 1.6 2009-08-29 15:26:23+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
