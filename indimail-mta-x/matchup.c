/*
 * $Log: matchup.c,v $
 * Revision 1.11  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.10  2020-11-24 13:46:11+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.9  2020-11-22 23:11:54+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.8  2020-05-10 17:46:47+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.7  2017-01-04 12:56:35+05:30  Cprogrammer
 * ignore qmail-daemon and local/remote debug lines
 *
 * Revision 1.6  2008-02-05 15:31:03+05:30  Cprogrammer
 * added ability to process both microsecond and tai64n format timestamp
 *
 * Revision 1.5  2004-10-22 20:27:24+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 15:36:00+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.3  2004-10-11 13:55:40+05:30  Cprogrammer
 * prevent inclusion of alloc.h with prototypes
 *
 * Revision 1.2  2004-01-03 00:32:09+05:30  Cprogrammer
 * changed return type to void
 *
 * Revision 1.1  2004-01-02 23:51:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <stralloc.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include <alloc.h>
#include <strerr.h>
#include <getln.h>
#include <substdio.h>
#include <subfd.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <case.h>
#include <noreturn.h>

#define FATAL "matchup: fatal: "

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
die_read()
{
	strerr_die2sys(111, FATAL, "unable to read input: ");
}

no_return void
die_write()
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

no_return void
die_write5()
{
	strerr_die2sys(111, FATAL, "unable to write fd 5: ");
}

void
out(buf, len)
	char           *buf;
	int             len;
{
	if (substdio_put(subfdout, buf, len) == -1)
		die_write();
}

void
outs(buf)
	char           *buf;
{
	if (substdio_puts(subfdout, buf) == -1)
		die_write();
}

char            buf5[512];
substdio        ss5 = SUBSTDIO_FDBUF(write, 5, buf5, sizeof buf5);

void
out5(buf, len)
	char           *buf;
	int             len;
{
	if (substdio_put(&ss5, buf, len) == -1)
		die_write5();
}

void
outs5(buf)
	char           *buf;
{
	if (substdio_puts(&ss5, buf) == -1)
		die_write5();
}

GEN_ALLOC_typedef(ulongalloc, unsigned long, u, len, a)
GEN_ALLOC_readyplus(ulongalloc, unsigned long, u, len, a, 30, ulongalloc_readyplus)
GEN_ALLOC_ready(ulongalloc, unsigned long, u, len, a, 30, ulongalloc_ready)
char            strnum[FMT_ULONG];
stralloc        pool = { 0 };
unsigned int    poolbytes = 0;
int             nummsg = 0;
ulongalloc      msg = { 0 };
ulongalloc      bytes = { 0 };
ulongalloc      qp = { 0 };
ulongalloc      uid = { 0 };
ulongalloc      numk = { 0 };
ulongalloc      numd = { 0 };
ulongalloc      numz = { 0 };
ulongalloc      sender = { 0 };
ulongalloc      birth = { 0 };

int
msg_find(m)
	unsigned long   m;
{
	int             i;
	for (i = 0; i < nummsg; ++i)
		if (msg.u[i] == m)
			return i;
	return -1;
}

int
msg_add(m)
	unsigned long   m;
{
	int             i;
	for (i = 0; i < nummsg; ++i)
		if (msg.u[i] == m)
			return i;
	i = nummsg++;
	if (!ulongalloc_ready(&msg, nummsg))
		nomem();
	if (!ulongalloc_ready(&bytes, nummsg))
		nomem();
	if (!ulongalloc_ready(&qp, nummsg))
		nomem();
	if (!ulongalloc_ready(&uid, nummsg))
		nomem();
	if (!ulongalloc_ready(&numk, nummsg))
		nomem();
	if (!ulongalloc_ready(&numd, nummsg))
		nomem();
	if (!ulongalloc_ready(&numz, nummsg))
		nomem();
	if (!ulongalloc_ready(&sender, nummsg))
		nomem();
	if (!ulongalloc_ready(&birth, nummsg))
		nomem();
	msg.u[i] = m;
	return i;
}

void
msg_kill(i)
	int             i;
{
	poolbytes -= str_len(pool.s + sender.u[i]) + 1;
	poolbytes -= str_len(pool.s + birth.u[i]) + 1;

	--nummsg;
	msg.u[i] = msg.u[nummsg];
	bytes.u[i] = bytes.u[nummsg];
	qp.u[i] = qp.u[nummsg];
	uid.u[i] = uid.u[nummsg];
	numk.u[i] = numk.u[nummsg];
	numd.u[i] = numd.u[nummsg];
	numz.u[i] = numz.u[nummsg];
	sender.u[i] = sender.u[nummsg];
	birth.u[i] = birth.u[nummsg];
}

int             numdel = 0;
ulongalloc      del = { 0 };
ulongalloc      dmsg = { 0 };
ulongalloc      dchan = { 0 };
ulongalloc      drecip = { 0 };
ulongalloc      dstart = { 0 };

int
del_find(d)
	unsigned long   d;
{
	int             i;
	for (i = 0; i < numdel; ++i)
		if (del.u[i] == d)
			return i;
	return -1;
}

int
del_add(d)
	unsigned long   d;
{
	int             i;
	for (i = 0; i < numdel; ++i)
		if (del.u[i] == d)
			return i;
	i = numdel++;
	if (!ulongalloc_ready(&del, numdel))
		nomem();
	if (!ulongalloc_ready(&dmsg, numdel))
		nomem();
	if (!ulongalloc_ready(&dchan, numdel))
		nomem();
	if (!ulongalloc_ready(&drecip, numdel))
		nomem();
	if (!ulongalloc_ready(&dstart, numdel))
		nomem();
	del.u[i] = d;
	return i;
}

void
del_kill(i)
	int             i;
{
	poolbytes -= str_len(pool.s + dchan.u[i]) + 1;
	poolbytes -= str_len(pool.s + drecip.u[i]) + 1;
	poolbytes -= str_len(pool.s + dstart.u[i]) + 1;
	--numdel;
	del.u[i] = del.u[numdel];
	dmsg.u[i] = dmsg.u[numdel];
	dchan.u[i] = dchan.u[numdel];
	drecip.u[i] = drecip.u[numdel];
	dstart.u[i] = dstart.u[numdel];
}

stralloc        pool2 = { 0 };

void
garbage()
{
	int             i;
	char           *x;

	if (pool.len - poolbytes < poolbytes + 4096)
		return;

	if (!stralloc_copys(&pool2, ""))
		nomem();

	for (i = 0; i < nummsg; ++i)
	{
		x = pool.s + birth.u[i];
		birth.u[i] = pool2.len;
		if (!stralloc_cats(&pool2, x))
			nomem();
		if (!stralloc_0(&pool2))
			nomem();
		x = pool.s + sender.u[i];
		sender.u[i] = pool2.len;
		if (!stralloc_cats(&pool2, x))
			nomem();
		if (!stralloc_0(&pool2))
			nomem();
	}

	for (i = 0; i < numdel; ++i)
	{
		x = pool.s + dstart.u[i];
		dstart.u[i] = pool2.len;
		if (!stralloc_cats(&pool2, x))
			nomem();
		if (!stralloc_0(&pool2))
			nomem();
		x = pool.s + dchan.u[i];
		dchan.u[i] = pool2.len;
		if (!stralloc_cats(&pool2, x))
			nomem();
		if (!stralloc_0(&pool2))
			nomem();
		x = pool.s + drecip.u[i];
		drecip.u[i] = pool2.len;
		if (!stralloc_cats(&pool2, x))
			nomem();
		if (!stralloc_0(&pool2))
			nomem();
	}

	if (!stralloc_copy(&pool, &pool2))
		nomem();

	poolbytes = pool.len;
						/*- redundant, but doesn't hurt */
}

 /*
  * turn TAI date into old fashioned date 
  * dates without @ are left alone 
  */

