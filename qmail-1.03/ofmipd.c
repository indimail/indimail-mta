/*
 * $Log: ofmipd.c,v $
 * Revision 1.11  2013-06-09 17:02:22+05:30  Cprogrammer
 * fixed addrparse function
 *
 * Revision 1.10  2010-07-27 09:50:46+05:30  Cprogrammer
 * added logging of senders and recipients
 *
 * Revision 1.9  2008-07-15 19:52:10+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.8  2005-08-23 17:33:25+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.7  2005-05-31 15:44:39+05:30  Cprogrammer
 * added authenticated SMTP
 *
 * Revision 1.6  2004-10-22 20:27:45+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:36:12+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-10-11 14:21:02+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.3  2004-07-13 22:44:41+05:30  Cprogrammer
 * control on max length a smtp command may have
 *
 * Revision 1.2  2004-06-17 22:20:05+05:30  Cprogrammer
 * included rwhconfig.h
 *
 * Revision 1.1  2004-06-16 01:19:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "commands.h"
#include "rwhconfig.h"
#include "sig.h"
#include "open.h"
#include "leapsecs.h"
#include "byte.h"
#include "rewritehost.h"
#include "getln.h"
#include "auto_qmail.h"
#include "qmail.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "stralloc.h"
#include "substdio.h"
#include "sconfig.h"
#include "env.h"
#include "exit.h"
#include "error.h"
#include "str.h"
#include "fmt.h"
#include "now.h"
#include "case.h"
#include "base64.h"
#include "wait.h"
#include "mess822.h"
#include "tai.h"
#include "caltime.h"
#include "cdb.h"

int             auth_login(char *);
int             auth_plain(char *);
int             auth_cram();
int             err_noauth();

int             timeout = 1200, auth_smtp = 0;
int             ctl_maxcmdlen=0;    /*- max length a smtp command may have */
int             authd = 0;
int             rcptcount = 0;
unsigned int    databytes = 0;
static char     strnum[FMT_ULONG], pid_str[FMT_ULONG], accept_buf[FMT_ULONG];
char           *protocol = "SMTP";
char           *hostname, *remoteip, *relayclient, *remoteinfo;
char          **childargs;
struct authcmd
{
	char           *text;
	int             (*fun) ();
} authcmds[] =
{
	{"login", auth_login},
	{"plain", auth_plain},
	{"cram-md5", auth_cram},
	{0, err_noauth}
};

static stralloc authin = { 0 };
static stralloc user = { 0 };
static stralloc pass = { 0 };
static stralloc resp = { 0 };
static stralloc slop = { 0 };

void            logerr(char *);
void            logerrf(char *);
void            logerr_start();

ssize_t
safewrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;

	if ((r = timeoutwrite(timeout, fd, buf, len)) <= 0)
	{
		logerr_start();
		logerrf("write error (disconnect?): quitting\n");
		_exit(1);
	}
	return r;
}

char            ssoutbuf[512], sserrbuf[512];
substdio        ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
substdio        sserr = SUBSTDIO_FDBUF(safewrite, 2, sserrbuf, sizeof sserrbuf);

void
flush()
{
	substdio_flush(&ssout);
}

void
out(s)
	char           *s;
{
	substdio_puts(&ssout, s);
}

void
logerr(s)
	char           *s;
{
	if (substdio_puts(&sserr, s))
		_exit (1);
}

void
logerr_start()
{
	logerr("ofmpid: ");
	if (*pid_str == '?')
		pid_str[fmt_ulong(pid_str, getpid())] = 0;
	logerr("pid ");
	logerr(pid_str);
	logerr(" from ");
	logerr(remoteip);
	logerr(" ");
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s))
		_exit (1);
	if (substdio_flush(&sserr))
		_exit (1);
}

