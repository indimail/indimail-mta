/*
 * $Log: qmail-dane.c,v $
 * Revision 1.2  2018-04-26 01:32:30+05:30  Cprogrammer
 * added tlsadomains control file
 *
 * Revision 1.1  2018-04-25 23:01:47+05:30  Cprogrammer
 * Initial revision
 *
 */
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
#include "stralloc.h"
#include "constmap.h"
#include "env.h"
#include "control.h"
#include "subfd.h"
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

#define FATAL "qmail-dane: fatal: "
#define WARN  "qmail-dane: warning: "

#define RECORD_NEW    0
#define RECORD_WHITE  1
#define RECORD_OK     2
#define RECORD_NOVRFY 3
#define RECORD_FAIL   4
#define BLOCK_SIZE 32768

union sockunion
{
	struct sockaddr     sa;
	struct sockaddr_in  sa4;
#ifdef INET6
	struct sockaddr_in6 sa6;
#endif
};

#if defined(LIBC_HAS_IP6) && defined(IPV6)
int             noipv6 = 0;
#else
int             noipv6 = 1;
#endif

struct danerec
{
	char           *domain;
	char           *data;
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
char           *daneprog;
stralloc        context_file = { 0 };
stralloc        daneprogargs = { 0 };
stralloc        line = { 0 };
stralloc        save = { 0 };
struct substdio ssin;

extern char   **MakeArgs(char *);

void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, FATAL);
	substdio_puts(subfderr, "out of memory\n");
	substdio_flush(subfderr);
	_exit(1);
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
	_exit(1);
}

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
logerr(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
logerrf(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfderr, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (substdio_flush(subfderr) == -1)
		strerr_die2sys(111, FATAL, "write: ");
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
		logerr("initializing tlsadomains\n");
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
		logerrf("initialized  tlsadomains\n");
	return;
}

void
whitelist_init(char *arg)
{
	if (verbose > 2)
		logerr("initializing whitelist\n");
	if ((whitelistok = control_readfile(&whitelist, arg, 0)) == -1)
		die_control(arg);
	if (whitelistok && !constmap_init(&mapwhite, whitelist.s, whitelist.len, 0))
		die_nomem();
	if (verbose > 2)
		logerrf("initialized  whitelist\n");
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
		_exit (1);
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
		_exit (1);
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
				logerrf("unable to add to hash table\n");
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
		return (RECORD_NEW);
}

/*
 * add entry to link list
 * group in the link list if found in hash list
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
	char            timestamp[FMT_ULONG];

	if (!dane_count)
		return;
	if (verbose > 2)
		logerr("saving context file\n");
	if ((context_fd = open(context_file.s, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to open file: ", context_file.s, ": ");
	for (ptr = head;ptr;ptr = ptr->next) {
		if (ptr->timestamp < time(0) - timeout) /*- don't save expired records */
			continue;
		timestamp[fmt_ulong(timestamp, ptr->timestamp)] = 0;
		if (write_file(context_fd, ptr->domain, str_len(ptr->domain)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, (char *) &ptr->status, 1) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, timestamp, str_len(timestamp)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, "\n", 1) == -1)
		{
			break;
		}
		if (!ptr->next)
			break;
	} /*- for (ptr = head;ptr;ptr = ptr->next) */
	close(context_fd);
	if (verbose > 2)
		logerrf("saved  context file\n");
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
	_exit(0);
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
	case RECORD_OK :
		return ("RECORD OK");
	}
	return ("RECORD UNKNOWN");
}

