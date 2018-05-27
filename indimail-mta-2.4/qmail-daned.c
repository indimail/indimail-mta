/*
 * $Log: qmail-daned.c,v $
 * Revision 1.10  2018-05-28 01:16:18+05:30  Cprogrammer
 * removed redundant setting of smtptext
 *
 * Revision 1.9  2018-05-27 17:46:06+05:30  Cprogrammer
 * added option for qmail-remote to query/update records
 *
 * Revision 1.8  2018-05-27 14:06:14+05:30  Cprogrammer
 * removed DANEPROG execution method
 *
 * Revision 1.7  2018-05-27 12:32:40+05:30  Cprogrammer
 * refactored code. Organized error messages and error codes in child
 *
 * Revision 1.6  2018-05-26 16:00:30+05:30  Cprogrammer
 * replaced getdns lib with inbuilt dns_tlsarr() function in dns.c
 *
 * Revision 1.5  2018-05-25 08:33:40+05:30  Cprogrammer
 * use do_tlsa_query() to get TLSA RR
 *
 * Revision 1.4  2018-05-13 21:34:29+05:30  Cprogrammer
 * renamed RECORD_STALE to RECORD_OLD
 *
 * Revision 1.3  2018-04-27 10:47:53+05:30  Cprogrammer
 * fixed memory leak due to multiple records getting added
 *
 * Revision 1.2  2018-04-26 01:32:30+05:30  Cprogrammer
 * added tlsadomains control file
 *
 * Revision 1.1  2018-04-25 23:01:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hastlsa.h"
#include "subfd.h"
#include "exit.h"
#if defined(HASTLSA) && defined(TLS)
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <search.h>
#define __USE_GNU
#include <netdb.h>
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "stralloc.h"
#include "tlsarralloc.h"
#include "case.h"
#include "socket.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "tls.h"
#include "ssl_timeoutio.h"
#include "gen_alloc.h"
#include "gen_allocdefs.h"
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <sys/stat.h>
#include "constmap.h"
#include "env.h"
#include "control.h"
#include "strerr.h"
#include "uint32.h"
#include "sig.h"
#include "getln.h"
#include "fmt.h"
#include "wait.h"
#include "str.h"
#include "byte.h"
#include "scan.h"
#include "variables.h"
#include "open.h"
#include "error.h"
#include "cdb.h"
#include "sgetopt.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "tlsacheck.h"
#include "haveip6.h"
#include "dns.h"
#include "ip.h"
#include "ipalloc.h"
#include "now.h"

#define FATAL "qmail-daned: fatal: "
#define WARN  "qmail-daned: warning: "

#define BLOCK_SIZE 32768

union sockunion
{
	struct sockaddr     sa;
	struct sockaddr_in  sa4;
#ifdef INET6
	struct sockaddr_in6 sa6;
#endif
};

struct danerec
{
	char           *domain;
	char           *data;
	int             datalen;
	time_t          timestamp;
	int             attempts;
	char            status; /*- dane verification status */
	struct danerec *prev, *next; /*- prev, next of linked list of danerec structures */
};
struct danerec *head;
struct danerec *tail;
int             dane_count, hcount;
int             hash_size, h_allocated = 0;
int             verbose = 0;
unsigned long   timeout;
char           *whitefn = 0, *tlsadomainsfn = 0;
stralloc        context_file = { 0 };
stralloc        line = { 0 };
stralloc        save = { 0 };
stralloc        sa = { 0 };
ipalloc         ia = { 0 };
struct substdio ssin;

extern char   **MakeArgs(char *);
tlsarralloc     ta = { 0 };
stralloc        hexstring = { 0 };

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
die(int e)
{
	if (ssl) {
		while (SSL_shutdown(ssl) == 0);
		SSL_free(ssl);
		ssl = 0;
	}
	_exit (e);
}

void
die_nomem_child()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, FATAL);
	substdio_puts(subfderr, "out of memory\n");
	substdio_flush(subfderr);
	die (2);
}

void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, FATAL);
	substdio_puts(subfderr, "out of memory\n");
	substdio_flush(subfderr);
	die (111);
}

void
die_control(char *arg)
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, FATAL);
	substdio_puts(subfderr, "unable to read controls: ");
	substdio_puts(subfderr, arg);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	die (111);
}

void
logerr(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1) {
		strerr_warn2(FATAL, "write: ", &strerr_sys);
		die(111);
	}
	return;
}

void
logerrf(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1 || substdio_flush(subfderr) == -1) {
		strerr_warn2(FATAL, "write: ", &strerr_sys);
		die(111);
	}
}

int             whitelistok = 0;
stralloc        whitelist = { 0 };
struct constmap mapwhite;
int             tlsadomainsok = 0;
stralloc        tlsadomains = { 0 };
struct constmap maptlsadomains;

void
tlsadomains_init(char *arg)
{
	if (verbose > 2)
		out("initializing tlsadomains\n");
#ifdef NETQMAIL /*- look for control files in QMAILHOME/control */
	static stralloc controlfile = {0};

	if (!stralloc_copys(&controlfile, "control/"))
		die_nomem();
	if (!stralloc_cats(&controlfile, arg))
		die_nomem();
	if (!stralloc_0(&controlfile))
		die_nomem();
	if ((tlsadomainsok = control_readfile(&tlsadomains, controlfile.s, 0)) == -1)
#else /*- look for control files in $CONTROLDIR/control */
	if ((tlsadomainsok = control_readfile(&tlsadomains, arg, 0)) == -1)
#endif
		die_control(arg);
	if (tlsadomainsok && !constmap_init(&maptlsadomains, tlsadomains.s, tlsadomains.len, 0))
		die_nomem();
	if (verbose > 2)
		out("initialized  tlsadomains\n");
	return;
}

void
whitelist_init(char *arg)
{
	if (verbose > 2)
		out("initializing whitelist\n");
#ifdef NETQMAIL /*- look for control files in QMAILHOME/control */
	static stralloc controlfile = {0};

	if (!stralloc_copys(&controlfile, "control/"))
		die_nomem();
	if (!stralloc_cats(&controlfile, arg))
		die_nomem();
	if (!stralloc_0(&controlfile))
		die_nomem();
	if ((whitelistok = control_readfile(&whitelist, controlfile.s, 0)) == -1)
#else /*- look for control files in $CONTROLDIR/control */
	if ((whitelistok = control_readfile(&whitelist, arg, 0)) == -1)
		die_control(arg);
#endif
	if (whitelistok && !constmap_init(&mapwhite, whitelist.s, whitelist.len, 0))
		die_nomem();
	if (verbose > 2)
		out("initialized  whitelist\n");
	return;
}

int
cdb_match(char *fn, char *addr, int len)
{
	static stralloc controlfile = {0};
	static stralloc temp = { 0 };
	uint32          dlen;
	int             fd_cdb, cntrl_ok;

	if (!len || !*addr || !fn)
		return (0);
	if (!stralloc_copys(&controlfile, controldir))
		die_nomem();
	if (!stralloc_cats(&controlfile, "/"))
		die_nomem();
	if (!stralloc_cats(&controlfile, fn))
		die_nomem();
	if (!stralloc_cats(&controlfile, ".cdb"))
		die_nomem();
	if (!stralloc_0(&controlfile))
		die_nomem();
	if ((fd_cdb = open_read(controlfile.s)) == -1) {
		if (errno != error_noent)
			die_control(controlfile.s);
		/*- cdb missing or entry missing */
		return (0);
	}
	if (!stralloc_copyb(&temp, addr, len)) {
		close(fd_cdb);
		die_nomem();
	}
	if ((cntrl_ok = cdb_seek(fd_cdb, temp.s, len, &dlen)) == -1) {
		close(fd_cdb);
		strerr_die2sys(111, FATAL, "lseek: ");
	}
	close(fd_cdb);
	return (cntrl_ok ? 1 : 0);
}

int
domain_match(char *fn, stralloc *domain, stralloc *content,
	struct constmap *ptrmap, char **errStr)
{
	int             x, len;
	char           *ptr;

	if (errStr)
		*errStr = 0;
	if (fn && (x = cdb_match(fn, domain->s, domain->len - 1)))
		return (x);
	else
	if (ptrmap && constmap(ptrmap, domain->s, domain->len - 1))
		return 1;
	if (!content)
		return (0);
	for (len = 0, ptr = content->s;len < content->len;) {
		if (!str_diff(domain->s, ptr))
			return (1);
		len += (str_len(ptr) + 1);
		ptr = content->s + len;
	}
	return (0);
}

int
is_tlsadomain(char *domain)
{
	char           *errStr = 0;
	static stralloc _domain = { 0 };

	if (!stralloc_copys(&_domain, domain))
		die_nomem();
	if (!stralloc_0(&_domain))
		die_nomem();
	switch (domain_match(tlsadomainsfn, &_domain, tlsadomainsok ? &tlsadomains : 0, 
			tlsadomainsok ? &maptlsadomains : 0, &errStr))
	{
	case 1:
		return (1);
	case 0:
		return (0);
	default:
		logerr("error in domain match: ");
		logerr(errStr);
		logerrf("\n");
		_exit (111);
	}
	/*- Not reached */
	return (0);
}

int
is_white(char *domain)
{
	char           *errStr = 0;
	static stralloc _domain = { 0 };

	if (!stralloc_copys(&_domain, domain))
		die_nomem();
	if (!stralloc_0(&_domain))
		die_nomem();
	switch (domain_match(whitefn, &_domain, whitelistok ? &whitelist : 0, 
			whitelistok ? &mapwhite : 0, &errStr))
	{
	case 1:
		return (1);
	case 0:
		return (0);
	default:
		logerr("error in domain match: ");
		logerr(errStr);
		logerrf("\n");
		_exit (111);
	}
	/*- Not reached */
	return (0);
}

int
copy_dane(struct danerec *ptr, char *domain, char status)
{
	int             len;

	len = str_len(domain);
	if (!(ptr->domain = (char *) malloc(len + 1)))
		die_nomem();
	if (str_copy(ptr->domain, domain) != len) {
		if (ptr->domain)
			free(ptr->domain);
		if (ptr->data)
			free(ptr->data);
		return (-1);
	}
	ptr->status = status;
	return (0);
}