void
log_trans(char *arg1, char *arg2, int rcptlen, char *arg3)
{
	char           *ptr;
	int             idx;

	for (ptr = arg2 + 1, idx = 0; idx < rcptlen; idx++)
	{
		if (!arg2[idx])
		{
			logerr_start();
			logerr("MAIL from <");
			logerr(arg1);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> AUTH <");
			if (arg3 && *arg3)
			{
				logerr(arg3);
				switch (authd)
				{
				case 0:
					break;
				case 1:
					logerr(": AUTH LOGIN");
				break;
				case 2:
					logerr(": AUTH PLAIN");
					break;
				case 3:
					logerr(": AUTH CRAM-MD5");
					break;
				default:
					logerr(": AUTH unknown");
					break;
				}
			}
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, databytes)] = 0;
			logerr(strnum);
			logerr("\n");
			ptr = arg2 + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
err_queue(char *arg1, char *arg2, int len, char *arg3, char *qqx,
	int permanent, unsigned long qp)
{
	char           *ptr;
	int             idx;

	accept_buf[fmt_ulong(accept_buf, qp)] = 0;
	strnum[fmt_ulong(strnum, databytes)] = 0;
	for (ptr = arg2 + 1, idx = 0; idx < len; idx++)
	{
		if (!arg2[idx])
		{
			logerr_start();
			logerr(qqx);
			if (permanent)
				logerr(" (permanent): ");
			else
				logerr(" (temporary): ");
			logerr("MAIL from <");
			logerr(arg1);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> AUTH <");
			if (arg3 && *arg3)
			{
				logerr(arg3);
				switch (authd)
				{
				case 0:
					break;
				case 1:
					logerr(": AUTH LOGIN");
					break;
				case 2:
					logerr(": AUTH PLAIN");
					break;
				case 3:
					logerr(": AUTH CRAM-MD5");
					break;
				default:
					logerr(": AUTH unknown");
					break;
				}
			}
			logerr("> Size: ");
			logerr(strnum);
			logerr(" qp ");
			logerr(accept_buf);
			logerr("\n");
			ptr = arg2 + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
die_read()
{
	logerr_start();
	logerrf("read error (disconnect?): quitting\n");
	_exit(1);
}

int
err_authabrt()
{
	out("501 auth exchange cancelled (#5.0.0)\r\n");
	return -1;
}

int
err_pipe()
{
	out("451 Requested action aborted: unable to open pipe and I can't auth (#4.3.0)\r\n");
	return -1;
}

void
err_authd()
{
	out("503 you're already authenticated (#5.5.0)\r\n");
}

void
err_authrequired()
{
	out("530 authentication required (#5.7.1)\r\n");
}

int
err_noauth()
{
	out("504 auth type unimplemented (#5.5.1)\r\n");
	return -1;
}

int
err_write()
{
	out("451 Requested action aborted: unable to write pipe and I can't auth (#4.3.0)\r\n");
	return -1;
}

int
err_input()
{
	out("501 malformed auth input (#5.5.4)\r\n");
	return -1;
}

int
err_child()
{
	out("451 Requested action aborted: problem with child and I can't auth (#4.3.0)\r\n");
	return -1;
}

int
err_fork()
{
	out("451 Requested action aborted: child won't start and I can't auth (#4.3.0)\r\n");
	return -1;
}

void
nomem()
{
	out("451 out of memory (#4.3.0)\r\n");
	flush();
	logerr_start();
	logerrf("out of memory: quitting\n");
	_exit(1);
}

void
die_config()
{
	out("451 unable to read configuration (#4.3.0)\r\n");
	flush();
	logerr_start();
	logerrf("unable to read configuration: quitting\n");
	_exit(1);
}

void
smtp_quit()
{
	out("221 ofmipd.local\r\n");
	flush();
	_exit(0);
}

void
smtp_help()
{
	out("214 qmail home page: http://pobox.com/~djb/qmail.html\r\n");
}

void
smtp_noop()
{
	out("250 ok\r\n");
}

void
smtp_vrfy()
{
	out("252 send some mail, i'll try my best\r\n");
}

void
smtp_unimpl()
{
	out("502 unimplemented (#5.5.1)\r\n");
}

void
err_syntax()
{
	out("555 syntax error (#5.5.4)\r\n");
}

void
err_wantmail()
{
	out("503 MAIL first (#5.5.1)\r\n");
}

void
err_wantrcpt()
{
	out("503 RCPT first (#5.5.1)\r\n");
}

void
err_qqt()
{
	out("451 qqt failure (#4.3.0)\r\n");
	logerr_start();
	logerrf("qqt failure\n");
}

void
err_cdb()
{
	out("451 unable to read cdb (#4.3.0)\r\n");
	logerr_start();
	logerrf("unable to read cdb: quitting\n");
}

config_str      rewrite = CONFIG_STR;
stralloc        idappend = { 0 };

stralloc        addr = { 0 };	/*- will be 0-terminated, if addrparse returns 1 */
stralloc        rwaddr = { 0 };

int
addrparse(arg)
	char           *arg;
{
	int             i, flagesc, flagquoted;
	char            ch, terminator;

	terminator = '>';
	i = str_chr(arg, '<');
	if (arg[i])
		arg += i + 1;
	else { /*- partner should go read rfc 821 */
		terminator = ' ';
		arg += str_chr(arg, ':');
		if (*arg == ':')
			++arg;
		if (!*arg)
			return (0);
		while (*arg == ' ')
			++arg;
	}
	if (*arg == '@') {
		while (*arg)
			if (*arg++ == ':')
				break;
	}
	if (!stralloc_copys(&addr, ""))
		nomem();
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) { /*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				nomem();
			flagesc = 0;
		} else {
			if (!flagquoted && (ch == terminator))
				break;
			switch (ch)
			{
			case '\\':
				flagesc = 1;
				break;
			case '"':
				flagquoted = !flagquoted;
				break;
			default:
				if (!stralloc_append(&addr, &ch))
					nomem();
			}
		}
	}
	if (!rewritehost_addr(&rwaddr, addr.s, addr.len, config_data(&rewrite)))
		nomem();
	return rwaddr.len < 900;
}

char           *fncdb;
int             fdcdb;
stralloc        cdbresult = { 0 };

int             seenmail = 0;
char           *name;			/*- defined if seenmail; points into cdbresult */

stralloc        mailfrom = { 0 };
stralloc        rcptto = { 0 };

void
smtp_helo(arg)
	char           *arg;
{
	seenmail = 0;
	out("250 ofmipd.local\r\n");
}

void
smtp_ehlo(arg)
	char           *arg;
{
	seenmail = 0;
	out("250-ofmipd.local\r\n");
	if (auth_smtp)
	{
		out("250-AUTH LOGIN CRAM-MD5 PLAIN\r\n");
		out("250-AUTH=LOGIN CRAM-MD5 PLAIN\r\n");
	}
	out("250-PIPELINING\r\n250 8BITMIME\r\n");
}

void
smtp_rset()
{
	seenmail = 0;
	out("250 flushed\r\n");
}

void
smtp_mail(arg)
	char           *arg;
{
	if (env_get("REQUIREAUTH") && !authd)
	{
		err_authrequired();
		return;
	}
	if (!addrparse(arg))
	{
		err_syntax();
		return;
	}
	name = 0;
	if (fncdb)
	{
		uint32          dlen;
		int             r;

		if ((r = cdb_seek(fdcdb, rwaddr.s, rwaddr.len, &dlen)) == -1)
		{
			err_cdb();
			return;
		}
		if (r)
		{
			if (!stralloc_ready(&cdbresult, (unsigned int) dlen))
				nomem();
			cdbresult.len = dlen;
			name = cdbresult.s;
			if (cdb_bread(fdcdb, name, cdbresult.len) == -1)
			{
				err_cdb();
				return;
			}
			r = byte_chr(name, cdbresult.len, '\0');
			if (r == cdbresult.len)
			{
				err_cdb();
				return;
			}
			if (!stralloc_copyb(&rwaddr, cdbresult.s + r + 1, cdbresult.len - r - 1))
				nomem();
		}
	}
	if (!stralloc_copy(&mailfrom, &rwaddr))
		nomem();
	if (!stralloc_0(&mailfrom))
		nomem();
	if (!stralloc_copys(&rcptto, ""))
		nomem();
	seenmail = 1;
	rcptcount = 0;
	out("250 ok\r\n");
}

void
smtp_rcpt(arg)
	char           *arg;
{
	if (!seenmail)
	{
		err_wantmail();
		return;
	}
	if (!addrparse(arg))
	{
		err_syntax();
		return;
	}
	if (!stralloc_0(&rwaddr))
		nomem();
	if (!stralloc_cats(&rcptto, "T"))
		nomem();
	if (!stralloc_cats(&rcptto, rwaddr.s))
		nomem();
	if (!stralloc_0(&rcptto))
		nomem();
	++rcptcount;
	out("250 ok\r\n");
}

struct qmail    qqt;

void
put(buf, len)
	char           *buf;
	int             len;
{
	qmail_put(&qqt, buf, len);
	databytes += len;
}

void
myputs(buf)
	char           *buf;
{
	int             len;

	len = str_len(buf);
	qmail_put(&qqt, buf, len);
	databytes += len;
}

stralloc        tmp = { 0 };
stralloc        tmp2 = { 0 };

void
rewritelist(list)
	stralloc       *list;
{
	if (!rewritehost_list(&tmp, list->s, list->len, config_data(&rewrite)))
		nomem();
	if (!stralloc_copy(list, &tmp))
		nomem();
}

void
putlist(name, list)
	char           *name;
	stralloc       *list;
{
	if (!list->len)
		return;
	if (!mess822_quotelist(&tmp, list))
		nomem();
	if (!mess822_fold(&tmp2, &tmp, name, 78))
		nomem();
	put(tmp2.s, tmp2.len);
}

mess822_time    datastart;
stralloc        datastamp = { 0 };

mess822_time    date;
stralloc        to = { 0 };
stralloc        cc = { 0 };
stralloc        nrudt = { 0 };
stralloc        from = { 0 };
stralloc        headersender = { 0 };
stralloc        replyto = { 0 };
stralloc        mailreplyto = { 0 };
stralloc        followupto = { 0 };

stralloc        msgid = { 0 };
stralloc        top = { 0 };
stralloc        bottom = { 0 };

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"date", 0, 0, 0, 0, &date}
	, {"to", 0, 0, 0, &to, 0}
	, {"cc", 0, 0, 0, &cc, 0}
	, {"notice-requested-upon-delivery-to", 0, 0, 0, &nrudt, 0}
	, {"from", 0, 0, 0, &from, 0}
	, {"sender", 0, 0, 0, &headersender, 0}
	, {"reply-to", 0, 0, 0, &replyto, 0}
	, {"mail-reply-to", 0, 0, 0, &mailreplyto, 0}
	, {"mail-followup-to", 0, 0, 0, &followupto, 0}
	, {"message-id", 0, &msgid, 0, 0, 0}
	, {"received", 0, &top, 0, 0, 0}
	, {"delivered-to", 0, &top, 0, 0, 0}
	, {"errors-to", 0, &top, 0, 0, 0}
	, {"return-receipt-to", 0, &top, 0, 0, 0}
	, {"resent-sender", 0, &top, 0, 0, 0}
	, {"resent-from", 0, &top, 0, 0, 0}
	, {"resent-reply-to", 0, &top, 0, 0, 0}
	, {"resent-to", 0, &top, 0, 0, 0}
	, {"resent-cc", 0, &top, 0, 0, 0}
	, {"resent-bcc", 0, &top, 0, 0, 0}
	, {"resent-date", 0, &top, 0, 0, 0}
	, {"resent-message-id", 0, &top, 0, 0, 0}
	, {"bcc", 0, 0, 0, 0, 0}
	, {"return-path", 0, 0, 0, 0, 0}
	, {"apparently-to", 0, 0, 0, 0, 0}
	, {"content-length", 0, 0, 0, 0, 0}
	, {0, 0, &bottom, 0, 0, 0}
};

void
finishheader()
{
	if (!mess822_end(&h))
		nomem();

	if (name)
		from.len = 0;

	rewritelist(&to);
	rewritelist(&cc);
	rewritelist(&nrudt);
	rewritelist(&from);
	rewritelist(&headersender);
	rewritelist(&replyto);
	rewritelist(&mailreplyto);
	rewritelist(&followupto);

	put(top.s, top.len);

	if (!date.known)
		date = datastart;
	if (!mess822_date(&tmp, &date))
		nomem();
	myputs("Date: ");
	put(tmp.s, tmp.len);
	myputs("\n");

	if (!msgid.len)
	{
		static int      idcounter = 0;

		if (!stralloc_copys(&msgid, "Message-ID: <"))
			nomem();
		if (!stralloc_catlong(&msgid, date.ct.date.year))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.date.month, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.date.day, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.hour, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.minute, 2))
			nomem();
		if (!stralloc_catint0(&msgid, date.ct.second, 2))
			nomem();
		if (!stralloc_cats(&msgid, "."))
			nomem();
		if (!stralloc_catint(&msgid, ++idcounter))
			nomem();
		if (!stralloc_cat(&msgid, &idappend))
			nomem();
		if (!stralloc_cats(&msgid, ">\n"))
			nomem();
	}
	put(msgid.s, msgid.len);

	putlist("From: ", &from);
	if (!from.len)
	{
		myputs("From: ");
		if (!mess822_quote(&tmp, mailfrom.s, name))
			nomem();
		put(tmp.s, tmp.len);
		myputs("\n");
	}

	putlist("Sender: ", &headersender);
	putlist("Reply-To: ", &replyto);
	putlist("Mail-Reply-To: ", &mailreplyto);
	putlist("Mail-Followup-To: ", &followupto);
	if (!to.len && !cc.len)
		myputs("Cc: recipient list not shown: ;\n");
	putlist("To: ", &to);
	putlist("Cc: ", &cc);
	putlist("Notice-Requested-Upon-Delivery-To: ", &nrudt);

	put(bottom.s, bottom.len);
}

ssize_t
saferead(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	flush();
	r = timeoutread(timeout, fd, buf, len);
	if (r <= 0)
		die_read();
	return r;
}

char            ssinbuf[1024];
substdio        ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);

