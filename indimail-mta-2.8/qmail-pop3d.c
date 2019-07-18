/*
 * $Log: qmail-pop3d.c,v $
 * Revision 1.11  2014-01-29 14:03:26+05:30  Cprogrammer
 * BUG - use logs_pidhostinfo.s instead of logs_pidhostinfo
 *
 * Revision 1.10  2009-04-11 19:46:40+05:30  Cprogrammer
 * added logging information to fd 5
 *
 * Revision 1.9  2008-07-15 19:52:19+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.8  2004-10-22 20:28:36+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.7  2004-10-22 15:37:09+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.6  2004-07-17 21:21:00+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "commands.h"
#include "sig.h"
#include "getln.h"
#include "stralloc.h"
#include "substdio.h"
#include "alloc.h"
#include "open.h"
#include "prioq.h"
#include "scan.h"
#include "fmt.h"
#include "str.h"
#include "exit.h"
#include "maildir.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

/*
 * Define to include a "Status: " header in the mail that reflects whether
 * the mail has been read.
 */
#define USE_STATUS_HEADER

/*- Define to include a "X-UIDL: " header to help some clients.  */
#define USE_XUIDL_HEADER

#define __mLOGGING

int             ctl_maxcmdlen = 0;    /*- max length a pop3 command may have */
int             rename(const char *, const char *);
  
#ifdef __mLOGGING
#include "env.h"
void            log_quit();
int             log_flag = 0;

unsigned long   log_bytes = 0;
#endif /* __mLOGGING */

void
die()
{
#ifdef __mLOGGING
	if (!log_flag)
		log_quit();  
#endif /* __mLOGGING */
	_exit(0);
}

ssize_t
saferead(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	r = timeoutread(1200, fd, buf, len);
	if (r <= 0)
		die();
	return r;
}

ssize_t
safewrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;

	if ((r = timeoutwrite(1200, fd, buf, len)) <= 0)
		die();
	return r;
}

char            ssoutbuf[1024];
char            ssinbuf[128];
substdio        ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
substdio        ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);

void
put(buf, len)
	char           *buf;
	int             len;
{
	substdio_put(&ssout, buf, len);
#ifdef __mLOGGING
	log_bytes += len;
#endif /* __mLOGGING */
}

void
my_puts(s)
	char           *s;
{
	substdio_puts(&ssout, s);
}

void
flush()
{
	substdio_flush(&ssout);
}

void
err(s)
	char           *s;
{
	my_puts("-ERR ");
	my_puts(s);
	my_puts("\r\n");
	flush();
}

void
die_nomem()
{
	err("out of memory");
	die();
}

void
die_nomaildir()
{
	err("this user has no $HOME/Maildir");
	die();
}

void
die_scan()
{
	err("unable to scan $HOME/Maildir");
	die();
}

void
err_syntax()
{
	err("syntax error");
}

void
err_unimpl()
{
	err("unimplemented");
}

void
err_deleted()
{
	err("already deleted");
}

void
err_nozero()
{
	err("messages are counted from 1");
}

void
err_toobig()
{
	err("not that many messages");
}

void
err_nosuch()
{
	err("unable to open that message");
}

void
err_nounlink()
{
	err("unable to unlink all deleted messages");
}

void
err_rename()
{
	err("unable to move message from new/ to cur/");
}

void
okay()
{
	my_puts("+OK \r\n");
	flush();
}

void
printfn(fn)
	char           *fn;
{
	fn += 4;
	put(fn, str_chr(fn, ':'));
}

char            strnum[FMT_ULONG];
stralloc        line = { 0 };

stralloc        filenames = { 0 };
prioq           pq = { 0 };

struct message
{
	int             flagdeleted;
	int             flagread;
	unsigned long   size;
	char           *fn;
}
               *m;
int             numm;

void
blast(ssfrom, limit, i)
	substdio       *ssfrom;
	unsigned long   limit;
	int             i;
{
	int             match;
	int             inheaders = 1;
	int             extradone = 0;

	for (;;)
	{
		if (getln(ssfrom, &line, &match, '\n') != 0)
			die();
		if (!match && !line.len)
			break;
		if (match)
			--line.len;	/*- no way to pass this info over POP */
		if (limit)
			if (!inheaders)
				if (!--limit)
					break;
		if (!line.len || !str_diffn(line.s, "Content-Type: ", 14) || !str_diffn(line.s, "-----", 5))
		{
			/*
			 * add our status notification here... 
			 */
#if defined(USE_STATUS_HEADER) || defined(USE_XUIDL_HEADER)
			if (!extradone && (inheaders || !str_diffn(line.s, "Content-Type: ", 14) || !str_diffn(line.s, "-----", 5)))
			{
#ifdef  USE_STATUS_HEADER
				if (m[i].flagread)
					put("Status: RO\r\n", 12);
				else
					put("Status:  U\r\n", 12);
#endif
#ifdef USE_XUIDL_HEADER
				put("X-UIDL: ", 8);
				put(m[i].fn + 4, str_chr(m[i].fn + 4, ':') ? str_chr(m[i].fn + 4, ':') : str_chr(m[i].fn + 4, 0));
				put("\r\n", 2);
#endif
				extradone = 1;
			}
#endif
			inheaders = 0;
		} else
		if (line.s[0] == '.')
			put(".", 1);
		put(line.s, line.len);
		put("\r\n", 2);
		if (!match)
			break;
	}
	put("\r\n.\r\n", 5);
	flush();
}

