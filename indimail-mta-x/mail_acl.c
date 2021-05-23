/*
 * $Log: mail_acl.c,v $
 * Revision 1.4  2021-05-23 07:10:02+05:30  Cprogrammer
 * include wildmat.h for wildmat_internal
 *
 * Revision 1.3  2014-03-07 02:09:22+05:30  Cprogrammer
 * fix regex match
 *
 * Revision 1.2  2011-11-17 20:03:58+05:30  Cprogrammer
 * fixed diag message getting printed without verbose flag
 *
 * Revision 1.1  2010-11-05 01:06:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <str.h>
#include <stralloc.h>
#include <fmt.h>
#include "matchregex.h"
#include "wildmat.h"

extern void     die_regex(char *);
extern void     out(char *);
extern void     flush();

int
mail_acl(stralloc *acclist, int qregex, char *sender, char *recipient, char verb)
{
	int             err, len, count, from_reject, rcpt_reject, rcpt_found, from_found;
	char           *ptr, *cptr, *rcpt_match, *from_match, *err_str;
	char            count_buf[FMT_ULONG];

	/*- Cannot reject bounces */
	if (!*sender || !str_diff(sender, "#@[]"))
	{
		if (verb)
		{
			out("access allowed for bounces\n");
			flush();
		}
		return(0);
	}
	if (!str_diffn(recipient, "postmaster@", 11) || !str_diff(recipient, "postmaster"))
	{
		if (verb)
			out("access allowed for postmaster\n");
		return(0);
	}
	if (!str_diffn(recipient, "abuse@", 6) || !str_diff(recipient, "abuse"))
	{
		if (verb)
			out("access allowed for this email\n");
		return(0);
	}
	/*
	 * Format:
	 * rcpt:sender:recipient
	 * from:sender:recipient
	 * e.g.
	 * rcpt:ceo@indimail.org:country_distribution_list@indimail.org
	 * rcpt:md@indimail.org:country_distribution_list@indimail.org
	 * from:recruiter@yahoo.com:hr@indimail.org 
	 */
	for (rcpt_reject = len = 0, count = 1, ptr = acclist->s;len < acclist->len;count++)
	{
		len += (str_len(ptr) + 1); /*- length of each rule */
		/*
		 * recipient is cptr + 1
		 */
		for (cptr = ptr + 5;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':')
			*cptr = 0;
		else
			continue;
		rcpt_found = 0;
		from_match = rcpt_match = 0;
		/*- find if a match for recipient occurs in the rule */
		if (!str_diff(recipient, cptr + 1)) /*- recipient */
		{
			rcpt_found = 1;
			rcpt_match = cptr + 1;
		} else
		{
			if (qregex)
			{
				if ((err = matchregex(recipient, cptr + 1, &err_str)) < 0)
				{
					*cptr = 0;
					return (err);
				} else
				if (err == 1)
				{
					rcpt_found = 1;
					rcpt_match = cptr + 1;
				}
			} else
			if (wildmat_internal(recipient, cptr + 1))
			{
				rcpt_found = 1;
				rcpt_match = cptr + 1;
			}
		}
		if (!rcpt_found)
		{
			*cptr = ':';
			ptr = acclist->s + len;
			continue;
		}
		if (!str_diffn(ptr, "rcpt:", 5))
		{
			rcpt_reject = 1;
			if (verb)
			{
				out("rule no ");
				count_buf[fmt_ulong(count_buf, (unsigned long) count)] = 0;
				out(count_buf);
				out(": ");
				*cptr = ':';
				out(ptr);
				*cptr = 0;
				out("\n");
				out("\tmatched recipient     [");
				out(recipient);
				out("] with [");
				out(rcpt_match);
				out("]\n");
			}
		}
		if (!rcpt_reject)
		{
			*cptr = ':';
			ptr = acclist->s + len;
			continue;
		}
		if (!str_diff(sender, ptr + 5)) /*- sender */
			from_match = ptr + 5;
		else
		{
			if (qregex)
			{
				if ((err = matchregex(sender, ptr + 5, &err_str)) < 0)
				{
					*cptr = ':';
					if (verb)
						die_regex(err_str);
					return (err);
				} else
					from_match = ptr + 5;
			} else
			if (wildmat_internal(sender, ptr + 5))
				from_match = ptr + 5;
		}
		if (rcpt_reject)
		{
			if (from_match) /*- explicit allow rule found for sender */
			{
				if (verb)
				{
					out("\tmatched sender        [");
					out(sender);
					out("] with [");
					out(from_match);
					out("] --> access allowed\n");
				}
				return (0);
			}
			if (verb)
			{
				out("\tsender not matched    [");
				out(sender);
				out("] --> access denied\n");
			}
		}
		*cptr = ':';
		ptr = acclist->s + len; /*- go to the next rule */
	}
	for (from_reject = len = 0, count = 1, ptr = acclist->s;len < acclist->len;count++)
	{
		len += (str_len(ptr) + 1); /*- length of each rule */
		/*
		 * sender is ptr + 5
		 */
		for (cptr = ptr + 5;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':')
			*cptr = 0;
		else
			continue;
		from_found = 0;
		from_match = rcpt_match = 0;
		/*- find if a match for sender occurs in the rule */
		if (!str_diff(sender, ptr + 5)) /*- sender */
		{
			from_found = 1;
			from_match = ptr + 5;
		} else
		{
			if (qregex)
			{
				if ((err = matchregex(sender, ptr + 5, &err_str)) < 0)
				{
					*cptr = ':';
					if (verb)
						die_regex(err_str);
					return (err);
				} else
				{
					from_found = 1;
					from_match = ptr + 5;
				}
			} else
			if (wildmat_internal(sender, ptr + 5))
			{
				from_found = 1;
				from_match = ptr + 5;
			}
		}
		if (!from_found) /*- this rule is irrelevant for the sender */
		{
			*cptr = ':';
			ptr = acclist->s + len;
			continue;
		}
		if (!str_diffn(ptr, "from:", 5))
		{
			from_reject = 1;
			if (verb)
			{
				out("rule no ");
				count_buf[fmt_ulong(count_buf, (unsigned long) count)] = 0;
				out(count_buf);
				out(": ");
				*cptr = ':';
				out(ptr);
				*cptr = 0;
				out("\n");
				out("\tmatched sender        [");
				out(sender);
				out("] with [");
				out(from_match);
				out("]\n");
			}
		}
		if (!from_reject)
		{
			*cptr = ':';
			ptr = acclist->s + len;
			continue;
		}
		if (!str_diff(recipient, cptr + 1)) /*- recipient */
			rcpt_match = cptr + 1;
		else
		{
			if (qregex)
			{
				if ((err = matchregex(recipient, cptr + 1, &err_str)) < 0)
				{
					*cptr = ':';
					if (verb)
						die_regex(err_str);
					return (err);
				} else
					rcpt_match = cptr + 1;
			} else
			if (wildmat_internal(recipient, cptr + 1))
				rcpt_match = cptr + 1;
		}
		if (from_reject)
		{
			if (rcpt_match) /*- explicit allow rule found for recipient */
			{
				if (verb)
				{
					out("\tmatched recipient     [");
					out(recipient);
					out("] with [");
					out(rcpt_match);
					out("] --> access allowed\n");
				}
				return (0);
			}
			if (verb)
			{
				out("\trecipient not matched [ ");
				out(recipient);
				out("] --> access denied\n");
			}
		}
		*cptr = ':';
		ptr = acclist->s + len; /*- go to the next rule */
	}
	return (rcpt_reject || from_reject);
}

void
getversion_mail_acl_c()
{
	static char    *x = "$Id: mail_acl.c,v 1.4 2021-05-23 07:10:02+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidwildmath;
	x++;
}