stralloc        line = { 0 };
int             match;

void
blast()
{
	int             flagheader = 1;
	int             i;

	if (!mess822_begin(&h, a))
		nomem();

	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') == -1)
			die_read();
		if (!match)
			die_read();

		--line.len;
		if (line.len && (line.s[line.len - 1] == '\r'))
			--line.len;
		if (line.len && (line.s[0] == '.'))
		{
			--line.len;
			if (!line.len)
				break;
			for (i = 0; i < line.len; ++i)
				line.s[i] = line.s[i + 1];
		}
		line.s[line.len++] = '\n';

		if (flagheader)
			if (!mess822_ok(&line))
			{
				finishheader();
				flagheader = 0;
				if (line.len > 1)
					put("\n", 1);
			}
		if (!flagheader)
			put(line.s, line.len);
		else
		if (!mess822_line(&h, &line))
			nomem();
	}

	if (flagheader)
		finishheader();
}

stralloc        received = { 0 };

void
smtp_data()
{
	struct tai      now;
	char           *qqx;
	unsigned long   qp;

	tai_now(&now);
	caltime_utc(&datastart.ct, &now, (int *) 0, (int *) 0);
	datastart.known = 1;
	if (!mess822_date(&datastamp, &datastart))
		nomem();
	if (!seenmail)
	{
		err_wantmail();
		return;
	}
	if (!rcptto.len)
	{
		err_wantrcpt();
		return;
	}
	seenmail = 0;
	databytes = 0;
	if (qmail_open(&qqt) == -1)
	{
		err_qqt();
		return;
	}
	qp = qmail_qp(&qqt); /*- pid of queue process */
	out("354 go ahead\r\n");
	qmail_put(&qqt, received.s, received.len);
	qmail_put(&qqt, datastamp.s, datastamp.len);
	qmail_puts(&qqt, "\n");
	databytes += (received.len + datastamp.len + 1);
	blast();
	qmail_from(&qqt, mailfrom.s);
	qmail_put(&qqt, rcptto.s, rcptto.len);
	qqx = qmail_close(&qqt);
	if (!*qqx)
	{
		out("250 ok\r\n");
		log_trans(mailfrom.s, rcptto.s, rcptto.len, authd ? remoteinfo : 0);
		return;
	}
	if (*qqx == 'D')
		out("554 ");
	else
		out("451 ");
	out(qqx + 1);
	out("\r\n");
	err_queue(mailfrom.s, rcptto.s, rcptto.len, authd ? remoteinfo : 0,
		qqx + 1, *qqx == 'D', qp);
}