int             last = 0;

void
getlist()
{
	struct prioq_elt pe;
	struct stat     st;
	int             i;

	maildir_clean(&line);
	if (maildir_scan(&pq, &filenames, 1, 1) == -1)
		die_scan();

	numm = pq.p ? pq.len : 0;
	m = (struct message *) alloc(numm * sizeof(struct message));
	if (!m)
		die_nomem();

	for (i = 0; i < numm; ++i)
	{
		if (!prioq_min(&pq, &pe))
		{
			numm = i;
			break;
		}
		prioq_delmin(&pq);
		m[i].fn = filenames.s + pe.id;
		m[i].flagdeleted = 0;
		m[i].flagread = (m[i].fn[0] == 'c');
		if (stat(m[i].fn, &st) == -1)
			m[i].size = 0;
		else
		{
#ifdef USE_STATUS_HEADER
			m[i].size = st.st_size + 12;	/*- account for the 'NEW' status header */
#else
			m[i].size = st.st_size;
#endif
#ifdef USE_XUIDL_HEADER
			/*
			 * account for the 'X-UIDL' status header
			 */
			m[i].size += (str_chr(m[i].fn, ':') ? str_chr(m[i].fn, ':') : str_chr(m[i].fn, 0)) + 6;
#endif
		}
	}
}

void
pop3_stat()
{
	int             i;
	unsigned long   total;

	total = 0;
	for (i = 0; i < numm; ++i)
		if (!m[i].flagdeleted)
			total += m[i].size;
	my_puts("+OK ");
	put(strnum, fmt_uint(strnum, numm));
	my_puts(" ");
	put(strnum, fmt_ulong(strnum, total));
	my_puts("\r\n");
	flush();
}

void
pop3_rset()
{
	int             i;
	for (i = 0; i < numm; ++i)
		m[i].flagdeleted = 0;
	last = 0;
	okay();
}

void
pop3_last()
{
	my_puts("+OK ");
	put(strnum, fmt_uint(strnum, last));
	my_puts("\r\n");
	flush();
}

void
pop3_quit()
{
	int             i;
	for (i = 0; i < numm; ++i)
	{
		if (m[i].flagdeleted)
		{
			if (unlink(m[i].fn) == -1)
				err_nounlink();
		} else
		if (str_start(m[i].fn, "new/"))
		{
			if (!stralloc_copys(&line, "cur/"))
				die_nomem();
			if (!stralloc_cats(&line, m[i].fn + 4))
				die_nomem();
			if (!stralloc_cats(&line, ":2,"))
				die_nomem();
			if (!stralloc_0(&line))
				die_nomem();
			rename(m[i].fn, line.s);	/*- if it fails, bummer */
		}
	}
	okay();
	die();
}

int
msgno(arg)
	char           *arg;
{
	unsigned long   u;
	if (!scan_ulong(arg, &u))
	{
		err_syntax();
		return -1;
	}
	if (!u)
	{
		err_nozero();
		return -1;
	}
	--u;
	if (u >= numm)
	{
		err_toobig();
		return -1;
	}
	if (m[u].flagdeleted)
	{
		err_deleted();
		return -1;
	}
	return u;
}

void
pop3_dele(arg)
	char           *arg;
{
	int             i;
	i = msgno(arg);
	if (i == -1)
		return;
	m[i].flagdeleted = 1;
	if (i + 1 > last)
		last = i + 1;
	okay();
}

void
list(i, flaguidl)
	int             i;
	int             flaguidl;
{
	put(strnum, fmt_uint(strnum, i + 1));
	my_puts(" ");
	if (flaguidl)
		printfn(m[i].fn);
	else
		put(strnum, fmt_ulong(strnum, m[i].size));
	my_puts("\r\n");
}

void
dolisting(arg, flaguidl)
	char           *arg;
	int             flaguidl;
{
	unsigned int    i;
	if (*arg)
	{
		i = msgno(arg);
		if (i == -1)
			return;
		my_puts("+OK ");
		list(i, flaguidl);
	} else
	{
		okay();
		for (i = 0; i < numm; ++i)
			if (!m[i].flagdeleted)
				list(i, flaguidl);
		my_puts(".\r\n");
	}
	flush();
}