static char     datebuf[FMT_ULONG + FMT_ULONG + 2]; /*- ssssssssss.ffffffffff\n */

char           *
tai64nunix(s)
	char           *s;
{
	int             c;
	int             len;
	unsigned long   u;
	unsigned long   seconds = 0;
	unsigned long   nanoseconds = 0;

	if (*s != '@')
		return s;
	s++;
	while ((c = *s++))
	{
		u = c - '0';
		if (u >= 10)
		{
			u = c - 'a';
			if (u >= 6)
				break;
			u += 10;
		}
		seconds <<= 4;
		seconds += nanoseconds >> 28;
		nanoseconds &= 0xfffffff;
		nanoseconds <<= 4;
		nanoseconds += u;
	}
	seconds -= 4611686018427387914ULL;
	len = fmt_ulong(datebuf, seconds);
	datebuf[len++] = '.';
	len += fmt_uint0(datebuf + len, nanoseconds, 9);
	datebuf[len] = 0;
	return datebuf;
}

stralloc        line = { 0 };
int             match;

#define FIELDS 20
int             field[FIELDS];

void
clear()
{
	while (numdel > 0)
		del_kill(0);
	garbage();
}

void
starting()
{
	unsigned long   d;
	unsigned long   m;
	int             dpos;

	scan_ulong(line.s + field[3], &d);
	scan_ulong(line.s + field[5], &m);

	dpos = del_add(d);

	dmsg.u[dpos] = m;

	dstart.u[dpos] = pool.len;
	if (!stralloc_cats(&pool, tai64nunix(line.s + field[0])))
		nomem();
	if (!stralloc_0(&pool))
		nomem();

	dchan.u[dpos] = pool.len;
	if (!stralloc_cats(&pool, line.s + field[7]))
		nomem();
	if (!stralloc_0(&pool))
		nomem();

	drecip.u[dpos] = pool.len;
	if (!stralloc_cats(&pool, line.s + field[8]))
		nomem();
	if (!stralloc_0(&pool))
		nomem();
	case_lowers(pool.s + drecip.u[dpos]);

	poolbytes += pool.len - dstart.u[dpos];
}