void
safecats(out, in)
	stralloc       *out;
	char           *in;
{
	char            ch;
	while ((ch = *in++))
	{
		if (ch < 33)
			ch = '?';
		if (ch > 126)
			ch = '?';
		if (ch == '(')
			ch = '?';
		if (ch == ')')
			ch = '?';
		if (ch == '@')
			ch = '?';
		if (ch == '\\')
			ch = '?';
		if (!stralloc_append(out, &ch))
			nomem();
	}
}

void
received_init()
{
	char           *x;

	if (!stralloc_copys(&received, "Received: (ofmipd "))
		nomem();
	if (remoteinfo)
	{
		safecats(&received, remoteinfo);
		if (!stralloc_append(&received, "@"))
			nomem();
	}
	if (!(x = env_get("TCPREMOTEIP")))
		x = "unknown";
	safecats(&received, x);
	if (!stralloc_cats(&received, "); "))
		nomem();
}

int
authgetl(void)
{
	int             i;

	if (!stralloc_copys(&authin, ""))
		nomem();
	for (;;)
	{
		if (!stralloc_readyplus(&authin, 1))
			nomem(); /*- XXX */
		i = substdio_get(&ssin, authin.s + authin.len, 1);
		if (i != 1)
			die_read();
		if (authin.s[authin.len] == '\n')
			break;
		++authin.len;
	}
	if (authin.len > 0 && authin.s[authin.len - 1] == '\r')
		--authin.len;
	authin.s[authin.len] = 0;
	if (*authin.s == '*' && *(authin.s + 1) == 0)
		return err_authabrt();
	if (authin.len == 0)
		return err_input();
	return (authin.len);
}