int
dane_compare(int *key, struct danerec *ptr)
{
	if (*key < ptr->timestamp)
		return (-1);
	else
	if (*key == ptr->timestamp)
		return (0);
	return (1);
}

void
print_record(char *domain, time_t timestamp, char status, int expire_flag)
{
	char            strnum[FMT_ULONG];

	strnum[fmt_ulong(strnum, (unsigned long) timestamp)] = 0;
	out(strnum);
	out(" Domain: ");
	out(domain);
	out(" status=");
	switch(status)
	{
	case RECORD_NEW:
		out("RECORD_NEW");
		break;
	case RECORD_WHITE:
		out("RECORD_WHITE");
		break;
	case RECORD_NOVRFY:
		out("RECORD_NOVERIFY");
		break;
	case RECORD_OK:
		out("RECORD_OK");
		break;
	case RECORD_FAIL:
		out("RECORD_FAIL");
		break;
	case RECORD_OLD:
		out("RECORD_OLD");
		break;
	}
	out(expire_flag ? " expired\n" : "\n");
	flush();
	return;
}

int
create_hash(struct danerec *curr)
{
	struct danerec *ptr;
	ENTRY           e, *ep;
	char            strnum[FMT_ULONG];

	if (!curr) {
		hdestroy();
		h_allocated = hcount = 0;
	}
	if (!(hcount % hash_size)) { /*- first time or hash table full. recreate it */
		if (h_allocated++) {
			/*-
			 * keep count of hash table recreation 
			 * If this happens too often, increase
			 * the hash table size on command line
			 */
			hdestroy();
			hcount = 0;
			curr = 0;
			logerr("WARNING!! recreating hash table, size=");
			strnum[fmt_ulong(strnum, (unsigned long) ((125 * hash_size * h_allocated)/100))] = 0;
			logerr(strnum);
			logerrf("\n");
		} else { /*- first time/expiry */
			if (verbose > 2) {
				out("creating hash table, size=");
				strnum[fmt_ulong(strnum, (unsigned long) ((125 * hash_size * h_allocated)/100))] = 0;
				out(strnum);
				out("\n");
			}
		}
		if (!hcreate((125 * hash_size * h_allocated)/100)) /*- be 25% generous */
			strerr_die2sys(111, FATAL, "unable to create hash table: ");
	}
	/*- 
	 * if curr is null -> add all entries from linked list.
	 * this is done when hash is recreated after
	 * destroying the hash during expiry
	 */
	for (ptr = (curr ? curr : head);ptr;ptr = ptr->next) {
		e.key = ptr->domain;
		if (!(ep = hsearch(e, FIND))) { /*- add entries when hash table is new or after destruction */
			e.data = ptr;
			if (!(ep = hsearch(e, ENTER))) {
				strerr_warn2(WARN, "unable to add to hash table: ", &strerr_sys);
				return (-1);
			} else
				hcount++;
		}
		if (curr) /*- add only one entry */
			break;
	}
	return (0);
}

void
expire_records(time_t cur_time)
{
	struct danerec *ptr;
	time_t          start;

	logerr("expiring records\n");
	/*- find all records that are expired where timestamp < start */
	start = cur_time - timeout;
	for (ptr = head; ptr; ptr = ptr->next) {
		if (ptr->timestamp >= start)
			continue;
		if (verbose > 1)
			print_record(ptr->domain, ptr->timestamp, ptr->status, 1);
		dane_count--;
		if (ptr == head) {
			head = ptr->next;
			head->prev = 0;
		}
		(ptr->prev)->next = ptr->next;
		(ptr->next)->prev = ptr->prev;
		if (ptr->domain)
			free(ptr->domain);
		if (ptr->data)
			free(ptr->data);
		free(ptr);
	}
	logerrf("expired  records\n");
	/*- destroy and recreate the hash */
	if (create_hash(0))
		die_nomem();
	return;
}

int
search_record(char *domain, int fr_int, struct danerec **store)
{
	struct danerec *ptr;
	time_t          cur_time, start;
	ENTRY           e, *ep;

	*store = (struct danerec *) 0;
	if (whitelistok && is_white(domain))
		return (RECORD_WHITE);
	if (!head)
		return (RECORD_NEW);
	cur_time = time(0);
	/* point ptr to the latest unexpired entry */
	if (cur_time - timeout > head->timestamp) {
		/*- records waiting to be expired */
		if (!(cur_time % fr_int)) {
			/*-expire records */
			expire_records(cur_time);
			if (!head)
				return (RECORD_NEW);
			ptr = head; /*- latest unexpired entry */
		} else {
			start = cur_time - timeout;
			for (ptr = head;ptr && ptr->timestamp < start; ptr = ptr->next); /*- latest unexpired entry */
			if (!ptr)
				return (RECORD_NEW);
		}
	} else
		ptr = head; /*- latest unexpired entry */
	e.key = domain;
	if (!(ep = hsearch(e, FIND)))
		return (RECORD_NEW);
	ptr = (struct danerec *) ep->data;
	*store = ptr;
	ptr->attempts++;
	/*- # not older than timeout days */
	if ((ptr->status == RECORD_OK) && (ptr->timestamp > cur_time - timeout))
		return (RECORD_OK);
	else
		return (RECORD_OLD);
}

/*
 * add entry to link list
 * added to hash list if not found in hash list
 */
struct danerec *
add_record(char *domain, char status, struct danerec **dnrec)
{
	struct danerec *ptr;

	if (!head) {
		if (!(ptr = (struct danerec *) malloc(sizeof(struct danerec))))
			die_nomem();
		ptr->prev = ptr->next = 0;
		head = tail = ptr;
	} else {
		if (!(ptr = (struct danerec *) malloc(sizeof(struct danerec))))
			die_nomem();
		ptr->prev = tail;
		tail->next = ptr;
		tail = ptr;
	}
	*dnrec = ptr;
	dane_count++;
	ptr->domain = (char *) 0;
	ptr->data = (char *) 0;
	ptr->timestamp = time(0);
	ptr->attempts = 1;
	ptr->status = RECORD_NEW;
	if (copy_dane(ptr, domain, status)) {
		free(ptr);
		return ((struct danerec *) 0);
	}
	return (ptr);
}

int
write_file(int fd, char *arg, int len)
{
	int             i;

	if ((i = write(fd, arg, len)) == -1) {
		strerr_warn2(WARN, "write failed: ", &strerr_sys);
		return (-1);
	}
	return (i);
}

int
write_0(int fd)
{
	int             i;

	if ((i = write(fd, "\0", 1)) == -1)
		strerr_warn2(WARN, "write failed: ", &strerr_sys);
	return (i);
}

/*
 * save records on shutdown, in a context file so that they
 * can be reloaded on restart
 */