void
pop3_uidl(arg)
	char           *arg;
{
	dolisting(arg, 1);
}

void
pop3_list(arg)
	char           *arg;
{
	dolisting(arg, 0);
}

substdio        ssmsg;
char            ssmsgbuf[1024];

void
do_send(arg, updateread)
	char           *arg;
	int             updateread;
{
	int             i;
	unsigned long   limit;
	int             fd;

	i = msgno(arg);
	if (i == -1)
		return;
	arg += scan_ulong(arg, &limit);
	while (*arg == ' ')
		++arg;
	if (scan_ulong(arg, &limit))
		++limit;
	else
		limit = 0;
	fd = open_read(m[i].fn);
	if (fd == -1)
	{
		err_nosuch();
		return;
	}
	/*
	 * okay(); 
	 */
	my_puts("+OK ");
	put(strnum, fmt_ulong(strnum, m[i].size));
	my_puts(" octets\r\n");
	flush();
	substdio_fdbuf(&ssmsg, read, fd, ssmsgbuf, sizeof(ssmsgbuf));
	blast(&ssmsg, limit, i);
	close(fd);
	if (updateread)
		m[i].flagread = 1;
}

void
pop3_top(arg)
	char           *arg;
{
	do_send(arg, 0);
}

void
pop3_retr(arg)
	char           *arg;
{
	do_send(arg, 1);
}

#ifdef __mLOGGING

stralloc        logs_pidhostinfo = { 0 };
char            sslogbuf[512];
substdio        sslog = SUBSTDIO_FDBUF(safewrite, 5, sslogbuf, sizeof (sslogbuf));

void
my_log(s)
	char           *s;
{
	log_flag = 1;
	if (substdio_puts(&sslog, s) == -1)
		_exit(1);
	log_flag = 0;
}

void
my_logf(s)
	char           *s;
{
	log_flag = 1;
	if (substdio_puts(&sslog, s) == -1)
		_exit(1);
	if (substdio_flush(&sslog) == -1)
		_exit(1);
	log_flag = 0;
}

void
log_quit()
{
	char            strnum[FMT_ULONG];

	my_log("acct:");
	my_log(logs_pidhostinfo.s);
	my_log("logout ");
	strnum[fmt_ulong(strnum, log_bytes)] = 0;
	my_log(strnum);
	my_logf(" bytes transferred\n");
}

void
log_init()
{
	char            strnum[FMT_ULONG];
	char           *remotehost;
	char           *remoteip;
	char           *remoteinfo;
	char           *remoteport;
	char           *user;

	remoteip = env_get("TCPREMOTEIP");
	if (!remoteip)
		remoteip = "unknown";
	remotehost = env_get("TCPREMOTEHOST");
	if (!remotehost)
		remotehost = "unknown";
	remoteinfo = env_get("TCPREMOTEINFO");
	if (!remoteinfo)
		remoteinfo = "";
	remoteport = env_get("TCPREMOTEPORT");
	if (!remoteport)
		remoteport = "";
	user = env_get("USER");
	if (!user)
		user = "unknown";

	if (!stralloc_copys(&logs_pidhostinfo, " pid "))
		die_nomem();
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (!stralloc_cats(&logs_pidhostinfo, strnum))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, ": "))
		die_nomem();

	if (!stralloc_cats(&logs_pidhostinfo, remotehost))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, ":"))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, remoteip))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, ":"))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, remoteinfo))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, ":"))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, remoteport))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, " "))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, user))
		die_nomem();
	if (!stralloc_cats(&logs_pidhostinfo, " "))
		die_nomem();
	if (!stralloc_0(&logs_pidhostinfo))
		die_nomem();
}
#endif							/* __mLOGGING */

struct commands pop3commands[] = {
	{"quit", pop3_quit, 0},
	{"stat", pop3_stat, 0},
	{"list", pop3_list, 0},
	{"uidl", pop3_uidl, 0},
	{"dele", pop3_dele, 0},
	{"retr", pop3_retr, 0},
	{"rset", pop3_rset, 0},
	{"last", pop3_last, 0},
	{"top", pop3_top, 0},
	{"noop", okay, 0},
	{0, err_unimpl, 0}
};

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	sig_alarmcatch(die);
	sig_pipeignore();
#ifdef __mLOGGING
	log_init();
	my_log("acct:");
	my_log(logs_pidhostinfo.s);
	my_logf("login\n");
#endif /* __mLOGGING */
	if (!argv[1])
		die_nomaildir();
	if (chdir(argv[1]) == -1)
		die_nomaildir();
	getlist();
	okay();
	commands(&ssin, pop3commands);
	die();
	/*- Not reached */
	return(0);
}

void
getversion_qmail_pop3d_c()
{
	static char    *x = "$Id: qmail-pop3d.c,v 1.11 2014-01-29 14:03:26+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