static char     upbuf[128];
static substdio ssup;

int
authenticate(void)
{
	int             child;
	int             wstat;
	int             pi[2];

	if (!stralloc_0(&user))
		nomem();
	if (!stralloc_0(&pass))
		nomem();
	if (!stralloc_0(&resp))
		nomem();
	if (pipe(pi) == -1)
		return err_pipe();
	switch (child = fork())
	{
	case -1:
		return err_fork();
	case 0:
		close(pi[1]);
		if (pi[0] != 3)
		{
			dup2(pi[0], 3);
			close(pi[0]);
		}
		sig_pipedefault();
		execvp(*childargs, childargs);
		_exit(1);
	}
	close(pi[0]);
	substdio_fdbuf(&ssup, write, pi[1], upbuf, sizeof upbuf);
	if (substdio_put(&ssup, user.s, user.len) == -1)
		return err_write();
	if (substdio_put(&ssup, pass.s, pass.len) == -1)
		return err_write();
	if (substdio_put(&ssup, resp.s, resp.len) == -1)
		return err_write();
	if (substdio_flush(&ssup) == -1)
		return err_write();
	close(pi[1]);
	byte_zero(pass.s, pass.len);
	byte_zero(upbuf, sizeof upbuf);
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	if (wait_crashed(wstat))
		return err_child();
	return (wait_exitcode(wstat));
}