void
delivery()
{
	unsigned long   d;
	unsigned long   m;
	int             dpos;
	int             mpos;
	char           *result = "?";
	char           *reason = "";

	scan_ulong(line.s + field[2], &d);

	dpos = del_find(d);
	if (dpos == -1)
		return;

	m = dmsg.u[dpos];
	mpos = msg_find(m);

	if (str_start(line.s + field[3], "succ"))
	{
		if (mpos != -1)
			++numk.u[mpos];
		result = "d k ";
		reason = line.s + field[4];
	} else
	if (str_start(line.s + field[3], "fail"))
	{
		if (mpos != -1)
			++numd.u[mpos];
		result = "d d ";
		reason = line.s + field[4];
	} else
	if (str_start(line.s + field[3], "defer"))
	{
		if (mpos != -1)
			++numz.u[mpos];
		result = "d z ";
		reason = line.s + field[4];
	} else
	if (str_start(line.s + field[3], "report"))
	{
		if (mpos != -1)
			++numz.u[mpos];
		result = "d z ";
		reason = "report_mangled";
	}

	outs(result);

	if (mpos != -1)
	{
		outs(pool.s + birth.u[mpos]);
		outs(" ");
		outs(pool.s + dstart.u[dpos]);
		outs(" ");
		outs(tai64nunix(line.s + field[0]));
		outs(" ");
		out(strnum, fmt_ulong(strnum, bytes.u[mpos]));
		outs(" ");
		outs(pool.s + sender.u[mpos]);
		outs(" ");
		outs(pool.s + dchan.u[dpos]);
		outs(".");
		outs(pool.s + drecip.u[dpos]);
		outs(" ");
		out(strnum, fmt_ulong(strnum, qp.u[mpos]));
		outs(" ");
		out(strnum, fmt_ulong(strnum, uid.u[mpos]));
		outs(" ");
		outs(reason);
	} else
	{
		outs(pool.s + dstart.u[dpos]);
		outs(" ");
		outs(pool.s + dstart.u[dpos]);
		outs(" ");
		outs(tai64nunix(line.s + field[0]));
		outs(" 0 ? ");
		outs(pool.s + dchan.u[dpos]);
		outs(".");
		outs(pool.s + drecip.u[dpos]);
		outs(" ? ? ");
		outs(reason);
	}

	outs("\n");

	del_kill(dpos);
	garbage();
}