void
save_context()
{
	struct danerec *ptr;
	int             context_fd;
	char            timestamp[FMT_ULONG], datalen[FMT_ULONG];

	if (!dane_count)
		return;
	if (verbose > 2)
		out("saving context file\n");
	if ((context_fd = open(context_file.s, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to open file: ", context_file.s, ": ");
	for (ptr = head;ptr;ptr = ptr->next) {
		if (ptr->timestamp < time(0) - timeout) /*- don't save expired records */
			continue;
		timestamp[fmt_ulong(timestamp, ptr->timestamp)] = 0;
		datalen[fmt_ulong(datalen, ptr->datalen)] = 0;
		if (write_file(context_fd, ptr->domain, str_len(ptr->domain)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, (char *) &ptr->status, 1) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, timestamp, str_len(timestamp)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, datalen, str_len(datalen)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, ptr->data, ptr->datalen) == -1
			|| write_file(context_fd, "\n", 1) == -1)
		{
			break;
		}
		if (!ptr->next)
			break;
	} /*- for (ptr = head;ptr;ptr = ptr->next) */
	close(context_fd);
	if (verbose > 2)
		out("saved  context file\n");
	return;
}

stralloc        context = { 0 };
/*-
 * load records on startup from a context file which
 * was saved on shutdown (SIGTERM)
 */
void
load_context()
{
	int             fd, match;
	time_t          timestamp, cur_time;
	substdio        ss;
	char            inbuf[2048];
	char           *cptr, *domain;
	char            dnstate;
	struct danerec *ptr;

	if (verbose > 2) {
		out("loading context\n");
		flush();
	}
	if ((fd = open(context_file.s, O_RDONLY)) == -1) {
		if (errno == error_noent)
			return;
		die_control(context_file.s);
	}
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
	cur_time = time(0);
	for (;;) {
		if (getln(&ss, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len) {
			close(fd);
			return;
		}
		/*- process records */
		if (!stralloc_0(&line))
			die_nomem();
		cptr = line.s;
		domain = cptr;
		cptr += str_len(cptr) + 1;
		dnstate = *cptr;
		cptr += 2;
		scan_ulong(cptr, (unsigned long *) &timestamp);
		if (verbose > 1) {
			if (timestamp < cur_time - timeout) { /*- expired records */
				print_record(domain, timestamp, dnstate, 1);
				continue;
			} else
				print_record(domain, timestamp, dnstate, 0);
		}
		if (!head) {
			if (!(ptr = (struct danerec *) malloc(sizeof(struct danerec))))
				die_nomem();
			ptr->prev = 0;
			ptr->next = 0;
			head = tail = ptr;
		} else {
			if (!(ptr = (struct danerec *) malloc(sizeof(struct danerec))))
				die_nomem();
			ptr->prev = tail;
			tail->next = ptr;
			tail = ptr;
		}
		dane_count++;
		ptr->timestamp = timestamp;
		cptr += (str_len(cptr) + 1);
		scan_int(cptr, &ptr->datalen);
		cptr += (str_len(cptr) + 1);
		if (!(ptr->data = (char *) malloc(ptr->datalen)))
			die_nomem();
		byte_copy(ptr->data, ptr->datalen, cptr);
		ptr->status = dnstate;
		if (copy_dane(ptr, domain, dnstate)) {
			free(ptr);
			return;
		}
		if (create_hash(ptr)) /*- load previous context into hash table */
			die_nomem();
		if (!match) {
			close(fd);
			return;
		}
	} /*- for (;;) { */
	close(fd);
	return;
}

/*
 * Reload whitelist on sighup
 */
void
sighup()
{
	sig_block(SIGHUP);
	if (whitelistok) {
		constmap_free(&mapwhite);
		whitelist_init(whitefn);
	}
	sig_unblock(SIGHUP);
}

/*-
 * save context file on SIGUSR1
 */
void
sigusr1()
{
	sig_block(SIGUSR1);
	save_context();
	flush();
	sig_unblock(SIGUSR1);
	return;
}

void
ready()
{
	char            strnum[FMT_ULONG];
	out("Ready for connections, dane_count=");
	strnum[fmt_ulong(strnum, (unsigned long) dane_count)] = 0;
	out(strnum);
	out(", hcount=");
	strnum[fmt_ulong(strnum, (unsigned long) hcount)] = 0;
	out(strnum);
	out(", hash_size=");
	strnum[fmt_ulong(strnum, (unsigned long) ((125 * hash_size * h_allocated)/100))] = 0;
	out(strnum);
	out("\n");
	flush();
}

/*-
 * expire records on SIGUSR2
 */
void
sigusr2()
{
	struct danerec *ptr;
	int             i;
	char            strnum[FMT_ULONG];

	sig_block(SIGUSR2);
	expire_records(time(0));
	if (verbose > 2) {
		for (i = 1, ptr = head; ptr; ptr = ptr->next, i++) {
			strnum[fmt_ulong(strnum, (unsigned long) i)] = 0;
			out(strnum);
			out(" ");
			print_record(ptr->domain, ptr->timestamp, ptr->status, 0);
		}
	}
	sig_unblock(SIGUSR2);
	ready();
	return;
}

void
sigterm()
{
	sig_block(SIGTERM);
	logerrf("ARGH!! Committing suicide on SIGTERM\n");
	save_context();
	flush();
	_exit (0);
}

char           *
print_status(char status)
{
	switch (status)
	{
	case RECORD_NEW:
		return ("RECORD NEW");
	case RECORD_WHITE:
		return ("RECORD WHITE");
	case RECORD_NOVRFY:
		return ("RECORD NOVERIFY");
	case RECORD_OK:
		return ("RECORD OK");
	case RECORD_OLD:
		return ("RECORD OLD");
	case RECORD_FAIL:
		return ("RECORD FAIL");
	}
	return ("RECORD UNKNOWN");
}

#define HUGESMTPTEXT  5000
GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, i, n, x, 10, saa_readyplus)

stralloc        saciphers = { 0 }, tlsFilename = { 0 }, clientcert = { 0 };
stralloc        smtptext = { 0 };
stralloc        host = { 0 };
stralloc        rhost = { 0 }; /*- host ip to which qmail-remote ultimately connects */
stralloc        sauninit = { 0 };
stralloc        helohost = { 0 };
SSL_CTX        *ctx;
char           *partner_fqdn = 0;
struct ip_mx    partner;
union v46addr   outip;
int             timeoutconnect = 60;
int             ssltimeout = 1200;
saa             ehlokw = { 0 };	/*- list of EHLO keywords and parameters */
int             maxehlokwlen = 0;
int             smtpfd;
const char     *ssl_err_str = 0;

void
outhost()
{
	char            x[IPFMT];
	unsigned int    len;

	switch (partner.af)
	{
#ifdef IPV6
	case AF_INET6:
		len = ip6_fmt(x, &partner.addr.ip6);
		break;
#endif
	case AF_INET:
		len = ip4_fmt(x, &partner.addr.ip);
		break;
	default:
		len = ip4_fmt(x, &partner.addr.ip);
		break;
	}
	if (!stralloc_copyb(&rhost, x, len))
		die_nomem();
	if (!stralloc_0(&rhost))
		die_nomem();
	if (substdio_put(subfderr, x, len) == -1) {
		strerr_warn2(FATAL, "write: ", &strerr_sys);
		die(111);
	}
}

void
dropped()
{
	logerr("Connected to ");
	outhost();
	logerr(" but connection died");
	if (ssl_err_str) {
		logerr(": ");
		logerr((char *) ssl_err_str);
	}
	logerrf("\n");
	die (111);
}

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

	if (ssl) {
		if ((r = ssl_timeoutread(ssltimeout, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = ssl_error_str();
	} else
		r = timeoutread(ssltimeout, smtpfd, buf, len);
	if (r <= 0)
		dropped();
	return r;
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

	if (ssl) {
		if ((r = ssl_timeoutwrite(ssltimeout, smtpfd, smtpfd, ssl, buf, len)) < 0)
			ssl_err_str = ssl_error_str();
	} else
		r = timeoutwrite(ssltimeout, smtpfd, buf, len);
	if (r <= 0)
		dropped();
	return r;
}

char            inbuf[1500];
char            smtptobuf[1500];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        smtpto = SUBSTDIO_FDBUF(safewrite, -1, smtptobuf, sizeof smtptobuf);
char            smtpfrombuf[128];
substdio        smtpfrom = SUBSTDIO_FDBUF(saferead, -1, smtpfrombuf, sizeof smtpfrombuf);

void
perm_dns()
{
	logerr("Sorry, I couldn't find any host named ");
	logerr(partner_fqdn);
	logerrf("\n");
	die (5);
}

void
temp_dns()
{
	logerr("no host by that name ");
	logerr(partner_fqdn);
	logerrf("\n");
	die (6);
}

void
temp_oserr()
{
	logerrf("System resources temporarily unavailable\n");
	die (111);
}

int
get_tlsa_rr(char *domain, int mxhost, int port)
{
	int             j;
	unsigned long   r;
	char            strnum[FMT_ULONG];

	if (!stralloc_copys(&sa, domain))
		die_nomem();
	if (mxhost) {
		if (!stralloc_copyb(&sa, "_", 1))
			die_nomem();
		if (!stralloc_catb(&sa, strnum, fmt_uint(strnum, port)))
			die_nomem();
		if (!stralloc_catb(&sa, "._tcp.", 6))
			die_nomem();
		if (!stralloc_cats(&sa, domain))
			die_nomem();
		dns_init(0);
		r = dns_tlsarr(&ta, &sa);
		switch (r)
		{
		case DNS_HARD:
			perm_dns();
		case DNS_SOFT:
			temp_dns();
		case DNS_MEM:
			die_nomem();
		}
	} else {
		dns_init(0);
		r = now() + getpid();
		r = dns_mxip(&ia, &sa, r);
		switch (r)
		{
		case DNS_HARD:
			perm_dns();
		case DNS_SOFT:
			temp_dns();
		case DNS_MEM:
			die_nomem();
		}
		for (j = 0; j < ia.len; ++j) {
			if (j && !str_diff(ia.ix[j].fqdn, ia.ix[j - 1].fqdn))
				continue;
			if (!stralloc_copyb(&sa, "_", 1))
				die_nomem();
			if (!stralloc_catb(&sa, strnum, fmt_uint(strnum, port)))
				die_nomem();
			if (!stralloc_catb(&sa, "._tcp.", 6))
				die_nomem();
			if (!stralloc_cats(&sa, ia.ix[j].fqdn))
				die_nomem();
			r = dns_tlsarr(&ta, &sa);
			switch (r)
			{
			case DNS_HARD:
				perm_dns();
			case DNS_SOFT:
				temp_dns();
			case DNS_MEM:
				die_nomem();
			}
		} /*- for (j = 0; j < ia.len; ++j) */
	}
	return (0);
}

void
get(char *ch)
{
	substdio_get(&smtpfrom, ch, 1);
	if (*ch != '\r') {
		if (smtptext.len < HUGESMTPTEXT) {
			if (!stralloc_append(&smtptext, ch))
				die_nomem();
		}
	}
}

unsigned long
smtpcode()
{
	unsigned char   ch;
	unsigned long   code;

	if (!stralloc_copys(&smtptext, ""))
		die_nomem();
	get((char *) &ch);
	code = ch - '0';
	get((char *) &ch);
	code = code * 10 + (ch - '0');
	get((char *) &ch);
	code = code * 10 + (ch - '0');
	for (;;) {
		get((char *) &ch);
		if (ch != '-')
			break;
		while (ch != '\n')
			get((char *) &ch);
		get((char *) &ch);
		get((char *) &ch);
		get((char *) &ch);
	}
	while (ch != '\n')
		get((char *) &ch);
	return code;
}

int
timeoutconn46(int fd, struct ip_mx *ix, union v46addr *ip, int port, int timeout)
{
	switch (ix->af)
	{
#ifdef IPV6
	case AF_INET6:
		return timeoutconn6(fd, &ix->addr.ip6, ip, port, timeout);
		break;
#endif
	case AF_INET:
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
		break;
	default:
		return timeoutconn4(fd, &ix->addr.ip, ip, port, timeout);
	}
}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define SSL_ST_BEFORE 0x4000
#endif
unsigned long
ehlo()
{
	stralloc       *sa;
	char           *s, *e, *p;
	unsigned long   code;

	if (ehlokw.len > maxehlokwlen)
		maxehlokwlen = ehlokw.len;
	ehlokw.len = 0;
	if (substdio_puts(&smtpto, "EHLO ") == -1)
		strerr_die2sys(111, FATAL, "write: ");
	else
	if (substdio_put(&smtpto, helohost.s, helohost.len) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	else
	if (substdio_puts(&smtpto, "\r\n") == -1)
		strerr_die2sys(111, FATAL, "write: ");
	else
	if (substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if ((code = smtpcode()) != 250)
		return code;
	s = smtptext.s;
	while (*s++ != '\n');		/*- skip the first line: contains the domain */
	e = smtptext.s + smtptext.len - 6;	/*- 250-?\n */
	while (s <= e) {
		int             wasspace = 0;

		if (!saa_readyplus(&ehlokw, 1))
			die_nomem();
		sa = ehlokw.sa + ehlokw.len++;
		if (ehlokw.len > maxehlokwlen)
			*sa = sauninit;
		else
			sa->len = 0;

		/*- smtptext is known to end in a '\n' */
		for (p = (s += 4);; ++p) {
			if (*p == '\n' || *p == ' ' || *p == '\t') {
				if (!wasspace)
					if (!stralloc_catb(sa, s, p - s) || !stralloc_0(sa))
						die_nomem();
				if (*p == '\n')
					break;
				wasspace = 1;
			} else
			if (wasspace == 1) {
				wasspace = 0;
				s = p;
			}
		}
		s = ++p;

		/*
		 * keyword should consist of alpha-num and '-'
		 * broken AUTH might use '=' instead of space
		 */
		for (p = sa->s; *p; ++p) {
			if (*p == '=') {
				*p = 0;
				break;
			}
		}
	}
	return 250;
}

void
outsmtptext()
{
	int             i;

	if (smtptext.s && smtptext.len) {
		out("Remote host said: ");
		for (i = 0; i < smtptext.len; ++i) {
			if (!smtptext.s[i])
				smtptext.s[i] = '?';
		}
		if (substdio_put(subfderr, smtptext.s, smtptext.len) == -1) {
			strerr_warn2(FATAL, "write: ", &strerr_sys);
			die (111);
		}
		smtptext.len = 0;
	}
}

/*
 * die =   0 - success
 * die =  -1 - failure system error
 * die =   1 - failure non-system error
 *
 * code =  xxx - smtp code
 * code =    0 - temporary failure
 */
void
quit(char *prepend, char *append, int e)
{
	substdio_putsflush(&smtpto, "QUIT\r\n");
	/*- waiting for remote side is just too ridiculous */
	logerr(prepend);
	outhost();
	logerr(append);
	logerr(".\n");
	outsmtptext();
	substdio_flush(subfderr);
	die (e);
}

void
tls_quit(const char *s1, char *s2, char *s3, char *s4, stralloc *sa)
{
	char            ch;
	int             i, state;

	logerr((char *) s1);
	if (s2)
		logerr((char *) s2);
	if (s3)
		logerr((char *) s3);
	if (s4)
		logerr((char *) s4);
	if (sa && sa->len) {
		for (i = 0; i < sa->len; ++i) {
			ch = sa->s[i];
			if (ch < 33)
				ch = '?';
			if (ch > 126)
				ch = '?';
			if (substdio_put(subfdout, &ch, 1) == -1)
				die (0);
		}
	}

	/*- 
	 * shouldn't talk to the client unless in an appropriate state 
	 * https://mta.openssl.org/pipermail/openssl-commits/2015-October/002060.html
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	state = ssl ? SSL_get_state(ssl) : SSL_ST_BEFORE;
	if ((state & TLS_ST_OK) || (!smtps && (state & SSL_ST_BEFORE)))
		substdio_putsflush(&smtpto, "QUIT\r\n");
#else
	state = ssl ? ssl->state : SSL_ST_BEFORE;
	if ((state & SSL_ST_OK) || (!smtps && (state & SSL_ST_BEFORE)))
		substdio_putsflush(&smtpto, "QUIT\r\n");
#endif
	logerr(ssl ? "; connected to " : "; connecting to ");
	outhost();
	logerrf("\n");
	if (env_get("DEBUG") && ssl) {
		X509           *peercert;

		logerr("STARTTLS proto=");
		logerr((char *) SSL_get_version(ssl));
		logerr("; cipher=");
		logerr((char *) SSL_get_cipher(ssl));

		/*- we want certificate details */
		if ((peercert = SSL_get_peer_certificate(ssl))) {
			char           *str;

			str = X509_NAME_oneline(X509_get_subject_name(peercert), NULL, 0);
			logerr("; subject=");
			logerr(str);
			OPENSSL_free(str);

			str = X509_NAME_oneline(X509_get_issuer_name(peercert), NULL, 0);
			logerr("; issuer=");
			logerr(str);
			OPENSSL_free(str);

			X509_free(peercert);
		}
		logerrf(";\n");
	}
	die (111);
}

int
match_partner(char *s, int len)
{
	if (!case_diffb(partner_fqdn, len, (char *) s) && !partner_fqdn[len])
		return 1;
	/*- we also match if the name is *.domainname */
	if (*s == '*') {
		const char     *domain = partner_fqdn + str_chr(partner_fqdn, '.');
		if (!case_diffb((char *) domain, --len, (char *) ++s) && !domain[len])
			return 1;
	}
	return 0;
}

/*-
 * don't want to fail handshake if certificate can't be verified
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX *ctx)
{
	return 1;
}

void
do_pkix(char *servercert)
{
	X509           *peercert;
	STACK_OF(GENERAL_NAME) *gens;
	int             r, i;
	char           *t;

	/*- PKIX */
	if ((r = SSL_get_verify_result(ssl)) != X509_V_OK) {
		t = (char *) X509_verify_cert_error_string(r);
		tls_quit("TLS unable to verify server with ", servercert, ": ", t, 0);
	}
	if (!(peercert = SSL_get_peer_certificate(ssl)))
		tls_quit("TLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);

	/*-
	 * RFC 2595 section 2.4: find a matching name
	 * first find a match among alternative names
	 */
	if ((gens = X509_get_ext_d2i(peercert, NID_subject_alt_name, 0, 0))) {
		for (i = 0, r = sk_GENERAL_NAME_num(gens); i < r; ++i) {
			const GENERAL_NAME *gn = sk_GENERAL_NAME_value(gens, i);
			if (gn->type == GEN_DNS)
				if (match_partner((char *) gn->d.ia5->data, gn->d.ia5->length))
					break;
		}
		sk_GENERAL_NAME_pop_free(gens, GENERAL_NAME_free);
	}

	/*- no alternative name matched, look up commonName */
	if (!gens || i >= r) {
		stralloc        peer = { 0 };
		X509_NAME      *subj = X509_get_subject_name(peercert);
		if ((i = X509_NAME_get_index_by_NID(subj, NID_commonName, -1)) >= 0) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			X509_NAME_ENTRY *t;
			t = X509_NAME_get_entry(subj, i);
			ASN1_STRING    *s = X509_NAME_ENTRY_get_data(t);
#else
			const ASN1_STRING *s = X509_NAME_get_entry(subj, i)->value;
#endif
			if (s) {
				peer.len = s->length;
				peer.s = (char *) s->data;
			}
		}
		if (peer.len <= 0)
			tls_quit("TLS unable to verify server ", partner_fqdn, ": certificate contains no valid commonName", 0, 0);
		if (!match_partner((char *) peer.s, peer.len))
			tls_quit("TLS unable to verify server ", partner_fqdn, ": received certificate for ", 0, &peer);
	}
	X509_free(peercert);
	return;
}

/*
 * 1. returns 0 --> fallback to non-tls
 *    if certs do not exist
 *    host is in notlshosts
 *    smtps == 0 and tls session cannot be initated
 * 2. returns 1 if tls session was initated
 * 3. exits on error, if smtps == 1 and tls session did not succeed
 */
int
tls_init(int pkix, int *needtlsauth, char **scert)
{
	int             code, i = 0, _needtlsauth = 0;
	static char     ssl_initialized;
	const char     *ciphers = 0;
	char           *t, *servercert = 0;
	static SSL     *myssl;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	stralloc        ssl_option = { 0 };
	int             method = 4;	/*- (1..2 unused) [1..3] = ssl[1..3], 4 = tls1, 5=tls1.1, 6=tls1.2 */
#endif
	int             method_fail = 1;

	if (needtlsauth)
		*needtlsauth = 0;
	if (scert)
		*scert = 0;
	/*- 
	 * tls_init() gets called in smtp()
	 * if smtp() returns for trying next mx
	 * we need to re-initialize
	 */
	if (ctx) {
		SSL_CTX_free(ctx);
		ctx = (SSL_CTX *) 0;
	}
	if (myssl) {
		SSL_free(myssl);
		ssl = myssl = (SSL *) 0;
	}
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (!stralloc_copys(&tlsFilename, controldir))
		die_nomem();
	if (!stralloc_catb(&tlsFilename, "/tlsclientmethod", 16))
		die_nomem();
	if (!stralloc_0(&tlsFilename))
		die_nomem();
	if (control_rldef(&ssl_option, tlsFilename.s, 0, "TLSv1_2") != 1)
		die_control(tlsFilename.s);
	if (!stralloc_0(&ssl_option))
		die_nomem();
	if (str_equal(ssl_option.s, "SSLv23"))
		method = 2;
	else
	if (str_equal(ssl_option.s, "SSLv3"))
		method = 3;
	else
	if (str_equal(ssl_option.s, "TLSv1"))
		method = 4;
	else
	if (str_equal(ssl_option.s, "TLSv1_1"))
		method = 5;
	else
	if (str_equal(ssl_option.s, "TLSv1_2"))
		method = 6;
#endif
	if (!certdir && !(certdir = env_get("CERTDIR")))
		certdir = auto_control;
	if (!stralloc_copys(&clientcert, certdir))
		die_nomem();
	if (!stralloc_catb(&clientcert, "/clientcert.pem", 15))
		die_nomem();
	if (!stralloc_0(&clientcert))
		die_nomem();
	if (access(clientcert.s, F_OK))
		return (0);
	if (partner_fqdn) {
		struct stat     st;
		if (!stralloc_copys(&tlsFilename, certdir))
			die_nomem();
		if (!stralloc_catb(&tlsFilename, "/tlshosts/", 10))
			die_nomem();
		if (!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn)))
			die_nomem();
		if (!stralloc_catb(&tlsFilename, ".pem", 4))
			die_nomem();
		if (!stralloc_0(&tlsFilename))
			die_nomem();
		if (stat(tlsFilename.s, &st)) {
			_needtlsauth = 0;
			if (needtlsauth)
				*needtlsauth = 0;
			if (!stralloc_copys(&tlsFilename, certdir))
				die_nomem();
			if (!stralloc_catb(&tlsFilename, "/notlshosts/", 12))
				die_nomem();
			if (!stralloc_catb(&tlsFilename, partner_fqdn, str_len(partner_fqdn) + 1))
				die_nomem();
			if (!stralloc_0(&tlsFilename))
				die_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
			if (!stralloc_copys(&tlsFilename, certdir))
				die_nomem();
			if (!stralloc_catb(&tlsFilename, "/tlshosts/exhaustivelist", 24))
				die_nomem();
			if (!stralloc_0(&tlsFilename))
				die_nomem();
			if (!stat(tlsFilename.s, &st))
				return (0);
		} else {
			*scert = servercert = tlsFilename.s;
			_needtlsauth = 1;
			if (needtlsauth)
				*needtlsauth = 1;
		}
	}

	if (!smtps) {
		stralloc       *sa = ehlokw.sa;
		unsigned int    len = ehlokw.len;

		/*- look for STARTTLS among EHLO keywords */
		for (; len && case_diffs(sa->s, "STARTTLS"); ++sa, --len);
		if (!len) {
			if (!_needtlsauth)
				return (0);
			tls_quit("No TLS achieved while", tlsFilename.s, " exists", 0, 0);
		}
	}
	if (!ssl_initialized++)
		SSL_library_init();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (method == 2 && (ctx = SSL_CTX_new(SSLv23_client_method())))
		method_fail = 0;
	else
	if (method == 3 && (ctx = SSL_CTX_new(SSLv3_client_method())))
		method_fail = 0;
#if defined(TLSV1_CLIENT_METHOD) || defined(TLS1_VERSION)
	else
	if (method == 4 && (ctx = SSL_CTX_new(TLSv1_client_method())))
		method_fail = 0;
#endif
#if defined(TLSV1_1_CLIENT_METHOD) || defined(TLS1_1_VERSION)
	else
	if (method == 5 && (ctx = SSL_CTX_new(TLSv1_1_client_method())))
		method_fail = 0;
#endif
#if defined(TLSV1_2_CLIENT_METHOD) || defined(TLS1_2_VERSION)
	else
	if (method == 6 && (ctx = SSL_CTX_new(TLSv1_2_client_method())))
		method_fail = 0;
#endif
#else /*- #if OPENSSL_VERSION_NUMBER < 0x10100000L */
	if ((ctx = SSL_CTX_new(TLS_client_method())))
		method_fail = 0;
	/*- SSL_OP_NO_SSLv3, SSL_OP_NO_TLSv1, SSL_OP_NO_TLSv1_1 and SSL_OP_NO_TLSv1_2 */
	/*- POODLE Vulnerability */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
#endif /*- #if OPENSSL_VERSION_NUMBER < 0x10100000L */
	if (method_fail) {
		if (!smtps && !_needtlsauth) {
			SSL_CTX_free(ctx);
			return (0);
		}
		t = (char *) ssl_error();
		SSL_CTX_free(ctx);
		switch (method_fail)
		{
		case 2:
			tls_quit("TLS error initializing SSLv23 ctx: ", t, 0, 0, 0);
			break;
		case 3:
			tls_quit("TLS error initializing SSLv3 ctx: ", t, 0, 0, 0);
			break;
		case 4:
			tls_quit("TLS error initializing TLSv1 ctx: ", t, 0, 0, 0);
			break;
		case 5:
			tls_quit("TLS error initializing TLSv1_1 ctx: ", t, 0, 0, 0);
			break;
		case 6:
			tls_quit("TLS error initializing TLSv1_2 ctx: ", t, 0, 0, 0);
			break;
		}
	}

	if (_needtlsauth) {
		if (!SSL_CTX_load_verify_locations(ctx, servercert, NULL)) {
			t = (char *) ssl_error();
			SSL_CTX_free(ctx);
			tls_quit("TLS unable to load ", servercert, ": ", t, 0);
		}
		/*- set the callback here; SSL_set_verify didn't work before 0.9.6c */
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
	}

	/*- let the other side complain if it needs a cert and we don't have one */
	if (SSL_CTX_use_certificate_chain_file(ctx, clientcert.s))
		SSL_CTX_use_RSAPrivateKey_file(ctx, clientcert.s, SSL_FILETYPE_PEM);

	if (!(myssl = SSL_new(ctx))) {
		t = (char *) ssl_error();
		if (!smtps && !_needtlsauth) {
			SSL_CTX_free(ctx);
			return (0);
		}
		SSL_CTX_free(ctx);
		tls_quit("TLS error initializing ssl: ", t, 0, 0, 0);
	} else
		SSL_CTX_free(ctx);

	if (!smtps)
		substdio_putsflush(&smtpto, "STARTTLS\r\n");

	/*- while the server is preparing a response, do something else */
	if (!ciphers) {
		if (control_readfile(&saciphers, "tlsclientciphers", 0) == -1) {
			while (SSL_shutdown(myssl) == 0);
			SSL_free(myssl);
			die_control("tlsclientciphers");
		}
		if (saciphers.len) {
			for (i = 0; i < saciphers.len - 1; ++i)
				if (!saciphers.s[i])
					saciphers.s[i] = ':';
			ciphers = saciphers.s;
		} else
			ciphers = "DEFAULT";
	}
	SSL_set_cipher_list(myssl, ciphers);

	/*- SSL_set_options(myssl, SSL_OP_NO_TLSv1); */
	SSL_set_fd(myssl, smtpfd);

	/*- read the response to STARTTLS */
	if (!smtps) {
		if (smtpcode() != 220) {
			while (SSL_shutdown(myssl) == 0);
			SSL_free(myssl);
			ssl = myssl = (SSL *) 0;
			if (!_needtlsauth)
				return (0);
			tls_quit("STARTTLS rejected while ", tlsFilename.s, " exists", 0, 0);
		}
	}
	ssl = myssl;
	if (ssl_timeoutconn(ssltimeout, smtpfd, smtpfd, ssl) <= 0) {
		t = (char *) ssl_error_str();
		tls_quit("TLS connect failed: ", t, 0, 0, 0);
	}
	if (smtps && (code = smtpcode()) != 220) /*- 220 ready for tls */
		quit("TLS Connected to ", " but greeting failed", code);
	if (pkix && _needtlsauth)
		do_pkix(servercert);
	return (1);
}

/*-
 * USAGE
 * 0, 1, 2, 3, 255
 * 255 PrivCert
 * 
 * SELECTOR
 * --------
 * 0, 1, 255
 * 255 PrivSel
 * 
 * Matching Type
 * -------------
 * 0, 1, 2, 255
 * 255 PrivMatch
 *
 * Return Value
 * 0   - match target certificate & payload data
 * 1,2 - successful match
 */
stralloc        certData = { 0 };
int
tlsa_vrfy_records(char *certDataField, int usage, int selector, int match_type, char **err_str )
{
	EVP_MD_CTX     *mdctx;
	const EVP_MD   *md = 0;
	BIO            *membio = 0;
	EVP_PKEY       *pkey = 0;
	X509           *xs = 0;
	STACK_OF(X509) *sk;
	static char     errbuf[256];
	char            buffer[512], hex[2];
	unsigned char   md_value[EVP_MAX_MD_SIZE];
	unsigned char  *ptr;
	int             i, len;
	unsigned int    md_len;

	if (!ssl)
		return (-1);
	switch (usage)
	{
	/*- Trust anchor */
	case 0: /*- PKIX-TA(0) maps to DANE-TA(2) */
		/*- flow through */
	case 2: /*- DANE-TA(2) */
		usage = 2;
		break;
	case 1: /*- PKIX-EE(1) maps to DANE-EE(3) */
		/*- flow through */
	case 3: /*- DANE-EE(3) */
		usage = 3;
		break;
	default:
		return (-2);
	}
	switch (selector) /*- match full certificate or subjectPublicKeyInfo */
	{
	case 0: /*- Cert(0) - match full certificate   data/SHA256fingerprint/SHA512fingerprint */
		break;
	case 1: /*- SPKI(1) - match subject public key data/SHA256fingerprint/SHA512fingerprint  */
		break;
	default:
		return (-2);
	}
	switch (match_type) /*- sha256, sha512 */
	{
	case 0: /*- Full(0) - match full cert data or subjectPublicKeyInfo data */
		break;
	case 1: /*- SHA256(1) fingerprint - servers should publish this mandatorily */
		md = EVP_get_digestbyname("sha256");
		break;
	case 2: /*- SHA512(2)  fingerprint - servers should not exclusively publish this */
		md = EVP_get_digestbyname("sha512");
		break;
	default:
		return (-2);
	}
	/*- SSL_ctrl(ssl, SSL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, servername); -*/
	if (!(sk =  SSL_get_peer_cert_chain(ssl)))
		tls_quit("TLS unable to verify server ", partner_fqdn, ": no certificate provided", 0, 0);
	/*- 
	 * the server certificate is generally presented
	 * as the first certificate in the stack along with
	 * the remaining chain.
	 * last certificate in the list is a trust anchor
	 * 5.2.2.  Trust Anchor Digests and Server Certificate Chain
	 * https://tools.ietf.org/html/rfc7671
	 *
	 * for Usage 2, check the last  certificate - sk_X509_value(sk, sk_509_num(sk) - 1)
	 * for Usage 3, check the first certificate - sk_X509_value(sk, 0)
	 */
	/*- for (i = (usage == 2 ? 1 : 0); i < (usage == 2 ? sk_X509_num(sk) : 1); i++) -*/
	i = (usage == 2 ? sk_X509_num(sk) - 1 : 0);
	xs = sk_X509_value(sk, i);
	/*- 
	 * DANE Validation 
	 * case 1 - match full certificate data
	 * case 2 - match full subjectPublicKeyInfo data
	 * case 3 - match  SHA512/SHA256 fingerprint of full certificate
	 * case 4 - match  SHA512/SHA256 fingerprint of subjectPublicKeyInfo
	 */
	if (match_type == 0 && selector == 0) { /*- match full certificate data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_X509(membio, xs)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to create write to membio: ", *err_str, 0, 0, 0);
		}
		for (certData.len = 0;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN CERTIFICATE-----"))
				continue;
			if (str_start(buffer, "-----END CERTIFICATE-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = 0;
			if (!stralloc_cats(&certData, buffer))
				die_nomem();
		}
		if (!stralloc_0(&certData))
			die_nomem();
		certData.len--;
		BIO_free_all(membio);
		if (!str_diffn(certData.s, certDataField, certData.len))
			return (0);
		return (1);
	}
	if (match_type == 0 && selector == 1) { /*- match full subjectPublicKeyInfo data */
		if (!(membio = BIO_new(BIO_s_mem()))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to create membio: ", *err_str, 0, 0, 0);
		}
		if (!(pkey = X509_get_pubkey(xs))) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to get pubkey: ", *err_str, 0, 0, 0);
		}
		if (!PEM_write_bio_PUBKEY(membio, pkey)) {
			*err_str = ERR_error_string(ERR_get_error(), errbuf);
			tls_quit("DANE unable to write pubkey to membio: ", *err_str, 0, 0, 0);
		}
		for (;;) {
			if (!BIO_gets(membio, buffer, sizeof(buffer) - 2))
				break;
			if (str_start(buffer, "-----BEGIN PUBLIC KEY-----"))
				continue;
			if (str_start(buffer, "-----END PUBLIC KEY-----"))
				continue;
			buffer[i = str_chr(buffer, '\n')] = 0;
			if (!stralloc_cats(&certData, buffer))
				die_nomem();
		}
		if (!stralloc_0(&certData))
			die_nomem();
		certData.len--;
		BIO_free_all(membio);
		if (!str_diffn(certData.s, certDataField, certData.len))
			return (0);
		return (1);
	}
	/*- SHA512/SHA256 fingerprint of full certificate */
	if ((match_type == 2 || match_type == 1) && selector == 0) {
		if (!X509_digest(xs, md, md_value, &md_len))
			tls_quit("TLS Unable to get peer cerficatte digest", 0, 0, 0, 0);
		for (i = hexstring.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hexstring, hex, 2))
				die_nomem();
		}
		if (!str_diffn(certDataField, hexstring.s, hexstring.len))
			return (0);
		return (1);
	}
	/*- SHA512/SHA256 fingerprint of subjectPublicKeyInfo */
	if ((match_type == 2 || match_type == 1) && selector == 1) {
		unsigned char  *tmpbuf = (unsigned char *) 0;
		if (!(mdctx = EVP_MD_CTX_new()))
			die_nomem();
		if (!(pkey = X509_get_pubkey(xs)))
			tls_quit("TLS Unable to get public key", 0, 0, 0, 0);
		if (!EVP_DigestInit_ex(mdctx, md, NULL))
			tls_quit("TLS Unable to initialize EVP Digest", 0, 0, 0, 0);
		if ((len = i2d_PUBKEY(pkey, NULL)) < 0)
			tls_quit("TLS failed to encode public key", 0, 0, 0, 0);
		if (!(tmpbuf = (unsigned char *) OPENSSL_malloc(len * sizeof(char))))
			die_nomem();
		ptr = tmpbuf;
		if ((len = i2d_PUBKEY(pkey, &ptr)) < 0)
			tls_quit("TLS failed to encode public key", 0, 0, 0, 0);
		if (!EVP_DigestUpdate(mdctx, tmpbuf, len))
			tls_quit("TLS Unable to update EVP Digest", 0, 0, 0, 0);
		OPENSSL_free(tmpbuf);
		if (!EVP_DigestFinal_ex(mdctx, md_value, &md_len))
			tls_quit("TLS Unable to compute EVP Digest", 0, 0, 0, 0);
		for (i = hexstring.len = 0; i < md_len; i++) {
			fmt_hexbyte(hex, (md_value + i)[0]);
			if (!stralloc_catb(&hexstring, hex, 2))
				die_nomem();
		}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		if (!str_diffn(certDataField, hexstring.s, hexstring.len))
			return (0);
		return (1);
	}
	return (1);
}