int
auth_login(arg)
	char           *arg;
{
	int             r;

	if (*arg)
	{
		if ((r = b64decode((const unsigned char *) arg, str_len(arg), &user)) == 1)
			return err_input();
	} else
	{
		out("334 VXNlcm5hbWU6\r\n");
		flush(); /*- Username: */
		if (authgetl() < 0)
			return -1;
		if ((r = b64decode((const unsigned char *) authin.s, authin.len, &user)) == 1)
			return err_input();
	}
	if (r == -1)
		nomem();
	out("334 UGFzc3dvcmQ6\r\n");
	flush(); /*- Password: */
	if (authgetl() < 0)
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &pass)) == 1)
		return err_input();
	if (r == -1)
		nomem();
	if (!user.len || !pass.len)
		return err_input();
	r = authenticate();
	if (!r || r == 3)
		authd = 1;
	return (r);
}

int
auth_plain(arg)
	char           *arg;
{
	int             r, id = 0;

	if (*arg)
	{
		if ((r = b64decode((const unsigned char *) arg, str_len(arg), &slop)) == 1)
			return err_input();
	} else
	{
		out("334 \r\n");
		flush();
		if (authgetl() < 0)
			return -1;
		if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
			return err_input();
	}
	if (r == -1 || !stralloc_0(&slop))
		nomem();
	while (slop.s[id])
		id++; /*- ignore authorize-id */
	if (slop.len > id + 1)
		if (!stralloc_copys(&user, slop.s + id + 1))
			nomem();
	if (slop.len > id + user.len + 2)
		if (!stralloc_copys(&pass, slop.s + id + user.len + 2))
			nomem();
	if (!user.len || !pass.len)
		return err_input();
	r = authenticate();
	if (!r || r == 3)
		authd = 2;
	return (r);
}

