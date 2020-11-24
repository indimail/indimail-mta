/*
 * $Log: 822print.c,v $
 * Revision 1.7  2020-11-24 13:43:41+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2016-05-21 14:47:58+05:30  Cprogrammer
 * use auto_sysconfdir for leapsecs_init()
 *
 * Revision 1.5  2016-01-28 08:59:37+05:30  Cprogrammer
 * chdir qmail_home for opening etc/leapsecs.dat
 *
 * Revision 1.4  2005-08-23 17:14:21+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.3  2004-10-22 20:14:19+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-17 23:27:16+05:30  Cprogrammer
 * error handling for substdio_puts(), substdio_flush()
 *
 * Revision 1.1  2004-06-16 01:19:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "substdio.h"
#include "byte.h"
#include "str.h"
#include "subfd.h"
#include "getln.h"
#include "mess822.h"
#include "strerr.h"
#include "leapsecs.h"
#include "caltime.h"
#include "tai.h"
#include "auto_sysconfdir.h"

#define FATAL "822print: fatal: "

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
putspaces(len)
	int             len;
{
	while (len-- > 0)
		substdio_put(subfdout, " ", 1);
}

void
putformat(buf, len)
	char           *buf;
	unsigned int    len;
{
	while (len > 1)
	{
		switch (*buf++)
		{
		case '_':
			substdio_put(subfdout, "_\010", 2);
			break;
		case '+':
			substdio_put(subfdout, buf, 1);
			substdio_put(subfdout, "\010", 1);
			break;
		}
		substdio_put(subfdout, buf++, 1);
		len -= 2;
	}
}

void
format(out, buf, len, pre, style1, style2)
	stralloc       *out;
	char           *buf;
	int             len;
	char           *pre;
	char           *style1;
	char           *style2;
{
	char            ch;
	char            ch2;

	if (!stralloc_copys(out, pre))
		nomem();

	while (len--)
	{
		ch = *buf++;
		if (ch == '\n')
			ch = 0;
		if (ch == '\t')
			ch = ' ';
		if ((ch >= 32) && (ch <= 126))
		{
			if (!stralloc_append(out, style1))
				nomem();
			if (!stralloc_append(out, &ch))
				nomem();
			continue;
		}

		if (!stralloc_cats(out, style2))
			nomem();
		if (!stralloc_append(out, "\\"))
			nomem();
		ch2 = '0' + (ch >> 6);
		if (!stralloc_cats(out, style2))
			nomem();
		if (!stralloc_append(out, &ch2))
			nomem();
		ch2 = '0' + (7 & (ch >> 3));
		if (!stralloc_cats(out, style2))
			nomem();
		if (!stralloc_append(out, &ch2))
			nomem();
		ch2 = '0' + (7 & ch);
		if (!stralloc_cats(out, style2))
			nomem();
		if (!stralloc_append(out, &ch2))
			nomem();
	}
}

stralloc        leftline = { 0 };
stralloc        rightline = { 0 };
stralloc        tmp = { 0 };

void
doleft()
{
	int             len;
	int             pos;
	int             i;

	len = leftline.len / 2;
	pos = 0;
	while (len > 73)
	{
		i = byte_rchr(leftline.s + pos, 73 * 2, ' ') / 2;
		/*XXX*/ if (!i)
			i = 73;
		putformat(leftline.s + pos, i * 2);
		substdio_puts(subfdout, "\n");
		len -= i;
		pos += 2 * i;
	}
	putformat(leftline.s + pos, len * 2);
	substdio_puts(subfdout, "\n");
}

void
doleftright()
{
	int             leftxlen;
	int             leftxpos;
	int             rightxlen;
	int             rightxpos;
	int             i;
	int             j;

	rightxlen = rightline.len / 2;
	leftxlen = leftline.len / 2;

	leftxpos = 0;
	rightxpos = 0;

	while (leftxlen + rightxlen > 76)
	{
		i = 50;
		if (i > leftxlen)
			i = leftxlen;
		j = 50;
		if (j > rightxlen)
			j = rightxlen;
		if (i + j > 76)
			j = 76 - i;

		putformat(leftline.s + leftxpos, i * 2);
		putspaces(78 - i - j);
		putformat(rightline.s + rightxpos, j * 2);
		substdio_put(subfdout, "\n", 1);
		leftxlen -= i;
		leftxpos += 2 * i;
		rightxlen -= j;
		rightxpos += 2 * j;
	}

	putformat(leftline.s + leftxpos, leftxlen * 2);
	putspaces(78 - leftxlen - rightxlen);
	putformat(rightline.s + rightxpos, rightxlen * 2);
	substdio_put(subfdout, "\n", 1);
}

