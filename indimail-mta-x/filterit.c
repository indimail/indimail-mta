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

#define FATAL "filterit: fatal: "
#define WARN  "filterit: warn: "

static int      doit = 1;

void
usage()
{
	strerr_die1x(100,
			"USAGE: filterit [ -n ] [ -r ] -h header -k keyword\n"
			"         -c comparision -a action -v action_value\n"
			"where\n"
			"  header       - Header Name\n"
			"  keyword      - keyword for match\n"
			"  comparision  - One of Equals, Contains, Starts with, Ends with,\n"
			"                 Numerical Expression, RegExp,\n"
			"                 Recipient not in To, CC, Bcc\n"
			"  action       - One of forward, exit, maildir, mailbox, bounce,\n"
			"                 drop\n"
			"  action_value - value if action is exit , maildir or mailbox");
	_exit(100);
}

int
numerical_compare(char *data, char *expression)
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

int
take_action(substdio *ss, char *result, int len, int act_type, char *act_val)
{
	if (lseek(0, 0, SEEK_SET) == -1)
		strerr_die2sys(111, FATAL, "unable to seek");
	if (!doit) {
		return 0;
	}
	switch (act_type)
	{
	case 0: /*- forword */
		break;
	case 1: /*- exit */
		break;
	case 2: /*- maildir */
		break;
	case 3: /*- mailbox */
		break;
	case 4: /*- bounce */
		break;
	case 5: /*- drop */
		break;
	}
	return 0;
}

int
main(int argc, char **argv)
{
	char           *local, *domain, *header, *keyword,
				   *action, *action_val, *comparision, *ptr;
	char           *comp[] = { "Equals", "Contains",
					"Starts with", "Ends with",
					"Numerical Logical Expression", "RegExp",
					"address not in To, CC, Bcc", 0 };
	char           *act[] = {"forward", "exit", "maildir", "mailbox", "bounce", "drop", 0};
	char            ssinbuf[1024], ssoutbuf[512];
	int             opt, i, match, negate = 0, keep_continue = 0, 
					c_opt = 0, a_opt = 0;
	substdio        ssin, ssout;
	stralloc        line = { 0 }, addr = { 0 }, tmp = { 0 }, result = { 0 };

	header = comparision = keyword = action = action_val = NULL;
	while ((opt = getopt(argc, argv, "nrh:c:k:a:")) != opteof) {
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
		case 'v':
			action_val = optarg;
			break;
		default:
			usage();
		}
	} /*- while ((opt = getopt(argc, argv, "h:b:c:k:a:")) != opteof) */
	if (!header || !comparision || !action || (c_opt == 0 && !keyword)
			|| (a_opt < 4 && !action_val))
		usage();
	if (!(local = env_get("LOCAL")))
		strerr_die1x(111, "No LOCAL environment variable");
	if (!(domain = env_get("HOST")))
		strerr_die1x(111, "No HOST environment variable");
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
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line)) /*- that ends the body */
				break;
		line.len--;
		if (line.s[0] == ' ' || line.s[0] == '\t') /*- RFC 822 LWSP char */ {
			if (keep_continue) {
				if (!stralloc_cat(&result, &line))
					strerr_die2x(111, FATAL, "out of memory");
			}
		} else {
			keep_continue = 0;
			if (!case_diffb(tmp.s, tmp.len, line.s)) {
				if (!stralloc_cat(&result, &line))
					strerr_die2x(111, FATAL, "out of memory");
				keep_continue = 1;
			}
		}
	}
	if (!result.len) /*- no headers matched -h value */
		return 0;
	if (!stralloc_0(&result))
		strerr_die2x(111, FATAL, "out of memory");
	result.len--;
	switch (c_opt)
	{
	case 0: /*- Equals */
		if (!stralloc_append(&tmp, " ") ||
				!stralloc_cats(&tmp, keyword) ||
				!stralloc_0(&tmp))
			strerr_die2x(111, FATAL, "out of memory");
		tmp.len--;
		match = !case_diffs(result.s, tmp.s);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, result.s, result.len, a_opt, action_val);
		break;
	case 1: /*- Contains */
		i = str_len(keyword);
		case_lowers(result.s);
		case_lowers(keyword);
		match = str_str(result.s + i, keyword) ? 1 : 0;
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, result.s, result.len, a_opt, action_val);
		break;
	case 2: /*- Starts with */
		if (!stralloc_append(&tmp, " ") ||
				!stralloc_cats(&tmp, keyword) ||
				!stralloc_0(&tmp))
			strerr_die2x(111, FATAL, "out of memory");
		tmp.len--;
		match = !case_diffb(result.s, tmp.len, tmp.s);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, result.s, result.len, a_opt, action_val);
		break;
	case 3: /*- Ends with */
		i = str_len(keyword);
		match = !case_diffb(result.s + i, i, keyword);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, result.s, result.len, a_opt, action_val);
		break;
	case 4: /*- Numerical Logical Expression */
		for (ptr = result.s + tmp.len; isspace(*ptr); ptr++, tmp.len++);
		match = numerical_compare(result.s + tmp.len, keyword);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, result.s, result.len, a_opt, action_val);
		break;
	case 5: /*- RegExp */
		for (ptr = result.s + tmp.len; isspace(*ptr); ptr++, tmp.len++);
		match = matchregex(result.s + tmp.len, keyword, 0);
		if (negate)
			match = !match;
		if (match)
			take_action(&ssout, result.s, result.len, a_opt, action_val);
		break;
	case 6: /*- Recipient not in To, Cc, Bcc */
		break;
	}
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
