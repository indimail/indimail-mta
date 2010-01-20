/*
 * $Log: uacl.c,v $
 * Revision 1.3  2010-01-20 11:26:32+05:30  Cprogrammer
 * new logic for access list
 *
 * Revision 1.2  2010-01-19 13:27:58+05:30  Cprogrammer
 * display error for chdir instead of 'unable to read controls'
 *
 * Revision 1.1  2010-01-19 13:24:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "auto_qmail.h"
#include "qregex.h"
#include "scan.h"
#include "control.h"
#include "str.h"
#include "matchregex.h"
#include "fmt.h"
#include "env.h"
#include "subfd.h"
#include "strerr.h"

#define FATAL "uacl: fatal: "

/*- accesslist */
int             acclistok = 0;
static stralloc acclist = { 0 };
int             qregex = 0;
char           *errStr = 0;

int             wildmat_internal(char *, char *);

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
die_control()
{
	substdio_putsflush(subfderr, "uacl: unable to read controls (#4.3.0)\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
die_nomem()
{
	substdio_putsflush(subfderr, "uacl: out of memory\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
die_usage()
{
	substdio_putsflush(subfderr, "usage: uacl sender recipient\n");
	substdio_flush(subfdout);
	_exit(111);
}

void
die_regex(char *str)
{
	substdio_puts(subfderr, "uacl: regex failed: ");
	substdio_puts(subfderr, str);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfdout);
	substdio_flush(subfderr);
	_exit(111);
}

void
die_chdir()
{
	substdio_putsflush(subfderr, "uacl: fatal: unable to chdir to home\n");
	_exit(111);
}

int
mail_acl(char *sender, char *recipient)
{
	int             err, len, count, from_reject, rcpt_reject, rcpt_found, from_found;
	char           *ptr, *cptr, *rcpt_match, *from_match;
	char            count_buf[FMT_ULONG];

	if (!acclistok)
		return(0);
	/*- Cannot reject bounces */
	if (!*sender || !str_diff(sender, "#@[]"))
	{
		out("access alowed for bounces\n");
		flush();
		return(0);
	}
	if (!str_diffn(recipient, "postmaster@", 11) || !str_diff(recipient, "postmaster"))
	{
		out("access allowed for postmaster\n");
		return(0);
	}
	if (!str_diffn(recipient, "abuse@", 6) || !str_diff(recipient, "abuse"))
	{
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
	for (rcpt_reject = len = 0, count = 1, ptr = acclist.s;len < acclist.len;count++)
	{
		len += (str_len(ptr) + 1);
		for (cptr = ptr + 5;*cptr && *cptr != ':';cptr++);
		if (*cptr == ':')
			*cptr = 0;
		else
			continue;
		rcpt_found = 0;
		from_match = rcpt_match = 0;
		if (!str_diff(recipient, cptr + 1)) /*- recipient */
		{
			rcpt_found = 1;
			rcpt_match = cptr + 1;
		} else
		{
			if (qregex)
			{
				if ((err = matchregex(recipient, cptr + 1, &errStr)) < 0)
					die_regex(errStr);
				else
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
			ptr = acclist.s + len;
			continue;
		}
		if (!str_diffn(ptr, "rcpt:", 5))
		{
			rcpt_reject = 1;
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
		if (!rcpt_reject)
		{
			*cptr = ':';
			ptr = acclist.s + len;
			continue;
		}
		if (!str_diff(sender, ptr + 5)) /*- sender */
			from_match = ptr + 5;
		else
		{
			if (qregex)
			{
				if ((err = matchregex(sender, ptr + 5, &errStr)) < 0)
					die_regex(errStr);
				else
					from_match = ptr + 5;
			} else
			if (wildmat_internal(sender, ptr + 5))
				from_match = ptr + 5;
		}
		if (rcpt_reject)
		{
			if (from_match) /*- explicit allow rule found for sender */
			{
				out("\tmatched sender        [");
				out(sender);
				out("] with [");
				out(from_match);
				out("] --> access allowed\n");
				return (0);
			}
			out("\tsender not matched    [");
			out(sender);
			out("] --> access denied\n");
		}
		*cptr = ':';
		ptr = acclist.s + len; /*- go to the next rule */
	} /*- for (rcpt_reject = len = 0, count = 1, ptr = acclist.s;len < acclist.len;count++) */
	for (from_reject = len = 0, count = 1, ptr = acclist.s;len < acclist.len;count++)
	{
		len += (str_len(ptr) + 1);
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
				if ((err = matchregex(sender, ptr + 5, &errStr)) < 0)
					return (err);
				else
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
			ptr = acclist.s + len;
			continue;
		}
		if (!str_diffn(ptr, "from:", 5))
		{
			from_reject = 1;
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
		if (!from_reject)
		{
			*cptr = ':';
			ptr = acclist.s + len;
			continue;
		}
		if (!str_diff(recipient, cptr + 1)) /*- recipient */
			rcpt_match = cptr + 1;
		else
		{
			if (qregex)
			{
				if ((err = matchregex(recipient, cptr + 1, &errStr)) < 0)
					return (err);
				else
					rcpt_match = cptr + 1;
			} else
			if (wildmat_internal(recipient, cptr + 1))
				rcpt_match = cptr + 1;
		}
		if (from_reject)
		{
			if (rcpt_match) /*- explicit allow rule found for recipient */
			{
				out("\tmatched recipient     [");
				out(recipient);
				out("] with [");
				out(rcpt_match);
				out("] --> access allowed\n");
				return (0);
			}
			out("\trecipient not matched [ ");
			out(recipient);
			out("] --> access denied\n");
		}
		*cptr = ':';
		ptr = acclist.s + len; /*- go to the next rule */
	} /*- for (from_reject = len = 0, count = 1, ptr = acclist.s;len < acclist.len;count++) */
	return ((rcpt_reject || from_reject) ? 100 : 0);
}

int
main(int argc, char **argv)
{
	char           *x;
	int             i;

	if (argc != 3)
		die_usage();
	if (chdir(auto_qmail) == -1)
		die_chdir();
	if ((x = env_get("QREGEX")))
		scan_int(x, &qregex);
	else
	{
		if (control_readint(&qregex, "qregex") == -1)
			die_control();
		if (qregex && !env_put("QREGEX=1"))
			die_nomem();
	}
	acclistok = control_readfile(&acclist, (x = env_get("ACCESSLIST")) && *x ? x : "accesslist", 0);
	i = mail_acl(argv[1], argv[2]);
	flush();
	return (i ? 100 : 0);
}

void
getversion_uacl_c()
{
	static char    *x = "$Id: uacl.c,v 1.3 2010-01-20 11:26:32+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