void
putvalue(a, pre)
	stralloc       *a;
	char           *pre;
{
	int             j;

	j = a->len;
	if (j && (a->s[j - 1] == '\n'))
		--j;
	format(&rightline, a->s, j, "", "_", "=");
	format(&leftline, "", 0, pre, "=", "_");

	doleftright();
}

void
putfields(a)
	stralloc       *a;
{
	int             j;
	int             i;

	for (j = i = 0; j < a->len; ++j)
		if (a->s[j] == '\n')
		{
			format(&leftline, a->s + i, j - i, "", "=", "_");
			doleft();
			i = j + 1;
		}
}

void
putaddr(a, pre, post, style)
	stralloc       *a;
	char           *pre;
	char           *post;
	char           *style;
{
	int             i;
	int             j;
	int             comment;
	char           *addr;

	comment = 0;
	for (j = i = 0; j < a->len; ++j)
		if (!a->s[j])
		{
			if (a->s[i] == '(')
			{
				if (comment)
				{
					format(&leftline, a->s + comment, str_len(a->s + comment), pre, "_", "=");
					doleft();
				}
				comment = i + 1;
			} else
			if (a->s[i] == '+')
			{
				addr = a->s + i + 1;
				/*
				 * XXX: replace addr with nickname? 
				 */
				if (comment)
					format(&leftline, a->s + comment, str_len(a->s + comment), pre, "_", "=");
				else
					format(&leftline, "", 0, pre, "=", "_");
				format(&rightline, addr, str_len(addr), post, style, "_");
				doleftright();
				comment = 0;
			}
			i = j + 1;
		}

	if (comment)
	{
		format(&leftline, a->s + comment, str_len(a->s + comment), pre, "_", "=");
		doleft();
	}
}

void
putdate(text, t)
	stralloc       *text;
	mess822_time   *t;
{
	struct tai      sec;
	unsigned char   secpack[TAI_PACK];
	time_t          secunix;
	struct tm      *tm;
	struct caltime  local;
	int             j;

	j = text->len;
	if (j && (text->s[j - 1] == '\n'))
		--j;
	format(&leftline, text->s, j, "=D=a=t=e=:", "=", "_");
	if (!t->known)
	{
		doleft();
		return;
	}

	caltime_tai(&t->ct, &sec);
	tai_pack((char *) secpack, &sec);
	secunix = secpack[0] - 64;
	secunix = (secunix << 8) + secpack[1];
	secunix = (secunix << 8) + secpack[2];
	secunix = (secunix << 8) + secpack[3];
	secunix = (secunix << 8) + secpack[4];
	secunix = (secunix << 8) + secpack[5];
	secunix = (secunix << 8) + secpack[6];
	secunix = (secunix << 8) + secpack[7];
	secunix -= 10;

	tm = localtime(&secunix);
	local.offset = 0;
	local.date.year = tm->tm_year + 1900;
	local.date.month = tm->tm_mon + 1;
	local.date.day = tm->tm_mday;
	local.hour = tm->tm_hour;
	local.minute = tm->tm_min;
	local.second = tm->tm_sec;

	if (!stralloc_ready(&tmp, caltime_fmt((char *) 0, &local)))
		nomem();
	tmp.len = caltime_fmt(tmp.s, &local) - 6;

	format(&rightline, tmp.s, tmp.len, "", "_", "=");
	doleftright();
}

stralloc        returnpath = { 0 };
stralloc        envelope = { 0 };
stralloc        threading = { 0 };
stralloc        mid = { 0 };
stralloc        refs = { 0 };

stralloc        subject = { 0 };

stralloc        dates = { 0 };
mess822_time    date;

stralloc        to = { 0 };
stralloc        cc = { 0 };
stralloc        bcc = { 0 };
stralloc        nrudt = { 0 };

stralloc        from = { 0 };
stralloc        sender = { 0 };
stralloc        replyto = { 0 };
stralloc        mailreplyto = { 0 };
stralloc        followupto = { 0 };