void
newmsg()
{
	unsigned long   m;
	int             mpos;

	scan_ulong(line.s + field[3], &m);
	mpos = msg_find(m);
	if (mpos == -1)
		return;
	msg_kill(mpos);
	garbage();
}

void
endmsg()
{
	unsigned long   m;
	int             mpos;

	scan_ulong(line.s + field[3], &m);
	mpos = msg_find(m);
	if (mpos == -1)
		return;

	outs("m ");
	outs(pool.s + birth.u[mpos]);
	outs(" ");
	outs(tai64nunix(line.s + field[0]));
	outs(" ");
	out(strnum, fmt_ulong(strnum, bytes.u[mpos]));
	outs(" ");
	out(strnum, fmt_ulong(strnum, numk.u[mpos]));
	outs(" ");
	out(strnum, fmt_ulong(strnum, numd.u[mpos]));
	outs(" ");
	out(strnum, fmt_ulong(strnum, numz.u[mpos]));
	outs(" ");
	outs(pool.s + sender.u[mpos]);
	outs(" ");
	out(strnum, fmt_ulong(strnum, qp.u[mpos]));
	outs(" ");
	out(strnum, fmt_ulong(strnum, uid.u[mpos]));
	outs("\n");

	msg_kill(mpos);
	garbage();
}

void
info()
{
	unsigned long   m;
	int             mpos;

	scan_ulong(line.s + field[3], &m);
	mpos = msg_add(m);

	scan_ulong(line.s + field[5], &bytes.u[mpos]);
	scan_ulong(line.s + field[9], &qp.u[mpos]);
	scan_ulong(line.s + field[11], &uid.u[mpos]);

	numk.u[mpos] = 0;
	numd.u[mpos] = 0;
	numz.u[mpos] = 0;

	birth.u[mpos] = pool.len;
	if (!stralloc_cats(&pool, tai64nunix(line.s + field[0])))
		nomem();
	if (!stralloc_0(&pool))
		nomem();

	sender.u[mpos] = pool.len;
	if (!stralloc_cats(&pool, line.s + field[7]))
		nomem();
	if (!stralloc_0(&pool))
		nomem();
	case_lowers(pool.s + sender.u[mpos]);

	poolbytes += pool.len - birth.u[mpos];
}

void
extra()
{
	unsigned long   m;
	int             mpos;

	scan_ulong(line.s + field[2], &m);
	mpos = msg_find(m);
	if (mpos == -1)
		return;

	scan_ulong(line.s + field[3], &numk.u[mpos]);
	scan_ulong(line.s + field[4], &numz.u[mpos]);
	scan_ulong(line.s + field[5], &numd.u[mpos]);
}

