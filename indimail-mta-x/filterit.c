/*
 * $Id: *
 */
#include <ctype.h>
#include <unistd.h>
#include <qprintf.h>
#include <subfd.h>
#include <getln.h>
#include <stralloc.h>
#include <env.h>
#include <str.h>
#include <error.h>
#include <strerr.h>
#include <noreturn.h>
#include <sgetopt.h>
#include <case.h>
#include <mess822.h>
#include <evaluate.h>
#include <replacestr.h>
#include <matchregex.h>
#include <scan.h>
#include <r_mkdir.h>
#include "maildir_deliver.h"
#include "quote.h"
#include "auto_prefix.h"
#ifdef USE_FSYNC
#include "syncdir.h"
#endif

#define FATAL "filterit: fatal: "
#define WARN  "filterit: warn: "

static int      doit = 1, exit_val_on_match = 99;
static stralloc line, addr, tmp, rpline, dtline, matched_header;

void
usage()
{
	strerr_die1x(100,
			"USAGE: filterit [ -n ] [ -r ] -h header -k keyword\n"
			"         -c comparision -a action -A action_value\n"
			"         -d default_action -D default_action_value\n"
			"         -b bounce_message\n"
			"         -e exit_value_on_match\n"
			"\nwhere\n\n"
			"  header             - Header Name\n"
			"  keyword            - keyword for match\n"
			"  comparision        - One of Equals, Contains, Starts with, Ends with,\n"
			"                       Numerical Expression, RegExp,\n"
			"  action             - One of exit, forward, maildir\n"
			"  action_val         - value if action is exit , forward, maildir\n"
			"  defaultaction      - One of forward, exit, forward, maildir\n"
			"                       Used when there is no match\n"
			"  default_action_val - value if action is exit , forward, maildir\n"
			"                       used when there no match\n"
			" bounce_message      - Text to be used for bounce message\n"
			" exit_value_on_match - Exit value to be used when a match occurs");
	_exit(100);
}

int
logical_exp(char *data, char *expression)
{
	char           *ptr;
	int             i;
	static stralloc buf = {0};
	struct val      result;
	struct vartable *vt;

	/*
	 * replace all occurences of %p in expression
	 * with the value of data
	 */
	if (!(vt = create_vartable()))
		return (0);
	buf.len = 0;
	if ((i = replacestr(expression, "%p", data, &buf)) == -1)
		strerr_die2x(111, FATAL, "out of memory");
	ptr = i ? buf.s : expression;
	switch (evaluate(ptr, &result, vt))
	{
	case ERROR_SYNTAX:
		strerr_die2x(100, FATAL, "syntax error");
	case ERROR_VARNOTFOUND:
		strerr_die2x(100, FATAL, "variable not found");
	case ERROR_NOMEM:
		strerr_die2x(111, FATAL, "out of memory");
	case ERROR_DIV0:
		strerr_die2x(100, FATAL, "division by zero");
	case RESULT_OK:
		if (result.type == T_INT)
			return (result.ival);
		else
			return (0);
	}
	free_vartable(vt);
	return (0);
}

void
do_rpline()
{
	char           *sender;
	int             i;

	if (!(sender = env_get("SENDER")))
		strerr_die2x(111, FATAL, "No SENDER environment variable");
	if (!quote2(&tmp, sender)
			|| !stralloc_copys(&rpline, "Return-Path: <")
			|| !stralloc_cat(&rpline, &tmp))
		strerr_die2x(111, FATAL, "out of memory");
	for (i = 0; i < rpline.len; ++i)
		if (rpline.s[i] == '\n')
			rpline.s[i] = '_';
	if (!stralloc_cats(&rpline, ">\n")
			|| !stralloc_copy(&tmp, &rpline)
			|| !stralloc_0(&tmp))
		strerr_die2x(111, FATAL, "out of memory");
	if (!env_put2("RPLINE", tmp.s))
		strerr_die2x(111, FATAL, "out of memory");
}