int
do_work(char *host, int port)
{
	static ipalloc  ip = { 0 };
    char           *err_str = 0, *servercert = 0;
	char            strnum[FMT_ULONG], hex[2];
	int             i, j, tlsa_status, authfullMatch, authsha256, authsha512,
					match0Or512, needtlsauth, usage;
	unsigned long   code;
    tlsarr         *rp;

	if (!stralloc_copys(&sa, host))
		die_nomem_child();
	partner_fqdn = host;
	/*- get the ip of the MX host */
	switch (dns_ip(&ip, &sa))
	{
	case DNS_MEM:
		die_nomem_child();
	case DNS_SOFT:
		temp_dns();
	case DNS_HARD:
		perm_dns();
	case 1:
		if (ip.len <= 0)
			temp_dns();
	}
	get_tlsa_rr(host, 1, port);
	if (!ta.len) {
		logerrf("dane_query_tlsa: No DANE data were found\n");
		_exit (0);
	}
	/*- print TLSA records */
	for (j = 0, usage = -1; j < ta.len; ++j) {
		rp = &(ta.rr[j]);
		out("TLSA[");
		strnum[fmt_ulong(strnum, (unsigned long) j)] = 0;
		out(strnum);
		out("]:");
		out(rp->host);
		out(" IN TLSA ( ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->usage)] = 0;
		out(strnum);
		out(" ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->selector)] = 0;
		out(strnum);
		out(" ");
		strnum[fmt_ulong(strnum, (unsigned long) rp->mtype)] = 0;
		out(strnum);
		out(" ");
		for (i = hexstring.len = 0; i < rp->data_len; i++) {
			fmt_hexbyte(hex, (rp->data + i)[0]);
			substdio_put(subfdout, hex, 2);
		}
		out(" )\n");
		flush();
	}
#ifdef IPV6
	for (i = 0; i < 16; i++)
		outip.ip6.d[i] = 0;
#else
	for (i = 0; i < 4; i++)
		outip.ip.d[i] = 0;
#endif
	for (i = j = 0; i < ip.len; ++i) {
#ifdef IPV6
		if ((smtpfd = (ip.ix[i].af == AF_INET ? socket_tcp4() : socket_tcp6())) == -1)
#else
		if ((smtpfd = socket_tcp4()) == -1)
#endif
			temp_oserr();
		if (timeoutconn46(smtpfd, &ip.ix[i], &outip, (unsigned int) port, timeoutconnect)) {
			close(smtpfd);
			continue;
		}
		partner = ip.ix[i];
		partner_fqdn = ip.ix[i].fqdn;
		break;
	}
	code = smtpcode();
	if (code >= 500 && code < 600)
		quit("Connected to ", " but greeting failed", code);
	else
	if (code >= 400 && code < 500)
		quit("Connected to ", " but greeting failed", code);
	else
	if (code != 220)
		quit("Connected to ", " but greeting failed", code);
	if (!smtps)
		code = ehlo();
	match0Or512 = authfullMatch = authsha256 = authsha512 = 0;
	if (!tls_init(0, &needtlsauth, &servercert)) {/*- tls is needed for DANE */
		logerr("Connected to ");
		outhost();
		logerrf(" but unable to intiate TLS for DANE\n");
		die (111);
	}
	for (j = 0, usage = -1; j < ta.len; ++j) {
		rp = &(ta.rr[j]);
		if (!rp->mtype || rp->mtype == 2)
			match0Or512 = 1;
		for (i = hexstring.len = 0; i < rp->data_len; i++) {
			fmt_hexbyte(hex, (rp->data + i)[0]);
			if (!stralloc_catb(&hexstring, hex, 2))
				die_nomem_child();
		}
		if (!stralloc_0(&hexstring))
			die_nomem_child();
		if (!(tlsa_status = tlsa_vrfy_records(hexstring.s, rp->usage, rp->selector, rp->mtype, &err_str))) {
			switch(rp->mtype)
			{
			case 0:
				authfullMatch = 1;
				break;
			case 1:
				authsha256 = 1;
				break;
			case 2:
				authsha512 = 1;
				break;
			}
		}
		if (!rp->usage || rp->usage == 2)
			usage = 2;
		if ((!match0Or512 && authsha256) || (match0Or512 && (authfullMatch || authsha512)))
			break;
   } /*- for (j = 0, usage = -1; j < ta.len; ++j) */

	/*-
	 * client SHOULD accept a server public key that
	 * matches either the "3 1 0" record or the "3 1 2" record, but it
	 * SHOULD NOT accept keys that match only the weaker "3 1 1" record.
	 * 9.  Digest Algorithm Agility
	 * https://tools.ietf.org/html/rfc7671
	 */
	if ((!match0Or512 && authsha256) || (match0Or512 && (authfullMatch || authsha512))) {
		if (needtlsauth && usage == 2)
			do_pkix(servercert);
		code = ehlo();
	} else { /*- dane validation failed */
		substdio_putsflush(&smtpto, "QUIT\r\n");
		logerr("Connected to ");
		outhost();
		logerr(" but recipient failed DANE validation.\n");
		substdio_flush(subfderr);
		die (1);
	}
	_exit (0);
	/*- not reached */
	return (0);
}

int
get_dane_records(char *host)
{
	char            strnum[FMT_ULONG];
	char           *ptr;
	int             dane_pid, wstat, dane_exitcode, match, len;
	int             pipefd[2];
	char            inbuf[2048];

	if (pipe(pipefd) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	substdio_fdbuf(&ssin, read, pipefd[0], inbuf, sizeof(inbuf));
	switch ((dane_pid = fork()))
	{
	case -1:
		strerr_die2sys(111, FATAL, "unable to fork: ");
	case 0:
		substdio_discard(subfdout);
		substdio_discard(subfderr);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			die (3);
		if (dup2(pipefd[1], 2) == -1)
			die (3);
		if (pipefd[1] != 1 && pipefd[1] != 2)
			close(pipefd[1]);
		do_work(host, 25);
		die (4);
	default:
		close(pipefd[1]);
		break;
	}
	for (save.len = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			break;
		if (!match && !line.len)
			break;
		line.len--; /*- remove newline */
		if (verbose == 2) {
			if (substdio_put(subfdout, line.s, line.len) == -1)
				strerr_die2sys(111, FATAL, "write: ");
			if (substdio_put(subfdout, "\n", 1) == -1)
				strerr_die2sys(111, FATAL, "write: ");
		}
		if (!str_diffn(line.s, "TLSA[", 5)) {
			for (len = 0, ptr = line.s + 5; *ptr; len++, ptr++) {
				if (*ptr == ':') {
					/*- record1\0record2\0...recordn\0 */
					if (!stralloc_catb(&save, ptr + 1, line.len - (len + 6)))
						die_nomem();
					if (!stralloc_0(&save))
						die_nomem();
					break;
				}
			}
		} else
		if (!str_diffn(line.s, "dane_query_tlsa: No DANE data were found", 40)) {
			if (!stralloc_cat(&save, &line))
				die_nomem();
			if (!stralloc_0(&save)) /*- serves as record field seperator */
				die_nomem();
		}
	}
	close(pipefd[0]);
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (wait_pid(&wstat, dane_pid) != dane_pid) {
		die (111);
	}
	if (wait_crashed(wstat)) {
		logerrf("child crashed\n");
		die (111);
	}
	switch (dane_exitcode = wait_exitcode(wstat))
	{
	case 0: /*- either verified or no TLSA records */
		return (RECORD_OK);
	case 1:
		return (RECORD_NOVRFY);
	case 2:
		if (verbose == 2)
			logerrf("child out of memory\n");
		break;
	case 3:
		if (verbose == 2)
			logerrf("child unable to dup pipefd\n");
		break;
	case 4:
		if (verbose == 2)
			logerrf("child exec failed\n");
		break;
	case 5: /*- perm dns - NO TLSA RR Records */
		if (!stralloc_copyb(&save, "_25._tcp.", 9))
			die_nomem();
		if (!stralloc_cats(&save, host))
			die_nomem();
		if (!stralloc_catb(&save, ": No TLSA RR", 12))
			die_nomem();
		if (!stralloc_0(&save))
			die_nomem();
		return (RECORD_OK);
	case 6:
		break;
	case 100: /*- child returning 100 means domain doesn't exist */
		break;
	default:
		strnum[fmt_ulong(strnum, (unsigned long) dane_exitcode)] = 0;
		logerr(WARN);
		logerr(": error with child: exit code [");
		logerr(strnum);
		logerrf("]\n");
		break;
	}
	return (RECORD_FAIL);
}

/*
 * Responses
 *
 * Failed to add  \0\1
 * unknown record \0\2
 * record bad     \0\3 - bade DANE signature 
 * record new     \1\0
 * record fail    \1\4
 * record white   \1\1
 * record ok      \1\2
 */
int
send_response(int s, union sockunion *from, int fromlen, char *domain,
	int fr_int, struct danerec **dnrecord, int *record_added, int *org_state, int qmr)
{
	char           *resp;
	struct danerec *ptr;
	char            dane_state, rbuf[2];
	int             i, n = 0;

	*record_added = 0;
	*dnrecord = (struct danerec *) 0;
	if (tlsadomainsok && !is_tlsadomain(domain)) {
		resp = "\1\3";
		*org_state = RECORD_OK;
		return (sendto(s, resp, 2, 0, (struct sockaddr *) from, fromlen));
	}
	*org_state = -1;
	switch ((dane_state = search_record(domain, fr_int, dnrecord)))
	{
	case RECORD_NEW:  /*- \1\0 */
		*org_state = dane_state;
		if (qmr == 'q') {
			resp = rbuf;
			rbuf[0] = 0;
			rbuf[1] = RECORD_NEW;
			break; /*- don't add the record & don't do dns, tls transaction */
		} else
		if (qmr == 'S') {
			resp = rbuf;
			rbuf[0] = 1;
			rbuf[1] = dane_state = RECORD_OK;
		} else
		if (qmr == 'F') {
			resp = rbuf;
			rbuf[0] = 1;
			rbuf[1] = dane_state = RECORD_NOVRFY;
		} 
		if (qmr == 'D') {
			dane_state = get_dane_records(domain);
			if (!save.len)
				dane_state = RECORD_OK;
			else
			if (!str_diffn(save.s, "dane_query_tlsa: No DANE data were found", 40))
				dane_state = RECORD_OK;
		}
		if (!add_record(domain, dane_state, dnrecord)) {
			logerrf("unable to add record\n");
			resp = "\0\1";
			n = -2;
		} else {
			ptr = *dnrecord;
			/*- record1\0record2\0...recordn\0\0 */
			if (save.len) {
				if (!(ptr->data = (char *) realloc(ptr->data, save.len + 1)))
					die_nomem();
				byte_copy(ptr->data, save.len, save.s);
				ptr->data[save.len] = 0;
				ptr->datalen = save.len;
			}
			*record_added = 1;
			resp = rbuf;
			if (qmr == 'S' || qmr == 'F')
				rbuf[0] = 1;
			else
				rbuf[0] = (dane_state == RECORD_OK ? 1 : 0);
			rbuf[1] = dane_state;
		}
		break;
	case RECORD_WHITE: /*- \1\1 */
	case RECORD_OK:    /*- \1\2 */
		*org_state = dane_state;
		resp = rbuf;
		if (qmr == 'q') {
			resp = rbuf;
			rbuf[0] = 1;
			rbuf[1] = dane_state;
			break; /*- don't add the record & don't do dns, tls transaction */
		} else
		if (qmr == 'S') {
			ptr = *dnrecord;
			rbuf[0] = 1;
			rbuf[1] = dane_state = ptr->status = RECORD_OK;
		} else
		if (qmr == 'F') {
			ptr = *dnrecord;
			rbuf[0] = 1;
			rbuf[1] = dane_state = ptr->status = RECORD_NOVRFY;
		} else {
			rbuf[0] = 1;
			rbuf[1] = dane_state;
		}
		break;
	case RECORD_FAIL: /*- \1\4 */
	case RECORD_OLD:  /*- \1\5 */
		*org_state = dane_state;
		ptr = *dnrecord;
		resp = rbuf;
		if (qmr == 'q') {
			resp = rbuf;
			rbuf[0] = 1;
			rbuf[1] = dane_state;
			break; /*- don't add the record & don't do dns, tls transaction */
		} else
		if (qmr == 'S') {
			rbuf[0] = 1;
			rbuf[1] = dane_state = ptr->status = RECORD_OK;
		} else
		if (qmr == 'F') {
			rbuf[0] = 1;
			rbuf[1] = dane_state = ptr->status = RECORD_NOVRFY;
		} else
		if (qmr == 'D') {
			dane_state = ptr->status = get_dane_records(domain);
			if (!save.len)
				dane_state = RECORD_OK;
			else
			if (!str_diffn(save.s, "dane_query_tlsa: No DANE data were found", 40))
				dane_state = RECORD_OK;
			if (save.len) {
				if (!(ptr->data = (char *) realloc(ptr->data, save.len + 1)))
					die_nomem();
				byte_copy(ptr->data, save.len, save.s);
				ptr->data[save.len] = 0;
				ptr->datalen = save.len;
			}
		}
		rbuf[0] = (dane_state == RECORD_OK ? 1 : 0);
		rbuf[1] = dane_state;
		break;
	case RECORD_NOVRFY: /*- \0\3 */
		*org_state = dane_state;
		resp = rbuf;
		if (qmr == 'q') {
			resp = rbuf;
			rbuf[0] = 1;
			rbuf[1] = dane_state;
			break; /*- don't add the record & don't do dns, tls transaction */
		} else
		if (qmr == 'S') {
			ptr = *dnrecord;
			rbuf[0] = 1;
			rbuf[1] = dane_state = ptr->status = RECORD_OK;
		} else
		if (qmr == 'F') {
			ptr = *dnrecord;
			rbuf[0] = 1;
			rbuf[1] = dane_state = ptr->status = RECORD_NOVRFY;
		} else {
			rbuf[0] = 0;
			rbuf[1] = dane_state;
		}
		break;
	default:
		*org_state = dane_state;
		resp = "\0\2";
		n = -3;
		break;
	}
	if (!n)
		return (sendto(s, resp, 2, 0, (struct sockaddr *) from, fromlen));
	else {
		if ((i = sendto(s, resp, 2, 0, (struct sockaddr *) from, fromlen)) == -1)
			return (i);
		return (n);
	}
}

char           *pusage =
				"usage: qmail-daned [options] ipaddr context_file\n"
				"Options [ vhtfswi ]\n"
				"        [ -v 0, 1 or 2]\n"
				"        [ -h hash size]\n"
				"        [ -t timeout (days) ]\n"
				"        [ -f free interval (min) ]\n"
				"        [ -s save interval (min) ]\n"
				"        [ -w whitelist ]\n"
				"        [ -T tlsadomains ]\n";
int
main(int argc, char **argv)
{
	int             s = -1, buf_len, rdata_len, n, port, opt, len,
					fromlen, rec_added, o_s, qmr;
	union sockunion sin, from;
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	struct addrinfo hints, *res, *res0;
#endif
	struct hostent *hp;
	struct danerec *dnrec;
	unsigned long   save_interval, free_interval;
	char           *ptr, *ipaddr = 0, *domain, *a_port = "1998";
#ifdef DYNAMIC_BUF
	char           *rdata = 0, *buf = 0;
	int             bufsize = MAXDANEDATASIZE;
#else
	char            rdata[MAXDANEDATASIZE];
#endif
	char            strnum[FMT_ULONG];
	in_addr_t       inaddr;
	fd_set          rfds;
	struct timeval  tv;

	if (control_init() == -1)
		die_control("me");
	if (control_readint(&timeoutconnect, "timeoutconnect") == -1)
		die_control("timeoutconnect");
	if (control_rldef(&helohost, "helohost", 1, (char *) 0) == -1)
		die_control("helohost");
	/*- defaults */
	save_interval = 5 * 60;    /*- 5 mins */
	timeout = 7 * 24 * 3600;   /*- 7 days */
	free_interval = 5 * 60;    /*- 5 minutes */
	hash_size = BLOCK_SIZE;
	while ((opt = getopt(argc, argv, "v:w:i:s:t:h:f:p:")) != opteof) {
		switch (opt) {
		case 'v':
			scan_int(optarg, &verbose);
			break;
		case 'T':
			tlsadomainsfn = optarg;
			break;
		case 'w':
			whitefn = optarg;
			break;
		case 's':
			scan_ulong(optarg, &save_interval);
			save_interval *= 60;
			break;
		case 't':
			scan_ulong(optarg, &timeout);
			timeout *= (24 * 3600);
			break;
		case 'h':
			scan_int(optarg, &hash_size);
			break;
		case 'f':
			scan_ulong(optarg, &free_interval);
			free_interval *= 60; /*- convert to seconds */
			break;
		case 'p':
			a_port = optarg;
			break;
		default:
			strerr_die1x(100, pusage);
			break;
		}
	} /*- while ((opt = getopt(argc, argv, "dw:s:t:g:m:")) != opteof) */
	if (hash_size <= 0 || save_interval <= 0 || free_interval <= 0)
		strerr_die1x(100, pusage);
	if (optind + 2 != argc)
		strerr_die1x(100, pusage);
	ipaddr = argv[optind++];
	if (*ipaddr == '*')
#ifdef IPV6
		ipaddr = "::";
#else
		ipaddr = INADDR_ANY;
#endif
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir: ", auto_qmail, ": ");
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (tlsadomainsfn)
		tlsadomains_init(tlsadomainsfn);
	if (whitefn)
		whitelist_init(whitefn);
	if (whitefn || tlsadomainsfn)
		sig_catch(SIGHUP, sighup);
	for (ptr = argv[optind++]; *ptr;ptr++);
	ptr = argv[optind - 1];
	if (*ptr != '/') {
		if (!stralloc_copys(&context_file, controldir))
			die_nomem();
		if (!stralloc_cats(&context_file, "/"))
			die_nomem();
	}
	if (!stralloc_cats(&context_file, ptr))
		die_nomem();
	if (!stralloc_0(&context_file))
		die_nomem();
	load_context();
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	byte_zero((char *) &hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if ((len = getaddrinfo(ipaddr, a_port, &hints, &res0))) {
		if (len == EAI_ADDRFAMILY || (len == EAI_SYSTEM && errno == EAFNOSUPPORT))
			noipv6 = 1;
		else
			strerr_die7x(111, FATAL, "getadrinfo: ", ipaddr, ": ", a_port, ":", (char *) gai_strerror(len));
	}
	close(0);
	if (!noipv6) {
		for (res = res0; res; res = res->ai_next) {
			if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
				if (errno == EINVAL || errno == EAFNOSUPPORT) {
					noipv6 = 1;
					break;
				}
				freeaddrinfo(res0);
				strerr_die2sys(111, FATAL, "unable to create socket6: ");
			}
			if (dup2(s, 0)) {
				freeaddrinfo(res0);
				strerr_die2sys(111, FATAL, "unable to dup socket: ");
			}
			if (s) {
				close(s);
				s = 0;
			}
			if (bind(s, res->ai_addr, res->ai_addrlen) == 0)
				break;
			freeaddrinfo(res0);
			strerr_die6sys(111, FATAL, "bind6: ", ipaddr, ":", a_port, ": ");
		} /*- for (res = res0; res; res = res->ai_next) */
	}
	freeaddrinfo(res0);
#else
	noipv6 = 1;
#endif
	if (noipv6) {
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
			strerr_die2sys(111, FATAL, "unable to create socket4: ");
		if (dup2(s, 0))
			strerr_die2sys(111, FATAL, "unable to dup socket: ");
		if (s) {
			close(s);
			s = 0;
		}
		port = atoi(a_port);
		sin.sa4.sin_port = htons(port);
		if ((inaddr = inet_addr(ipaddr)) != INADDR_NONE)
			byte_copy((char *) &sin.sa4.sin_addr, 4, (char *) &inaddr);
		else {
			if (!(hp = gethostbyname(ipaddr))) {
				errno = EINVAL;
				strerr_die4sys(111, FATAL, "gethostbyname: ", ipaddr, ": ");
			} else
				byte_copy((char *) &sin.sa4.sin_addr, hp->h_length, hp->h_addr);
		}
		if (bind(s, (struct sockaddr *) &sin.sa4, sizeof(sin)) == -1)
			strerr_die6sys(111, FATAL, "bind4: ", ipaddr, ":", a_port, ": ");
	} 
	sig_catch(SIGTERM, sigterm);
	sig_catch(SIGUSR1, sigusr1);
	sig_catch(SIGUSR2, sigusr2);
	ready();
	for (buf_len = 0, rdata_len = 0;;) {
		char            save = 0;
		int             ret;

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		tv.tv_sec = save_interval;
		tv.tv_usec = 0;
		if ((ret = select(1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &tv)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_die2sys(111, FATAL, "select: ");
		} else
		if (!ret) {
			/*- timeout occurred */
			if (save) {
				save_context();
				save = 0;
			}
			continue;
		}
		if (!FD_ISSET(0, &rfds))
			continue;
#ifdef DYNAMIC_BUF
		for (;;) {
			if ((buf_len < bufsize || buf_len/bufsize > 2) && !(buf = (char *) realloc(buf, (buf_len = bufsize))))
				die_nomem();
			if ((n = recvfrom(s, buf, bufsize, MSG_PEEK, 0, 0)) == -1) {
				if (errno == error_intr)
					continue;
				strerr_die2sys(111, FATAL, "recvfrom: MSG_PEEK: ");
			}
			if (n == bufsize)
				bufsize *= 2;
			else
				break;
		}
		if ((rdata_len < n || rdata_len/n > 2) && !(rdata = (char *) realloc(rdata, (rdata_len = n))))
			die_nomem();
#else
		n = MAXDANEDATASIZE;
#endif
		fromlen = sizeof(from);
		if ((n = recvfrom(s, rdata, n, 0, (struct sockaddr *) &from.sa, (socklen_t *)&fromlen)) == -1) {
			if (errno == error_intr)
				continue;
			strerr_die2sys(111, FATAL, "recvfrom: ");
		}
		save = 1;
		if (verbose) {
			out("qmail-daned IP: ");
#if defined(LIBC_HAS_IP6) && defined(IPV6)
			if (noipv6)
				out(inet_ntoa(from.sa4.sin_addr));
			else {
				static char     addrBuf[INET6_ADDRSTRLEN];
				if (from.sa.sa_family == AF_INET) {
					out((char *) inet_ntop(AF_INET,
						(void *) &from.sa4.sin_addr, addrBuf,
						INET_ADDRSTRLEN));
				} else
				if (from.sa.sa_family == AF_INET6) {
					out((char *) inet_ntop(AF_INET6,
						(void *) &from.sa6.sin6_addr, addrBuf,
						INET6_ADDRSTRLEN));
				} else
				if (from.sa.sa_family == AF_UNSPEC)
					out("::1");
			}
#else
			out(inet_ntoa(from.sa4.sin_addr));
#endif
		}
		/*- 
		 * danerec(3) protocol packet structure -
		 * Dgmail.com\0
		 */
		domain = 0;
		switch (rdata[0])
		{
		case 'q': /*- qmail-remote record exist check */
		case 'S': /*- qmail-remote record update success */
		case 'F': /*- qmail-remote record update failure */
		case 'D': /*- tlsacheck() function */
			qmr = rdata[0];
			if (verbose) {
				out(" rdata[");
				if (substdio_put(subfdout, rdata, 1) == -1)
					strerr_die2sys(111, FATAL, "write: ");
				out("] domain[");
			}
			domain = rdata + 1;
			len = str_len(domain);
			if (verbose) {
				out(domain);
				out("] bufsiz=");
				strnum[fmt_ulong(strnum, (unsigned long) n)] = 0;
				out(strnum);
			}
			if (!len) {
				logerrf("qmail-daned: null domain\n");
				continue;
			}
			n = send_response(s, &from, fromlen, domain, free_interval, &dnrec, &rec_added, &o_s, qmr);
			if (n == -2)
				logerrf("qmail-daned: copy failed\n");
			else
			if (n == -3)
				logerrf("qmail-daned: invalid send_response - report bug to author\n");
			else
			if (n < 0)
				strerr_warn2(WARN, "sendto failed: ", &strerr_sys);
			if (dnrec) {
				if (verbose) {
					out(", attempts=");
					strnum[fmt_ulong(strnum, (unsigned long) dnrec->attempts)] = 0;
					out(strnum);
					out(", Reponse: ");
					if (o_s != dnrec->status) {
						out(print_status(o_s));
						out(" --> ");
					}
					out(print_status(dnrec->status));
					out(" datalen: ");
					strnum[fmt_ulong(strnum, (unsigned long) dnrec->datalen)] = 0;
					out(strnum);
					out("\n");
					for (len = 0, ptr = dnrec->data;len < dnrec->datalen;ptr++, len++) {
						if (!*ptr)
							out("\n");
						else
						if (substdio_put(subfdout, ptr, 1) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				}
			} else {
				if (verbose) {
					out(", Reponse: ");
					out(print_status(o_s));
					out("\n");
				}
			}
			if (dnrec && rec_added && create_hash(dnrec)) /*- add the record to hash list if new */
				die_nomem();
			break;
		default:
			out("Response: RECORD INVALID\n");
			break;
		} /*- switch (rdata[0]) */
		flush();
	} /*- for (buf_len = 0, rdata_len = 0;;) */
	return (0);
}
#else
#warning "TLSA, TLS code not compiled"
int
main()
{
	substdio_puts(subfderr, "not compiled with -DHASTLSA & -DTLS. Check conf-tlsa, conf-tls\n");
	substdio_flush(subfderr);
	_exit (100);
}
#endif

void
getversion_qmail_dane_c()
{
	static char    *x = "$Id: qmail-daned.c,v 1.10 2018-05-28 01:16:18+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