void
pending()
{
	int             i;

	for (i = 0; i < nummsg; ++i)
	{
		outs5(pool.s + birth.u[i]);
		outs5(" info msg ");
		out5(strnum, fmt_ulong(strnum, msg.u[i]));
		outs5(": bytes ");
		out5(strnum, fmt_ulong(strnum, bytes.u[i]));
		outs5(" from ");
		outs5(pool.s + sender.u[i]);
		outs5(" qp ");
		out5(strnum, fmt_ulong(strnum, qp.u[i]));
		outs5(" uid ");
		out5(strnum, fmt_ulong(strnum, uid.u[i]));
		outs5("\n");
		outs5(pool.s + birth.u[i]);
		outs5(" extra ");
		out5(strnum, fmt_ulong(strnum, msg.u[i]));
		outs5(" ");
		out5(strnum, fmt_ulong(strnum, numk.u[i]));
		outs5(" ");
		out5(strnum, fmt_ulong(strnum, numz.u[i]));
		outs5(" ");
		out5(strnum, fmt_ulong(strnum, numd.u[i]));
		outs5("\n");
	}

	for (i = 0; i < numdel; ++i)
	{
		outs5(pool.s + dstart.u[i]);
		outs5(" starting delivery ");
		out5(strnum, fmt_ulong(strnum, del.u[i]));
		outs5(": msg ");
		out5(strnum, fmt_ulong(strnum, dmsg.u[i]));
		outs5(" to ");
		outs5(pool.s + dchan.u[i]);
		outs5(" ");
		outs5(pool.s + drecip.u[i]);
		outs5("\n");
	}

	out5(line.s, line.len);
	if (substdio_flush(&ss5) == -1)
		die_write5();
}

stralloc        outline = { 0 };

int
main()
{
	int             i;
	int             j;
	char            ch;

	if (!stralloc_copys(&pool, ""))
		nomem();

	if (!ulongalloc_ready(&msg, 1))
		nomem();
	if (!ulongalloc_ready(&bytes, 1))
		nomem();
	if (!ulongalloc_ready(&qp, 1))
		nomem();
	if (!ulongalloc_ready(&uid, 1))
		nomem();
	if (!ulongalloc_ready(&numk, 1))
		nomem();
	if (!ulongalloc_ready(&numd, 1))
		nomem();
	if (!ulongalloc_ready(&numz, 1))
		nomem();
	if (!ulongalloc_ready(&del, 1))
		nomem();
	if (!ulongalloc_ready(&dmsg, 1))
		nomem();

	for (;;)
	{
		if (getln(subfdin, &line, &match, '\n') == -1)
			die_read();
		if (!match)
			break;

		if (!stralloc_copy(&outline, &line))
			nomem();

		for (i = 0; i < line.len; ++i)
		{
			ch = line.s[i];
			if ((ch == '\n') || (ch == ' ') || (ch == '\t'))
				line.s[i] = 0;
		}
		j = 0;
		for (i = 0; i < FIELDS; ++i)
		{
			while (j < line.len)
				if (line.s[j])
					break;
				else
					++j;
			field[i] = j;
			while (j < line.len)
				if (!line.s[j])
					break;
				else
					++j;
		}
		if (!stralloc_0(&line))
			nomem();

		if (str_equal(line.s + field[1], "status:"));
		else
		if (str_equal(line.s + field[1], "starting"))
			starting();
		else
		if (str_equal(line.s + field[1], "delivery"))
			delivery();
		else
		if (str_equal(line.s + field[1], "new"))
			newmsg();
		else
		if (str_equal(line.s + field[1], "end"))
			endmsg();
		else
		if (str_equal(line.s + field[1], "info"))
			info();
		else
		if (str_equal(line.s + field[1], "extra"))
			extra();
		else
		if (str_equal(line.s + field[1], "running"))
			clear();
		else
		if (str_equal(line.s + field[1], "exiting"))
			clear();
		else
		if (str_equal(line.s + field[1], "number"));
		else
		if (str_equal(line.s + field[1], "local:"));
		else
		if (str_equal(line.s + field[1], "remote:"));
		else
		if (str_equal(line.s + field[1], "qmail-daemon:"));
		else
		if (str_equal(line.s + field[1], "warning:"))
			out(outline.s, outline.len);
		else
		if (str_equal(line.s + field[1], "alert:"))
			out(outline.s, outline.len);
		else
		{
			outs("? ");
			out(outline.s, outline.len);
		}
	}

	if (substdio_flush(subfdout) == -1)
		die_write();

	pending();

	_exit(0);
  /*- Not reached */
	return (0);
}

void
getversion_matchup_c()
{
	static char    *x = "$Id: matchup.c,v 1.11 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