void
do_dtline()
{
	char           *local, *host;
	int             i;

	if (!(local = env_get("LOCAL")))
		strerr_die2x(111, FATAL, "No LOCAL environment variable");
	if (!(host = env_get("HOST")))
		strerr_die2x(111, FATAL, "No HOST environment variable");
	if (!stralloc_copys(&tmp, local)
			|| !stralloc_cats(&tmp, "@")
			|| !stralloc_cats(&tmp, host))
		strerr_die2x(111, FATAL, "out of memory");
	if (!stralloc_copys(&dtline, "Delivered-To: ")
			|| !stralloc_cat(&dtline, &tmp))
		strerr_die2x(111, FATAL, "out of memory");
	for (i = 0; i < dtline.len; ++i)
		if (dtline.s[i] == '\n')
			dtline.s[i] = '_';
	if (!stralloc_append(&dtline, "\n") ||
			!stralloc_copy(&tmp, &dtline) ||
			!stralloc_0(&tmp))
		strerr_die2x(111, FATAL, "out of memory");
	if (!env_put2("DTLINE", tmp.s))
		strerr_die2x(111, FATAL, "out of memory");
}

no_return int
take_action(substdio *ss, char *header, int act_type, char *act_val,
		int matched, char *bounce_message)
{
	int             i;
	uid_t           uid;
	gid_t           gid;
	char           *(args[3]) = {0, 0, 0};
	char           *home, *ptr;
	char           *MailDirNames[] = { "cur", "new", "tmp", };

	if (lseek(0, 0, SEEK_SET) == -1)
		strerr_die2sys(111, FATAL, "unable to seek: ");
	if (!doit && 
			subprintf(ss, matched ? "Matched Header=[%s], " : "Unmatched Header=[%s], ", header) == -1)
		strerr_die2sys(111, FATAL, "unable to write output: ");
	switch (act_type)
	{
	case 0: /*- exit */
		scan_int(act_val, &i);
		if (!doit) {
			if (subprintf(ss, "action=[exit %d]\n", i) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			if (substdio_flush(ss) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
		}
		if (i == 100) {
			if (substdio_puts(ss, bounce_message ? bounce_message : "message failed to evade a local filter set by recipient") == -1 ||
					substdio_put(ss, "\n", 1))
				strerr_die2sys(111, FATAL, "unable to write output: ");
			if (substdio_flush(ss) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
		}
		_exit(i);
		break;
	case 1: /*- forward */
		if (doit) {
			if (!stralloc_copys(&tmp, auto_prefix) ||
					!stralloc_catb(&tmp, "/bin/forward", 12) ||
					!stralloc_0(&tmp))
				strerr_die2x(111, FATAL, "out of memory");
			args[0] = tmp.s;
			args[1] = act_val;
			execv(*args, args); /*- run forward */
			strerr_die4sys(111, FATAL, "execv: forward ", act_val, ": ");
		}
		if (subprintf(ss, "action=forward to [%s]\n", act_val) == -1)
			strerr_die2sys(111, FATAL, "unable to write output: ");
		break;
	case 2: /*- maildir */
		if (!(home = env_get("HOME")))
			strerr_die2x(111, FATAL, "No HOME environment variable");
		if (!stralloc_copys(&addr, home) ||
				!stralloc_cats(&addr, act_val + 1) ||
				!stralloc_0(&addr))
			strerr_die2x(111, FATAL, "out of memory");
		addr.len--;
		if (doit) {
			uid = getuid();
			gid = getgid();
			for (i = 0; i < 3; i++) {
				if (!stralloc_catb(&addr, MailDirNames[i], 3) ||
						!stralloc_0(&addr))
					strerr_die2x(111, FATAL, "out of memory");
				addr.len--;
				if (access(addr.s, F_OK)) {
					if (r_mkdir(addr.s, 0700, uid, gid))
						strerr_die4sys(111, FATAL, "mkdir ", addr.s, ": ");
				}
				addr.len -= 3; /*- remove cur, new or tmp */
			}
			if (!stralloc_0(&addr))
				strerr_die2x(111, FATAL, "out of memory");
			if (!env_get("RPLINE"))
				do_rpline();
			if (!env_get("DTLINE"))
				do_dtline();
			ptr = env_get("QQEH");
			i = maildir_deliver(addr.s, &rpline, &dtline, ptr);
			switch (i)
			{
			case 0:
				break;
			case 2:
				strerr_die1x(111, "Unable to chdir to maildir. (#4.2.1)");
			case 3:
				strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
			case 4:
				strerr_die1x(111, "Unable to read message. (#4.3.0)");
			case 5:
				strerr_die1x(100, "Recipient's mailbox is full");
			default:
				strerr_die1x(111, "Temporary error on maildir delivery. (#4.3.0)");
			}
			_exit(exit_val_on_match);
		}
		if (subprintf(ss, "action=deliver to Maildir [%s]\n", addr.s) == -1)
			strerr_die2sys(111, FATAL, "unable to write output: ");
		break;
	}
	if (!doit && substdio_flush(ss) == -1)
		strerr_die2sys(111, FATAL, "unable to write output: ");
	_exit(0);
}

int
main(int argc, char **argv)
{
	char           *local, *domain, *header, *keyword,
				   *action, *action_val, *d_action,
				   *d_action_val, *comparision, *bounce_message, *ptr;
	char           *comp[] = {
						"Equals", "Contains",
						"Starts with", "Ends with",
						"Numerical Logical Expression", "RegExp", 0
					};
	char           *act[] = {"exit", "forward", "maildir", 0};
	char            ssinbuf[1024], ssoutbuf[512];
	int             opt, i, match, negate = 0, keep_continue = 0, 
					c_opt = 0, a_opt = 0, default_a_opt = 0, in_header;
	substdio        ssin, ssout;

	header = comparision = keyword = action = action_val = d_action
		= d_action_val = bounce_message = NULL;
	while ((opt = getopt(argc, argv, "nrh:c:k:a:A:d:D:b:e:")) != opteof) {
		switch (opt)
		{
		case 'n':
			doit = 0;
			break;
		case 'r':
			negate = 1;
			break;
		case 'h':
			header = optarg;
			break;
		case 'c':
			for (i = 0; comp[i]; i++) {
				if (!case_diffs(optarg, comp[i]))
					break;
			}
			if (!comp[i])
				usage();
			c_opt = i;
			comparision = comp[i];
			break;
		case 'k':
			keyword = optarg;
			break;
		case 'a':
			for (i = 0; act[i]; i++) {
				if (!case_diffs(optarg, act[i]))
					break;
			}
			if (!act[i])
				usage();
			a_opt = i;
			action = act[i];
			break;
		case 'A':
			action_val = optarg;
			break;
		case 'd':
			for (i = 0; act[i]; i++) {
				if (!case_diffs(optarg, act[i]))
					break;
			}
			if (!act[i])
				usage();
			default_a_opt = i;
			d_action = act[i];
			break;
		case 'D':
			d_action_val = optarg;
			break;
		case 'b':
			bounce_message = optarg;
			break;
		case 'e':
			scan_int(optarg, &exit_val_on_match);
			break;
		default:
			usage();
		}
	} /*- while ((opt = getopt(argc, argv, "h:b:c:k:a:")) != opteof) */
#ifdef USE_FSYNC
	if (doit) {
		ptr = env_get("USE_FSYNC");
		use_fsync = (ptr && *ptr) ? 1 : 0;
		ptr = env_get("USE_FDATASYNC");
		use_fdatasync = (ptr && *ptr) ? 1 : 0;
		ptr = env_get("USE_SYNCDIR");
		use_syncdir = (ptr && *ptr) ? 1 : 0;
	}
#endif
	if (!header || !comparision || !action || (c_opt != 6 && !keyword)
			|| !action_val || !d_action_val)
		usage();
	if ((a_opt == 2 && str_diffn(action_val, "./Maildir/", 10)) ||
			(default_a_opt == 2 && str_diffn(d_action_val, "./Maildir/", 10))) {
		strerr_warn2(WARN, "Maildir path must start with ./Maildir/", 0);
		usage();
	}
	if (a_opt == 2) {
		i = str_len(action_val);
		if (action_val[i - 1] != '/') {
			strerr_warn2(WARN, "Maildir path must end with /", 0);
			usage();
		}
	}
	if (default_a_opt == 2) {
		i = str_len(d_action_val);
		if (d_action_val[i - 1] != '/') {
			strerr_warn2(WARN, "Maildir path must end with /", 0);
			usage();
		}
	}
	if (!(local = env_get("LOCAL")))
		strerr_die2x(111, FATAL, "No LOCAL environment variable");
	if (!(domain = env_get("HOST")))
		strerr_die2x(111, FATAL, "No HOST environment variable");
	if (!stralloc_copys(&addr, local) || !stralloc_append(&addr, "@"))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_cats(&addr, domain) || !stralloc_0(&addr))
		strerr_die2x(111, FATAL, "out of memory");
	addr.len--;

	substdio_fdbuf(&ssin, read, 0, ssinbuf, sizeof(ssinbuf));
	substdio_fdbuf(&ssout, write, 1, ssoutbuf, sizeof(ssoutbuf));
	if (!stralloc_cats(&tmp, header) ||
			!stralloc_append(&tmp, ":"))
		strerr_die2x(111, FATAL, "out of memory");
	for (in_header = 1;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!line.len)
			break;
		if (!mess822_ok(&line)) /*- that ends the body */
			in_header = 0;
		if (!doit && substdio_put(&ssout, line.s, line.len) == -1)
			strerr_die2sys(111, FATAL, "unable to write output: ");
		line.len--;
		if (!in_header)
			continue;
		if (line.s[0] == ' ' || line.s[0] == '\t') /*- RFC 822 LWSP char */ {
			if (keep_continue) {
				if (!stralloc_cat(&matched_header, &line))
					strerr_die2x(111, FATAL, "out of memory");
			}
		} else {
			keep_continue = 0;
			if (!case_diffb(tmp.s, tmp.len, line.s)) {
				if (!stralloc_cat(&matched_header, &line))
					strerr_die2x(111, FATAL, "out of memory");
				keep_continue = 1;
			}
		}
	}
	if (!matched_header.len) { /*- no headers matched -h value */
		take_action(&ssout, header, default_a_opt, d_action_val, 0, bounce_message);
		return 0;
	}
	if (!stralloc_0(&matched_header))
		strerr_die2x(111, FATAL, "out of memory");
	matched_header.len--;
	switch (c_opt)
	{
	case 0: /*- Equals */
		if (!stralloc_append(&tmp, " ") ||
				!stralloc_cats(&tmp, keyword) ||
				!stralloc_0(&tmp))
			strerr_die2x(111, FATAL, "out of memory");
		tmp.len--;
		match = !case_diffs(matched_header.s, tmp.s);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, matched_header.s, a_opt, action_val, 1, bounce_message);
		break;
	case 1: /*- Contains */
		i = str_len(keyword);
		case_lowers(matched_header.s);
		case_lowers(keyword);
		match = str_str(matched_header.s + i, keyword) ? 1 : 0;
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, matched_header.s, a_opt, action_val, 1, bounce_message);
		break;
	case 2: /*- Starts with */
		if (!stralloc_append(&tmp, " ") ||
				!stralloc_cats(&tmp, keyword) ||
				!stralloc_0(&tmp))
			strerr_die2x(111, FATAL, "out of memory");
		tmp.len--;
		match = !case_diffb(matched_header.s, tmp.len, tmp.s);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, matched_header.s, a_opt, action_val, 1, bounce_message);
		break;
	case 3: /*- Ends with */
		i = str_len(keyword);
		match = !case_diffb(matched_header.s + i, i, keyword);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, matched_header.s, a_opt, action_val, 1, bounce_message);
		break;
	case 4: /*- Numerical Logical Expression */
		for (ptr = matched_header.s + tmp.len; isspace(*ptr); ptr++, tmp.len++);
		match = logical_exp(matched_header.s + tmp.len, keyword);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, matched_header.s, a_opt, action_val, 1, bounce_message);
		break;
	case 5: /*- RegExp */
		for (ptr = matched_header.s + tmp.len; isspace(*ptr); ptr++, tmp.len++);
		match = matchregex(matched_header.s + tmp.len, keyword, 0);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, matched_header.s, a_opt, action_val, 1, bounce_message);
		break;
	} /* switch (c_opt) */
	take_action(&ssout, matched_header.s, default_a_opt, d_action_val, 0, bounce_message);
}

void
getversion_filterit_c()
{
	char *x = sccsidevalh;
	x++;
}

/*
 * $Log: $
 */