int
auth_cram()
{
	int             i, r;
	char           *s;
	char            unique[FMT_ULONG + FMT_ULONG + 3];

	s = unique;
	s += fmt_uint(s, getpid());
	*s++ = '.';
	s += fmt_ulong(s, (unsigned long) now());
	*s++ = '@';
	*s++ = 0;
	if (!stralloc_copys(&pass, "<")) /*- generate challenge */
		nomem();
	if (!stralloc_cats(&pass, unique))
		nomem();
	if (!stralloc_cats(&pass, hostname))
		nomem();
	if (!stralloc_cats(&pass, ">"))
		nomem();
	if (b64encode(&pass, &slop) < 0)
		nomem();
	if (!stralloc_0(&slop))
		nomem();
	out("334 ");	/*- "334 mychallenge \r\n" */
	out(slop.s);
	out("\r\n");
	flush();
	if (authgetl() < 0)	/*- got response */
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
		return err_input();
	if (r == -1 || !stralloc_0(&slop))
		nomem();
	i = str_chr(slop.s, ' ');
	s = slop.s + i;
	while (*s == ' ')
		++s;
	slop.s[i] = 0;
	if (!stralloc_copys(&user, slop.s))	/*- userid */
		nomem();
	if (!stralloc_copys(&resp, s)) /*- digest */
		nomem();
	if (!user.len || !resp.len)
		return err_input();
	r = authenticate();
	if (!r || r == 3)
		authd = 3;
	return (r);
}

void
smtp_auth(arg)
	char           *arg;
{
	int             i;
	char           *cmd = arg;

	if (env_get("SHUTDOWN"))
	{
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	}
	if (!hostname || !*hostname || !childargs || !*childargs)
	{
		out("503 auth not available (#5.3.3)\r\n");
		return;
	}
	if (authd)
	{
		err_authd();
		return;
	}
	if (seenmail)
	{
		out("503 no auth during mail transaction (#5.5.0)\r\n");
		return;
	}
	if (!stralloc_copys(&user, ""))
		nomem();
	if (!stralloc_copys(&pass, ""))
		nomem();
	if (!stralloc_copys(&resp, ""))
		nomem();
	i = str_chr(cmd, ' ');
	arg = cmd + i;
	while (*arg == ' ')
		++arg;
	cmd[i] = 0;
	for (i = 0; authcmds[i].text; ++i)
	{
		if (case_equals(authcmds[i].text, cmd))
			break;
	}
	switch (authcmds[i].fun(arg))
	{
	case 0:
		relayclient = "";
	case 3:
		remoteinfo = user.s;
		if (!env_put2("TCPREMOTEINFO", remoteinfo))
			nomem();
		protocol = "ESMTPA";
		out("235 ok, go ahead (#2.0.0)\r\n");
		break;
	case 1:
	case 2:
		sleep(5);
		out("535 authorization failed (#5.7.1)\r\n");
		break;
	case -1:
		break;
	default:
		err_child();
	}
	return;
}

struct commands smtpcommands[] = {
	{"rcpt", smtp_rcpt, 0},
	{"mail", smtp_mail, 0},
	{"data", smtp_data, flush},
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"rset", smtp_rset, 0},
	{"help", smtp_help, flush},
	{"noop", smtp_noop, flush},
	{"vrfy", smtp_vrfy, flush},
	{0, smtp_unimpl, flush}
};

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	sig_pipeignore();

	if (!(remoteip = env_get("TCPREMOTEIP")))
		remoteip = "unknown";
	fncdb = argv[1];
	if (fncdb && *fncdb)
	{
		if ((fdcdb = open_read(fncdb)) == -1)
			die_config();
	} else
		fncdb = 0;
	if (argc > 3)
	{
		hostname = argv[2];
		childargs = argv + 3;
		auth_smtp = 1;
	}
	remoteinfo = env_get("REMOTEINFO");
	relayclient = env_get("RELAYCLIENT");
	received_init();
	if (leapsecs_init() == -1)
		die_config();
	if (chdir(auto_qmail) == -1)
		die_config();
	if (rwhconfig(&rewrite, &idappend) == -1)
		die_config();
	out("220 ofmipd.local ESMTP\r\n");
	commands(&ssin, smtpcommands);
	nomem();
  /*- Not reached */
	return (0);
}

void
getversion_ofmipd_c()
{
	static char    *x = "$Id: ofmipd.c,v 1.11 2013-06-09 17:02:22+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