stralloc        misc = { 0 };

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"date", 0, 0, &dates, 0, &date}
	, {"subject", 0, 0, &subject, 0, 0}
	, {"to", 0, 0, 0, &to, 0}
	, {"cc", 0, 0, 0, &cc, 0}
	, {"bcc", 0, 0, 0, &bcc, 0}
	, {"apparently-to", 0, 0, 0, &bcc, 0}
	, {"notice-requested-upon-delivery-to", 0, 0, 0, &nrudt, 0}
	, {"from", 0, 0, 0, &from, 0}
	, {"sender", 0, 0, 0, &sender, 0}
	, {"reply-to", 0, 0, 0, &replyto, 0}
	, {"mail-reply-to", 0, 0, 0, &mailreplyto, 0}
	, {"mail-followup-to", 0, 0, 0, &followupto, 0}
	, {"message-id", 0, 0, 0, &mid, 0}
	, {"references", 0, 0, 0, &refs, 0}
	, {"in-reply-to", 0, &threading, 0, 0, 0}
	, {"return-path", 0, 0, 0, &returnpath, 0}
	, {"received", 0, &envelope, 0, 0, 0}
	, {"delivered-to", 0, &envelope, 0, 0, 0}
	, {"errors-to", 0, &envelope, 0, 0, 0}
	, {"resent-sender", 0, &envelope, 0, 0, 0}
	, {"resent-from", 0, &envelope, 0, 0, 0}
	, {"resent-reply-to", 0, &envelope, 0, 0, 0}
	, {"resent-to", 0, &envelope, 0, 0, 0}
	, {"resent-cc", 0, &envelope, 0, 0, 0}
	, {"resent-bcc", 0, &envelope, 0, 0, 0}
	, {"resent-date", 0, &envelope, 0, 0, 0}
	, {"resent-message-id", 0, &envelope, 0, 0, 0}
	, {0, 0, &misc, 0, 0, 0}
};

void
finishheader()
{
	if (!mess822_end(&h))
		nomem();

	putvalue(&subject, "=S=u=b=j=e=c=t=:");
	putaddr(&from, "=F=r=o=m=:= ", "", "+");

	putdate(&dates, &date);

	putaddr(&returnpath, "=R=e=t=u=r=n=-=P=a=t=h=:= ", "=r=e=t=u=r=n= ", "+");
	putfields(&envelope);
	putfields(&threading);
	putaddr(&refs, "=R=e=f=:= ", "", "_");
	putaddr(&mid, "=M=e=s=s=a=g=e=-=I=D=:= ", "", "_");

	substdio_puts(subfdout, "------------------------------------------------------------------------------\n");

	putaddr(&to, "=T=o=:= ", "", "+");
	putaddr(&cc, "=C=c=:= ", "", "+");
	putaddr(&bcc, "=B=c=c=:= ", "", "+");
	putaddr(&nrudt, "=N=o=t=i=c=e=-=R=e=q=u=e=s=t=e=d=-=U=p=o=n=-=D=e=l=i=v=e=r=y=-=T=o=:= ", "", "+");

	substdio_puts(subfdout, "------------------------------------------------------------------------------\n");

	putaddr(&sender, "=S=e=n=d=e=r=:= ", "=s=e=n=d=e=r= ", "+");
	putaddr(&replyto, "=R=e=p=l=y=-=T=o=:= ", "=r=e=p=l=y= =t=o= ", "+");
	putaddr(&mailreplyto, "=M=a=i=l=-=R=e=p=l=y=-=T=o=:= ", "=r=e=p=l=y= =t=o= ", "+");
	putaddr(&followupto, "=M=a=i=l=-=F=o=l=l=o=w=u=p=-=T=o=:= ", "=f=o=l=l=o=w= =u=p= =t=o= ", "+");

	putfields(&misc);
}

stralloc        line = { 0 };
int             match;

int
main()
{
	int             flagheader = 1;

	if (chdir(auto_sysconfdir) == -1)
		strerr_die3sys(111, FATAL, "chdir: ", auto_sysconfdir);
	if (leapsecs_init() == -1)
		strerr_die2sys(111, FATAL, "unable to init leapsecs: ");
	if (!mess822_begin(&h, a))
		nomem();
	for (;;)
	{
		if (getln(subfdin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (flagheader && !mess822_ok(&line))
		{
			finishheader();
			flagheader = 0;
		}
		if (!flagheader)
		{
			if (substdio_put(subfdout, line.s, line.len))
				strerr_die2sys(111, FATAL, "unable to write: ");
		} else
		if (!mess822_line(&h, &line))
			nomem();
		if (!match)
			break;
	}
	if (flagheader)
		finishheader();
	if (substdio_flush(subfdout))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(0);
}

void
getversion_822print_c()
{
	static char    *x = "$Id: 822print.c,v 1.7 2020-11-24 13:43:41+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
