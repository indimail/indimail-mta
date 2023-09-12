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
#include <mess822.h>

#define FATAL "filterit: fatal: "
#define WARN  "filterit: warn: "

void
usage()
{
	strerr_die1x(100,
			"USAGE: filterit [ -n ] [ -r ]\n"
			"         -h header || -b body\n"
			"         -k keyword\n"
			"         -c comparision\n"
			"         -a action\n"
			"  where\n"
			"          header      - Header Name\n"
			"          body        - string in body\n"
			"          keyword     - keyword for match\n"
			"          comparision - One of Equals, Contains, Starts with, Ends with,\n"
			"                        Numerical Expression, RegExp, Recipient on in To, CC, Bcc\n");
	_exit(100);
}

int
main(int argc, char **argv)
{
	char           *local, *domain, *header, *body, *keyword,
				   *action, *comparision;
	char           *comp[] = { "Equals", "Contains",
					"Starts with", "Ends with",
					"Numerical Logical Expression", "RegExp",
					"My id not in To, CC, Bcc", 0 };
	char           *act[] = {"forward", "exit", "maildir", "mailbox", "bounce", 0};
	char            ssinbuf[1024], ssoutbuf[512];
	int             opt, i, doit = 1, match, negate = 0, keep_continue = 0;
	substdio        ssin, ssout;
	stralloc        line = { 0 }, addr = { 0 }, incl = { 0 };

	header = body = comparision = keyword = action = NULL;
	while ((opt = getopt(argc, argv, "nrh:b:c:k:a:")) != opteof) {
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
		case 'b':
			body = optarg;
			break;
		case 'c':
			for (i = 0; comp[i]; i++) {
				if (!str_diff(optarg, comp[i]))
					break;
			}
			if (!comp[i])
				usage();
			comparision = comp[i];
			break;
		case 'k':
			keyword = optarg;
			break;
		case 'a':
			for (i = 0; act[i]; i++) {
				if (!str_diff(optarg, act[i]))
					break;
			}
			if (!act[i])
				usage();
			action = act[i];
			break;
		default:
			usage();
		}
	} /*- while ((opt = getopt(argc, argv, "h:b:c:k:a:")) != opteof) */
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
	substdio_fdbuf(&ssout, write, 0, ssoutbuf, sizeof(ssoutbuf));
	if (header) {
		if (!stralloc_cats(&incl, optarg) ||
				!stralloc_append(&incl, ":") ||
				!stralloc_0(&incl))
			strerr_die2x(111, FATAL, "out of memory");
	}
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
	}
}
