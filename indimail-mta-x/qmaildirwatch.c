/*
 * $Id: qmaildirwatch.c,v 1.11 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <getln.h>
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <str.h>
#include <open.h>
#include <strerr.h>
#include <noreturn.h>
#include "prioq.h"
#include "hfield.h"
#include "headerbody.h"
#include "maildir.h"

#define FATAL "qmaildirwatch: fatal: "

static stralloc recipient = { 0 };
static stralloc sender = { 0 };
static stralloc fromline = { 0 };
static stralloc text = { 0 };

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
addtext(char *s, int n)
{
	if (!stralloc_catb(&text, s, n))
		die_nomem();
	if (text.len > 158)
		text.len = 158;
}

void
dobody(stralloc *h)
{
	addtext(h->s, h->len);
}

void
doheader(stralloc *h)
{
	int             i;
	switch (hfield_known(h->s, h->len))
	{
	case H_SUBJECT:
		i = hfield_skipname(h->s, h->len);
		addtext(h->s + i, h->len - i);
		break;
	case H_DELIVEREDTO:
		i = hfield_skipname(h->s, h->len);
		if (i < h->len)
			if (!stralloc_copyb(&recipient, h->s + i, h->len - i - 1))
				die_nomem();
		break;
	case H_RETURNPATH:
		i = hfield_skipname(h->s, h->len);
		if (i < h->len)
			if (!stralloc_copyb(&sender, h->s + i, h->len - i - 1))
				die_nomem();
		break;
	case H_FROM:
		if (!stralloc_copyb(&fromline, h->s, h->len - 1))
			die_nomem();
		break;
	}
}
void
finishheader()
{;
}

stralloc        filenames = { 0 };
prioq           pq = { 0 };

char            inbuf[SUBSTDIO_INSIZE];
substdio        ssin;

int
main()
{
	struct prioq_elt pe;
	int             fd;
	int             i;

	if (maildir_chdir() == -1)
		strerr_die1(111, FATAL, &maildir_chdir_err);
	for (;;) {
		maildir_clean(&filenames);
		if (maildir_scan(&pq, &filenames, 1, 0) == -1)
			strerr_die1(111, FATAL, &maildir_scan_err);
		substdio_putsflush(subfdout, "\033[;H\033[;J");
		while (prioq_get(&pq, &pe)) {
			prioq_del(&pq);
			fd = open_read(filenames.s + pe.id);
			if (fd == -1)
				continue;
			substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
			if (!stralloc_copys(&sender, "?"))
				die_nomem();
			if (!stralloc_copys(&recipient, "?"))
				die_nomem();
			if (!stralloc_copys(&fromline, ""))
				die_nomem();
			if (!stralloc_copys(&text, ""))
				die_nomem();
			if (headerbody(&ssin, doheader, finishheader, dobody) == -1)
				strerr_die2x(111, FATAL, "trouble reading new message");
			for (i = 0; i < fromline.len; ++i)
				if ((fromline.s[i] < 32) || (fromline.s[i] > 126))
					fromline.s[i] = '/';
			for (i = 0; i < sender.len; ++i)
				if ((sender.s[i] < 32) || (sender.s[i] > 126))
					sender.s[i] = '?';
			for (i = 0; i < recipient.len; ++i)
				if ((recipient.s[i] < 32) || (recipient.s[i] > 126))
					recipient.s[i] = '?';
			for (i = 0; i < text.len; ++i)
				if ((text.s[i] < 32) || (text.s[i] > 126))
					text.s[i] = '/';
			substdio_puts(subfdout, "FROM ");
			substdio_put(subfdout, sender.s, sender.len);
			substdio_puts(subfdout, " TO <");
			substdio_put(subfdout, recipient.s, recipient.len);
			substdio_puts(subfdout, ">\n");
			if (fromline.len) {
				substdio_puts(subfdout, "\033[1m");
				substdio_put(subfdout, fromline.s, fromline.len);
				substdio_puts(subfdout, "\033[0m\n");
			}
			substdio_put(subfdout, text.s, text.len);
			substdio_puts(subfdout, "\n\n");
			close(fd);
		}
		substdio_flush(subfdout);
		sleep(30);
	}
	/*- Not reached */
	return(0);
}

void
getversion_qmaildirwatch_c()
{
	const char     *x = "$Id: qmaildirwatch.c,v 1.11 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: qmaildirwatch.c,v $
 * Revision 1.11  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.10  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.8  2021-06-03 18:15:46+05:30  Cprogrammer
 * use new prioq functions
 *
 * Revision 1.7  2020-11-24 13:45:47+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2004-10-22 20:26:30+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:35:46+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-07-17 21:19:46+05:30  Cprogrammer
 * added RCS log
 *
 */
