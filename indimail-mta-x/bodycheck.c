/*
 * $Log: bodycheck.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2009-01-06 20:43:56+05:30  Cprogrammer
 * added bodycheck_free()
 * corrected logic for -header and -body
 *
 * Revision 1.4  2008-08-03 18:24:03+05:30  Cprogrammer
 * added proper proto for die_nomem()
 *
 * Revision 1.3  2004-10-22 20:19:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-05-26 09:32:38+05:30  Cprogrammer
 * ability to run bodycheck selectively on header/body/both
 *
 * Revision 1.1  2004-02-05 00:07:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "substdio.h"
#include "alloc.h"
#include "stralloc.h"
#include "str.h"
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

static regex_t **qreg;
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 2, sserrbuf, sizeof(sserrbuf));
static int      allocated;
static stralloc Desc = { 0 };

void            die_nomem(void); /*- defined in qmail-smtpd.c */

int
bodycheck(stralloc *body, stralloc *line, const char **desc, int in_header)
{
	char            errbuf[512];
	int             len, tmp_len, count, retval, header_check, body_check;
	char           *pos1, *pos2, *ptr, *cptr;
	static stralloc tmpLine = { 0 };

	if (!allocated)
	{
		for (count = len = 0, ptr = body->s;len < body->len;count++)
		{
			len += (str_len(ptr) + 1);
			ptr = body->s + len;
		}
		if (!(qreg = (regex_t **) alloc(sizeof(regex_t *) * (count + 1))))
			die_nomem();
		for (count = len = 0, ptr = body->s; len < body->len; count++)
		{
			tmp_len = str_len(ptr);
			pos1 = pos2 = (char *) 0;
			header_check = body_check = 0;
			for (cptr = ptr + tmp_len - 1;cptr != ptr;cptr--)
			{
				if (*cptr == ':')
				{
					pos2 = cptr;
					*cptr = 0;
					for (cptr++;*cptr && isspace(*cptr);cptr++);
					if (!str_diffn(cptr, "-header", 7))
						header_check = 1;
					else
					if (!str_diffn(cptr, "-body", 5))
						body_check = 1;
					if (header_check || body_check)
					{
						for (cptr = pos2;*cptr != ':' && cptr != ptr;cptr--);
						if (*cptr == ':')
						{
							*cptr = 0;
							pos1 = cptr;
						}
					}
					break;
				}
			}
			if (!(qreg[count] = (regex_t *) alloc(sizeof(regex_t))))
				die_nomem();
			if ((retval = regcomp(qreg[count], ptr, REG_EXTENDED|REG_ICASE)) != 0)
			{
				if (pos1)
					*pos1 = ':';
				if (pos2)
					*pos2 = ':';
				regerror(retval, qreg[count], errbuf, sizeof(errbuf));
				for (len = 0;len <= count;len++)
				{
					regfree(qreg[len]);
					free(qreg[len]);
				}
				free(qreg);
				if (substdio_puts(&sserr, ptr) == -1)
					return (-retval);
				if (substdio_puts(&sserr, ": ") == -1)
					return (-retval);
				if (substdio_puts(&sserr, errbuf) == -1)
					return (-retval);
				if (substdio_puts(&sserr, "\n") == -1)
					return (-retval);
				if (substdio_flush(&sserr) == -1)
					return (-retval);
				return (-retval);
			}
			if (pos1)
				*pos1 = ':';
			if (pos2)
				*pos2 = ':';
			len += (tmp_len + 1);
			ptr = body->s + len; /*- advance to the next rule */
		}
		qreg[count] = (regex_t *) 0;
		allocated = 1;
	}
	if (!stralloc_copy(&tmpLine, line))
		die_nomem();
	if (!stralloc_0(&tmpLine))
		die_nomem();
	*desc = "unknown";
	for (retval = REG_NOMATCH, count = len = 0, ptr = body->s; qreg[count] && len < body->len; count++)
	{
		pos1 = pos2 = (char *) 0;
		header_check = body_check = 0;
		tmp_len = str_len(ptr);
		for (cptr = ptr + tmp_len - 1;cptr != ptr;cptr--)
		{
			if (*cptr == ':')
			{
				pos2 = cptr;
				*cptr = 0;
				for (cptr++;*cptr && isspace(*cptr);cptr++);
				if (!str_diffn(cptr, "-header", 7))
					header_check = 1;
				else
				if (!str_diffn(cptr, "-body", 5))
					body_check = 1;
				if (header_check || body_check)
				{
					for (cptr = pos2;*cptr != ':' && cptr != ptr;cptr--);
					if (*cptr == ':')
					{
						*cptr = 0;
						pos1 = cptr;
					}
				} else
				{
					pos1 = pos2;
					pos2 = 0;
				}
				break;
			}
		}
		if (pos1)
			*pos1 = ':';
		if (pos2)
			*pos2 = ':';
		len += (tmp_len + 1);
		ptr = body->s + len;
		if ((body_check && in_header) || (header_check && !in_header))
			continue;
		retval = regexec(qreg[count], tmpLine.s, (size_t) 0, (regmatch_t *) 0, (int) 0);
		if (retval == REG_NOMATCH)
			continue;
		/*- Form the comment */
		if (pos1)
		{
			if (pos2)
				*pos2 = 0;
			if (!stralloc_copys(&Desc, pos1 + 1))
				die_nomem();
			*pos1 = ':';
			if (pos2)
				*pos2 = ':';
		} else
		if (!stralloc_copys(&Desc, ptr))
			die_nomem();
		if (!stralloc_0(&Desc))
			die_nomem();
		*desc = Desc.s;
		break;
		/*- regfree(&qreg); -*/
	}
	return (retval == REG_NOMATCH ? 0 : 1);
}

void
bodycheck_free()
{
	int             i, count;

	if (!allocated)
		return;
	allocated = 0;
	for (count = 0; qreg[count]; count++);
	for (i = 0;i < count;i++)
	{
		regfree(qreg[i]);
		free(qreg[i]);
	}
	free(qreg);
	return;
}

void
getversion_bodycheck_c()
{
	const char     *x = "$Id: bodycheck.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