int
get_dane_record(char *domain)
{
	char          **Argv;
	int             dane_pid, wstat, dane_exitcode, match;
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
		if (!stralloc_copys(&daneprogargs, daneprog))
			_exit(2);
		if (!stralloc_catb(&daneprogargs, " ", 1))
			_exit(2);
		if (!stralloc_cats(&daneprogargs, domain))
			_exit(2);
		if (!stralloc_0(&daneprogargs))
			_exit(2);
		if (!(Argv = MakeArgs(daneprogargs.s)))
			_exit(2);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			_exit(3);
		if (dup2(pipefd[1], 2) == -1)
			_exit(3);
		if (pipefd[1] != 1 && pipefd[1] != 2)
			close(pipefd[1]);
		execv(*Argv, Argv);
		_exit(4);
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
		if (!stralloc_copy(&save, &line))
			die_nomem();
		if (verbose == 2 && substdio_put(subfdout, line.s, line.len) == -1)
			strerr_die2sys(111, FATAL, "write: ");
	}
	close(pipefd[0]);
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (wait_pid(&wstat, dane_pid) != dane_pid) {
		_exit(111);
	}
	if (wait_crashed(wstat)) {
		logerr("program ");
		logerr(daneprog);
		logerrf(" crashed\n");
		_exit(111);
	}
	switch (dane_exitcode = wait_exitcode(wstat))
	{
	case 0:
		return (RECORD_OK);
	case 1:
		return (RECORD_NOVRFY);
	case 2:
		logerrf("child: out of memory\n");
		break;
	case 3:
		logerrf("child: unable to dup pipefd\n");
		break;
	case 4:
		logerrf("child: exec failed\n");
		break;
	default:
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
	int fr_int, struct danerec **dnrecord, int *record_added, int *org_state)
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
	case RECORD_FAIL: /*- \1\4 */
		*org_state = dane_state;
		dane_state = get_dane_record(domain);
		if (!str_diffn(save.s, "dane_query_tlsa: No DANE data were found", 40)) {
			dane_state = RECORD_OK;
		}
		if (!add_record(domain, dane_state, dnrecord)) {
			logerrf("unable to add record\n");
			resp = "\0\1";
			n = -2;
		} else {
			ptr = *dnrecord;
			if (save.len) {
				if (!(ptr->data = (char *) realloc(ptr->data, save.len + 1)))
					die_nomem();
				if (str_copyb(ptr->data, save.s, save.len) != save.len) {
					die_nomem();
				}
				ptr->data[save.len] = 0;
			}
			*record_added = 1;
			resp = rbuf;
			rbuf[0] = (dane_state == RECORD_OK ? 1 : 0);
			rbuf[1] = dane_state;
		}
		break;
	case RECORD_NOVRFY: /*- \0\3 */
		*org_state = dane_state;
		rbuf[0] = 0;
		rbuf[1] = dane_state;
		break;
	case RECORD_WHITE: /*- \1\1 */
	case RECORD_OK:    /*- \1\2 */
		*org_state = dane_state;
		resp = rbuf;
		rbuf[0] = 1;
		rbuf[1] = dane_state;
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

char           *usage =
				"usage: qmail-dane [options] ipaddr context_file\n"
				"Options [ vhtfsw ]\n"
				"        [ -v 0, 1 or 2]\n"
				"        [ -h hash size]\n"
				"        [ -t timeout (days) ]\n"
				"        [ -f free interval (min) ]\n"
				"        [ -s save interval (min) ]\n"
				"        [ -w whitelist ]\n"
				"        [ -i tlsadomains ]\n";
int
main(int argc, char **argv)
{
	int             s = -1, buf_len, rdata_len, n, port, opt, len,
					fromlen, rec_added, o_s;
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

	if (!(daneprog = env_get("DANEPROG"))) {
		logerrf("dane verifier program DANEPROG not defined in environment\n");
		_exit (111);
	}
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
		case 'i':
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
			strerr_die1x(100, usage);
			break;
		}
	} /*- while ((opt = getopt(argc, argv, "dw:s:t:g:m:")) != opteof) */
	if (hash_size <= 0 || save_interval <= 0 || free_interval <= 0)
		strerr_die1x(100, usage);
	if (optind + 2 != argc)
		strerr_die1x(100, usage);
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
			out("qmail-dane IP: ");
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
		case 'D':
			if (verbose)
				out(" [");
			domain = rdata + 1;
			len = str_len(domain);
			if (verbose) {
				out("Domain: ");
				out(domain);
				out("] bufsiz=");
				strnum[fmt_ulong(strnum, (unsigned long) n)] = 0;
				out(strnum);
			}
			if (!len) {
				logerrf("qmail-dane: null domain\n");
				continue;
			}
			n = send_response(s, &from, fromlen, domain, free_interval, &dnrec, &rec_added, &o_s);
			if (n == -2)
				logerrf("qmail-dane: copy failed\n");
			else
			if (n == -3)
				logerrf("qmail-dane: invalid send_response - report bug to author\n");
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
					out(" ");
					out(dnrec->data);
				}
			} else {
				if (verbose) {
					out(", Reponse: ");
					out(print_status(o_s));
				}
			}
			if (verbose)
				out("\n");
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

void
getversion_qmail_dane_c()
{
	static char    *x = "$Id: qmail-dane.c,v 1.2 2018-04-26 01:32:30+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
