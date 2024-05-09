/*
 * $Id: qmail-greyd.c,v 1.37 2024-02-08 22:02:15+05:30 Cprogrammer Exp mbhangui $
 */
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include "sgetopt.h"
#include <sig.h>
#include <getln.h>
#include <fmt.h>
#include <str.h>
#include <subfd.h>
#include <byte.h>
#include <strerr.h>
#include <scan.h>
#include <env.h>
#include <stralloc.h>
#include <constmap.h>
#include <error.h>
#include <uint32.h>
#include <open.h>
#include <noreturn.h>
#include "ip.h"
#include "auto_control.h"
#include "control.h"
#include "tablematch.h"
#include "variables.h"
#include "greylist.h"
#include "haveip6.h"
#include "socket.h"
#include "cdb_match.h"
#include "varargs.h"
#include <search.h>

#define FATAL "qmail-greyd: fatal: "
#define WARN  "qmail-greyd: warn: "

#define RECORD_FREE  -1
#define RECORD_NEW    0
#define RECORD_EARLY  1
#define RECORD_STALE  2
#define RECORD_WHITE  3
#define RECORD_OK     4
#define RECORD_GREY   5
#define RECORD_BOUNCE 6
#define RECORD_IPV6   7
#define BUILD_IP(IP) ((IP[0]<<24) | (IP[1]<<16) | (IP[2]<<8) | IP[3])
#define VALID_IP(IP) ((IP[0]<256) && (IP[1]<256) && (IP[2]<256) && (IP[3]<256))
#define BLOCK_SIZE 32768

union sockunion
{
	struct sockaddr     sa;
	struct sockaddr_in  sa4;
#ifdef IPV6
	struct sockaddr_in6 sa6;
#endif
};

struct greylst
{
	union v46addr   ip;
	char            ip_str[IPFMT];
	char           *rpath;
	char           *rcpt; /* Trcpt1\0Trcpt2\0.....Trcptn\0 */
	unsigned int    rcptlen;
	time_t          timestamp;
	int             attempts;
	char            status; /*- greylisting status */
	struct greylst *prev, *next; /*- prev, next of linked list of greylst structures */
	struct greylst *ip_prev, *ip_next; /*- group records for an ip address+rpath+rcpt together */
};

struct netspec
{
	unsigned int    min;
	unsigned int    max;
};

static struct greylst *head;
static struct greylst *tail;
static unsigned long   timeout;
static int      verbose = 0;
static int      grey_count, hcount;
static int      hash_size, h_allocated = 0;
static char    *whitefn = 0;
static stralloc context_file = { 0 };

no_return void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, FATAL);
	substdio_puts(subfderr, "out of memory\n");
	substdio_flush(subfderr);
	_exit(1);
}

no_return void
die_control(const char *arg)
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
#ifdef  HAVE_STDARG_H
out(const char *s1, ...)
#else
out(va_alist)
va_dcl
#endif
{
	va_list         ap;
	const char     *str;
#ifndef HAVE_STDARG_H
	const char     *s1;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, s1);
#else
	va_start(ap);
	s1 = va_arg(ap, const char *);
#endif

	if (substdio_puts(subfdout, s1) == -1)
		_exit(1);
	while (1) {
		str = va_arg(ap, const char *);
		if (!str)
			break;
		if (substdio_puts(subfdout, str) == -1)
			_exit(1);
	}
	va_end(ap);
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
#ifdef  HAVE_STDARG_H
logerr(const char *s1, ...)
#else
logerr(va_alist)
va_dcl
#endif
{
	va_list         ap;
	const char     *str;
#ifndef HAVE_STDARG_H
	const char     *s1;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, s1);
#else
	va_start(ap);
	s1 = va_arg(ap, const char *);
#endif

	if (substdio_puts(subfderr, s1) == -1)
		_exit(1);
	while (1) {
		str = va_arg(ap, const char *);
		if (!str)
			break;
		if (substdio_puts(subfderr, str) == -1)
			_exit(1);
	}
	va_end(ap);
}

void
logflush()
{
	if (substdio_flush(subfderr) == -1)
		_exit(1);
	return;
}

/*
 * Convert Classless Inter-Domain Routing addresses into range
 * Range can filled in struct netspec
 */
int
cidr2IPrange(const char *ipaddr, int mask, struct netspec *spec)
{
	ip_addr         ip;

	if (!ip4_scan(ipaddr, &ip)) {
		logerr("failed to scan ip", "[", ipaddr, "]\n", NULL);
		logflush();
		return (-1);
	}
	spec->min = BUILD_IP(ip.d) & (~((1 << (32 - mask)) -1) & 0xFFFFFFFF);
	spec->max = spec->min | (((1 << (32 - mask)) - 1) & 0xFFFFFFFF);
	return (0);
}

int             whitelistok = 0;
stralloc        whitelist = { 0 };
struct constmap mapwhite;

void
whitelist_init(const char *arg)
{
	if (verbose > 2) {
		logerr("initializing whitelist\n", NULL);
		logflush();
	}
	if ((whitelistok = control_readfile(&whitelist, arg, 0)) == -1)
		die_control(arg);
	if (whitelistok && !constmap_init(&mapwhite, whitelist.s, whitelist.len, 0))
		die_nomem();
	if (verbose > 2) {
		logerr("initialized  whitelist\n", NULL);
		logflush();
	}
	return;
}

int
ip_match(stralloc *ipaddr, stralloc *content, struct constmap *ptrmap,
	const char *errStr[])
{
	int             x, len, mask;
	struct netspec  netspec;
	unsigned int    tmp_ip;
	ip_addr         ip;
	char           *ptr;

	if (errStr)
		*errStr = NULL;
	if (whitefn) {
		switch ((x = cdb_matchaddr(whitefn, ipaddr->s, ipaddr->len - 1)))
		{
		case 0:
		case 1:
			return (x);
		case CDB_MEM_ERR:
			die_nomem();
		case CDB_LSEEK_ERR:
			strerr_die3sys(111, FATAL, whitefn, ".cdb: lseek: ");
		case CDB_FILE_ERR:
			strerr_die4sys(111, FATAL, "unable to read cdb file ", whitefn, ".cdb: ");
		}
	} else
	if (ptrmap && constmap(ptrmap, ipaddr->s, ipaddr->len - 1))
		return 1;
	if (!content)
		return (0);
	for (len = 0, ptr = content->s;len < content->len;) {
		x = str_chr(ptr, '/');
		if (ptr[x]) {
			ptr[x] = 0;
			scan_int(ptr + 1, &mask);
			if (cidr2IPrange(ptr, mask, &netspec) == -1) {
				if (errStr)
					*errStr = error_str(errno);
				ptr[x] = '/';
				logerr("invalid IP in CIDR format: ", ptr, "\n", NULL);
				logflush();
				return (-1);
			}
			ptr[x] = '/';
			if (!ip4_scan(ipaddr->s, &ip)) { /*- record contains invalid IP */
				logerr("failed to scan ip", "[", ipaddr->s, "]\n", NULL);
				logflush();
				return (0);
			}
			tmp_ip = BUILD_IP(ip.d);
			if (tmp_ip == netspec.min || tmp_ip == netspec.max
					|| (tmp_ip > netspec.min && tmp_ip < netspec.max))
				return (1);
		}  else /*- not in cidr notation */
		if (matchinet(ipaddr->s, ptr, 0))
			return (1);
		len += (str_len(ptr) + 1);
		ptr = content->s + len;
	}
	return (0);
}

int
is_white(const char *ip)
{
	const char     *errStr = NULL;
	static stralloc ipaddr = { 0 };

	if (!stralloc_copys(&ipaddr, ip) ||
			!stralloc_0(&ipaddr))
		die_nomem();
	switch (ip_match(&ipaddr, whitelistok ? &whitelist : 0,
			whitelistok ? &mapwhite : 0, &errStr))
	{
	case 1:
		return (1);
	case 0:
		return (0);
	default:
		substdio_flush(subfdout);
		logerr("error in ip_match: ", errStr, "\n", NULL);
		logflush();
		_exit (1);
	}
	/*- Not reached */
	return (0);
}

int
copy_grey(struct greylst *ptr, const char *ipaddr, const char *rpath, const char *rcpt, int rcptlen)
{
	int             len;

#ifdef IPV6
	byte_zero((char *) &ptr->ip, sizeof(union v46addr));
	if (!ip6_scan(ipaddr, &ptr->ip.ip6))
#else
	if (!ip4_scan(ipaddr, &ptr->ip.ip))
#endif
	{
		logerr("failed to scan ip", "[", ipaddr, "]\n", NULL);
		logflush();
		return (-1);
	}
	len = str_len(rpath);
	if (!(ptr->rpath = (char *) malloc(len + 1)))
		die_nomem();
	if (str_copy(ptr->rpath, rpath) != len) {
		free(ptr->rpath);
		return (-1);
	}
	if (!(ptr->rcpt = (char *) malloc(rcptlen)))
		die_nomem();
	ptr->rcptlen = rcptlen;
	byte_copy(ptr->rcpt, rcptlen, rcpt);
	return (0);
}

int
grey_compare(int *key, struct greylst *ptr)
{
	if (*key < ptr->timestamp)
		return (-1);
	else
	if (*key == ptr->timestamp)
		return (0);
	return (1);
}

void
print_record(const char *ip, const char *rpath, const char *rcpt, int rcptlen, time_t timestamp,
	char status, int operation)
{
	char            strnum[FMT_ULONG];
	const char     *ptr;

	strnum[fmt_ulong(strnum, (unsigned long) timestamp)] = 0;
	out(strnum, " IP: ", ip, " FROM: ", rpath, " RCPT: [", NULL);
	for (ptr = rcpt;ptr < rcpt + rcptlen;) {
		out(ptr + 1, NULL);
		ptr += (str_len(ptr) + 1);
		if (ptr < rcpt + rcptlen)
			out(" ", NULL);
		else
			out("]", NULL);
	}
	out(" status=", NULL);
	switch(status)
	{
	case RECORD_NEW:
		out("RECORD_NEW", NULL);
		break;
	case RECORD_EARLY:
		out("RECORD_EARLY", NULL);
		break;
	case RECORD_STALE:
		out("RECORD_STALE", NULL);
		break;
	case RECORD_WHITE:
		out("RECORD_WHITE", NULL);
		break;
	case RECORD_OK:
		out("RECORD_OK", NULL);
		break;
	case RECORD_GREY:
		out("RECORD_GREY", NULL);
		break;
	case RECORD_BOUNCE:
		out("RECORD_BOUNCE", NULL);
		break;
	}
	strnum[fmt_ulong(strnum, (unsigned long) rcptlen)] = 0;
	out(" rcptlen=", strnum, operation ? " expired\n" : "\n", NULL);
	flush();
	return;
}

int
compare_ip(const unsigned char *ip1, const unsigned char *ip2)
{
	register int    i;

#ifdef IPV6
	for (i = 0; i < 16; i++)
#else
	for (i = 0; i < 4; i++)
#endif
	{
		if (ip1[i] != ip2[i])
			return (ip1[i] < ip2[i] ? -1 : 1);
	}
	return (0);
}

int
create_hash(struct greylst *curr)
{
	struct greylst *ip_ptr, *ptr;
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
			strnum[fmt_ulong(strnum, (unsigned long) ((125 * hash_size * h_allocated)/100))] = 0;
			logerr("WARNING!! recreating hash table, size=", strnum, "\n", NULL);
			logflush();
		} else { /*- first time/expiry */
			if (verbose > 2) {
				strnum[fmt_ulong(strnum, (unsigned long) ((125 * hash_size * h_allocated)/100))] = 0;
				out("creating hash table, size=", strnum, "\n", NULL);
			}
		}
		if (!hcreate((125 * hash_size * h_allocated)/100)) /*- be 25% generous */
			strerr_die2sys(111, FATAL, "unable to create hash table: ");
	}
	for (ptr = (curr ? curr : head);ptr;ptr = ptr->next) { /*-
															* if curr is null -> add all entries from linked list.
															* this will happen if hash table is new
															*/
		e.key = ptr->ip_str;
		if (!(ep = hsearch(e, FIND))) { /*- add entries when hash table is new or after destruction */
			e.data = ptr;
			if (!(ep = hsearch(e, ENTER))) {
				logerr("unable to add to hash table\n", NULL);
				logflush();
				return (-1);
			} else
				hcount++;
		} else { /*- add new entry to end of linked list */
			/*- multiple entries for an IP gets linked through ip_next and ip_prev */
			for (ip_ptr = (struct greylst *) ep->data;ip_ptr && ip_ptr->ip_next;ip_ptr = ip_ptr->ip_next);
			if (!ip_ptr->ip_next) { /*- we are now at the end of the linked list */
				ip_ptr->ip_next = ptr;
				ptr->ip_next = 0;
				ptr->ip_prev = ip_ptr;
			}
		}
		if (curr) /*- add only one entry */
			break;
	}
	return (0);
}

void
expire_records(time_t cur_time)
{
	struct greylst *ptr, *x;
	time_t          start;

	logerr("expiring records\n", NULL);
	/*- find all records that are expired where timestamp < start */
	start = cur_time - timeout;
	for (ptr = head; ptr; ) {
		if (ptr->timestamp >= start) {
			ptr = ptr->next;
			continue;
		}
		if (verbose > 1)
			print_record(ptr->ip_str, ptr->rpath, ptr->rcpt, ptr->rcptlen, ptr->timestamp,
				ptr->status, 1);
		grey_count--;
		if (ptr == head) {
			head = ptr->next;
			head->prev = 0;
		}
		(ptr->prev)->next = ptr->next;
		(ptr->next)->prev = ptr->prev;
		free(ptr->rpath);
		free(ptr->rcpt);
		x = ptr->next;
		free(ptr);
		ptr = x;
	}
	logerr("expired  records\n", NULL);
	logflush();
	/*- destroy and recreate the hash */
	if (create_hash(0))
		die_nomem();
	return;
}

struct greylst *grey_index;
/*
 * |- head  <-----
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             | expired entries
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |             |
 * |- start <-----                 <-----
 * |                                    |
 * |                                    |
 * |                                    |
 * |- record entry (gt) <--  <--        |
 * |                      |    |        |
 * |                      |    |        |
 * |-        min_resend <--    |        | --> timeout
 * |                           |        |
 * |                           |        |
 * |- tail                     |        |
 * |                           |        |
 * |                           |        |
 * |- now                      |   <-----
 * |                           |
 * |-          resend_window <--
 * |
 */

int
search_record(const char *remoteip, const char *rpath, const char *rcpt, int rcptlen, int min_resend,
	int resend_win, int fr_int, struct greylst **store)
{
	struct greylst *ptr;
	time_t          cur_time, start;
	union v46addr   r_ip;
	ENTRY           e, *ep;
	int             found;

	*store = (struct greylst *) 0;
	if (whitelistok && is_white(remoteip))
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
#ifdef IPV6
	byte_zero((char *) &r_ip, sizeof(union v46addr));
	if (!ip6_scan(remoteip, &r_ip.ip6))
#else
	if (!ip4_scan(remoteip, &r_ip.ip))
#endif
	{
		logerr("failed to scan ip [", remoteip, "]\n", NULL);
		logflush();
		return (0);
	}
	e.key = (char *) remoteip;
	if (!(ep = hsearch(e, FIND)))
		return (RECORD_NEW);
	for (found = 0, ptr = (struct greylst *) ep->data;ptr;ptr = ptr->ip_next) {
#ifdef IPV6
		if (!compare_ip(ptr->ip.ip6.d, r_ip.ip6.d) && !str_diffn(ptr->rpath, rpath, str_len(rpath))
			&& (rcptlen == ptr->rcptlen && !byte_diff(ptr->rcpt, ptr->rcptlen, rcpt)))
#else
		if (!compare_ip(ptr->ip.ip.d, r_ip.ip.d) && !str_diffn(ptr->rpath, rpath, str_len(rpath))
			&& (rcptlen == ptr->rcptlen && !byte_diff(ptr->rcpt, ptr->rcptlen, rcpt)))
#endif
		{
			found = 1;
			break;
		}
	}
	if (found) {
		*store = ptr;
		ptr->attempts++;
		/*- # not older than timeout days */
		if ((ptr->status == RECORD_GREY || ptr->status == RECORD_OK)
			&& (ptr->timestamp > cur_time - timeout)) {
			ptr->timestamp = cur_time;
			ptr->status = RECORD_OK;
			return (RECORD_OK); /*- "\1\2" */
		} else
		if (cur_time < ptr->timestamp + min_resend) {
			/*- too early */
			ptr->status = RECORD_EARLY;
			return (RECORD_EARLY); /*- "\0\2" */
		} else
		if (cur_time > ptr->timestamp + resend_win) {
			ptr->timestamp = cur_time;
			ptr->status = RECORD_STALE;
			return (RECORD_STALE); /*- "\0\3" */
		} else {
			ptr->timestamp = cur_time;
			ptr->status = (*rpath ? RECORD_GREY : RECORD_BOUNCE); /*- expire bounces */
			return (*rpath ? RECORD_GREY : RECORD_BOUNCE); /*- "\1\3" */
		}
	}
	return (RECORD_NEW);
}

/*
 * add entry to link list
 * group in the link list if found in hash list
 * added to hash list if not found in hash list
 */
struct greylst *
add_record(const char *ip, const char *rpath, const char *rcpt, int rcptlen, struct greylst **grey)
{
	struct greylst *ptr;

	if (!head) {
		if (!(ptr = (struct greylst *) malloc(sizeof(struct greylst))))
			die_nomem();
		ptr->prev = ptr->next = ptr->ip_next = ptr->ip_prev = 0;
		head = tail = ptr;
	} else {
		if (!(ptr = (struct greylst *) malloc(sizeof(struct greylst))))
			die_nomem();
		ptr->prev = tail;
		tail->next = ptr;
		tail = ptr;
		ptr->ip_prev = ptr->ip_next = 0;
	}
	byte_copy(ptr->ip_str, str_len(ip) + 1, ip);
	*grey = ptr;
	grey_count++;
	ptr->timestamp = time(0);
	ptr->attempts = 1;
	ptr->status = RECORD_NEW;
	if (copy_grey(ptr, ip, rpath, rcpt, rcptlen)) {
		free(ptr);
		return ((struct greylst *) 0);
	}
	return (ptr);
}

int
send_response(int s, union sockunion *from, int fromlen, const char *ip, const char *rpath,
	const char *rcpt, int rcptlen, int min_resend, int resend_win, int fr_int,
	struct greylst **grey, int *record_added)
{
	const char     *resp;
	int             i, n = 0;
#ifndef IPV6
	unsigned char   ibuf[sizeof(struct in6_addr)];
#endif

	*grey = (struct greylst *) 0;
	*record_added = 0;
#ifndef IPV6
	if ((i = inet_pton(AF_INET6, ip, ibuf)) == 1) {
		resp = "\1\4";
		n = -3; /*- inet6 address */
	} else
	if (i == -1) {
		resp = "\0\4";
		n = -1; /*- system error */
	} else /*- Not in ip6 presentation format */
#endif
	switch (search_record(ip, rpath, rcpt, rcptlen, min_resend, resend_win, fr_int, grey))
	{
	case RECORD_NEW:
		if (!add_record(ip, rpath, rcpt, rcptlen, grey)) {
			logerr("unable to add record\n", NULL);
			logflush();
			resp = "\0\4";
			n = -4;
		} else {
			*record_added = 1;
			resp = "\0\1";
		}
		break;
	case RECORD_EARLY:
		resp = "\0\2";
		break;
	case RECORD_STALE:
		resp = "\0\3";
		break;
	case RECORD_WHITE:
		resp = "\1\1";
		break;
	case RECORD_OK :
		resp = "\1\2";
		break;
	case RECORD_GREY:
		resp = "\1\3";
		break;
	case RECORD_BOUNCE:
		resp = "\1\4";
		break;
	default:
		resp = "0\5";
		n = -2;
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

int
write_file(int fd, const char *arg, int len)
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
	struct greylst *ptr;
	int             context_fd;
	char            rcptlen[FMT_ULONG], timestamp[FMT_ULONG];

	if (!grey_count)
		return;
	if (verbose > 2) {
		logerr("saving context file\n", NULL);
		logflush();
	}
	if ((context_fd = open(context_file.s, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to open file: ", context_file.s, ": ");
	for (ptr = head;ptr;ptr = ptr->next) {
		if (ptr->timestamp < time(0) - timeout) /*- don't save expired records */
			continue;
		rcptlen[fmt_ulong(rcptlen, ptr->rcptlen)] = 0;
		timestamp[fmt_ulong(timestamp, ptr->timestamp)] = 0;
		if (write_file(context_fd, ptr->ip_str, str_len(ptr->ip_str)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, ptr->rpath, str_len(ptr->rpath)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, rcptlen, str_len(rcptlen)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, ptr->rcpt, ptr->rcptlen) == -1
			|| write_file(context_fd, timestamp, str_len(timestamp)) == -1
			|| write_0(context_fd) == -1
			|| write_file(context_fd, (char *) &ptr->status, 1) == -1
			|| write_file(context_fd, "\n", 1) == -1)
		{
			break;
		}
		if (!ptr->next)
			break;
	} /*- for (ptr = head;ptr;ptr = ptr->next) */
	close(context_fd);
	if (verbose > 2) {
		logerr("saved  context file\n", NULL);
		logflush();
	}
	return;
}

stralloc        context = { 0 };
stralloc        line = { 0 };

/*-
 * load records on startup from a context file which
 * was saved on shutdown (SIGTERM)
 */
void
load_context()
{
	int             fd, match, rcptlen;
	time_t          timestamp, cur_time;
	substdio        ss;
	char            inbuf[2048];
	char            status;
	char           *cptr, *ip, *rpath, *rcpt;
	struct greylst *ptr;

	if (verbose > 2) {
		out("loading context\n", NULL);
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
		ip = cptr;
		cptr += str_len(cptr) + 1;
		rpath = cptr;
		cptr += str_len(cptr) + 1;
		scan_int(cptr, &rcptlen); /*- rcptlen */
		cptr += str_len(cptr) + 1;
		rcpt = cptr;
		cptr += rcptlen;
		scan_ulong(cptr, (unsigned long *) &timestamp);
		cptr += str_len(cptr) + 1;
		status = *cptr;
		if (verbose > 1) {
			if (timestamp < cur_time - timeout) { /*- expired records */
				print_record(ip, rpath, rcpt, rcptlen, timestamp, status, 1);
				continue;
			} else
				print_record(ip, rpath, rcpt, rcptlen, timestamp, status, 0);
			}
		if (!head) {
			if (!(ptr = (struct greylst *) malloc(sizeof(struct greylst))))
				die_nomem();
			ptr->prev = 0;
			ptr->next = 0;
			head = tail = ptr;
		} else {
			if (!(ptr = (struct greylst *) malloc(sizeof(struct greylst))))
				die_nomem();
			ptr->prev = tail;
			tail->next = ptr;
			tail = ptr;
		}
		byte_copy(ptr->ip_str, str_len(ip) + 1, ip);
		if (create_hash(ptr)) /*- load previous context into hash table */
			die_nomem();
		grey_count++;
		ptr->timestamp = timestamp;
		ptr->status = status;
		if (copy_grey(ptr, ip, rpath, rcpt, rcptlen)) {
			free(ptr);
			return;
		}
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

	strnum[fmt_ulong(strnum, (unsigned long) grey_count)] = 0;
	out("Ready for connections, grey_count=", strnum, ", hcount=", NULL);
	strnum[fmt_ulong(strnum, (unsigned long) hcount)] = 0;
	out(strnum, ", hash_size=", NULL);
	strnum[fmt_ulong(strnum, (unsigned long) ((125 * hash_size * h_allocated)/100))] = 0;
	out(strnum, "\n", NULL);
	flush();
}

/*-
 * expire records on SIGUSR2
 */
void
sigusr2()
{
	struct greylst *ptr, *ip_ptr;
	int             i, j;
	char            strnum[FMT_ULONG];

	sig_block(SIGUSR2);
	expire_records(time(0));
	if (verbose > 2) {
		for (i = 1, ptr = head; ptr; ptr = ptr->next, i++) {
			strnum[fmt_ulong(strnum, (unsigned long) i)] = 0;
			out(strnum, " ", NULL);
			print_record(ptr->ip_str, ptr->rpath, ptr->rcpt, ptr->rcptlen, ptr->timestamp,
				ptr->status, 0);
			for (j = 1, ip_ptr = ptr->ip_next;ip_ptr;ip_ptr = ip_ptr->ip_next, j++) {
				strnum[fmt_ulong(strnum, (unsigned long) j)] = 0;
				out(" ", strnum, " ", NULL);
				print_record(ip_ptr->ip_str, ip_ptr->rpath, ip_ptr->rcpt, ip_ptr->rcptlen, ip_ptr->timestamp,
					ptr->status, 0);
			}
		}
	}
	sig_unblock(SIGUSR2);
	ready();
	return;
}

no_return void
sigterm()
{
	sig_block(SIGTERM);
	logerr("ARGH!! Committing suicide on SIGTERM\n", NULL);
	logflush();
	save_context();
	flush();
	_exit(0);
}

const char     *
print_status(char status)
{
	switch (status)
	{
	case RECORD_NEW:
		return ("RECORD NEW");
	case RECORD_EARLY:
		return ("RECORD EARLY");
	case RECORD_STALE:
		return ("RECORD STALE");
	case RECORD_WHITE:
		return ("RECORD WHITE");
	case RECORD_OK :
		return ("RECORD OK");
	case RECORD_GREY:
		return ("RECORD GREY");
	case RECORD_BOUNCE:
		return ("RECORD BOUNCE");
	case RECORD_IPV6:
		return ("RECORD IPV6");
	}
	return ("RECORD UNKNOWN");
}

/*
 * Arguments on the lines of greydaemon by John Levine
 * greydaemon [-w whitelist] [-t timeout_days] [-g resend_window_hours]
 *     [-m min_resend_minutes] ipaddr savefile
 */
const char     *usage =
				"usage: qmail-greyd [options] ipaddr context_file\n"
				"Options [ vhtgmfspw ]\n"
				"        [ -v 0, 1 or 2]\n"
				"        [ -h hash size]\n"
				"        [ -t timeout (days) ]\n"
				"        [ -g resend window (hours) ]\n"
				"        [ -m minimum resend time (min) ]\n"
				"        [ -f free interval (min) ]\n"
				"        [ -s save interval (min) ]\n"
				"        [ -p port]\n"
				"        [ -w whitelist ]\n";
int
main(int argc, char **argv)
{
	int             s = -1, buf_len, rdata_len, n, port, opt, fromlen, rcptlen, len, state, rec_added;
	union sockunion sin, from;
#if defined(LIBC_HAS_IP6)
	struct addrinfo hints = {0}, *res = 0, *res0 = 0;
#else
	struct hostent *hp;
#endif
	struct greylst *grey;
	unsigned long   resend_window, min_resend, save_interval, free_interval;
	char           *ptr, *client_ip = 0, *rpath = 0, *rcpt_head = 0;
	const char     *ipaddr, *a_port = "1999";
#ifdef DYNAMIC_BUF
	char           *rdata = 0, *buf = 0;
	int             bufsize = MAXGREYDATASIZE;
#else
	char            rdata[MAXGREYDATASIZE];
#endif
	char            strnum[FMT_ULONG];
	in_addr_t       inaddr;
	fd_set          rfds;
	struct timeval  tv;

	/*- defaults */
	save_interval = 5 * 60;    /*- 5 mins */
	timeout = 7 * 24 * 3600;   /*- 7 days */
	resend_window = 12 * 3600; /*- 12 hours */
	min_resend = 5 * 60;       /*- 5 minutes */
	free_interval = 5 * 60;    /*- 5 minutes */
	hash_size = BLOCK_SIZE;
	while ((opt = getopt(argc, argv, "v:w:s:t:g:m:h:p:")) != opteof) {
		switch (opt)
		{
		case 'v':
			scan_int(optarg, &verbose);
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
		case 'g':
			scan_ulong(optarg, &resend_window);
			resend_window *= 3600; /*- convert to seconds */
			break;
		case 'h':
			scan_int(optarg, &hash_size);
			break;
		case 'm':
			scan_ulong(optarg, &min_resend);
			min_resend *= 60; /*- convert to seconds */
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
	if (hash_size <= 0 || save_interval <= 0 || resend_window <= 0 || min_resend <= 0 || free_interval <= 0)
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
	if (!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (whitefn) {
		whitelist_init(whitefn);
		sig_catch(SIGHUP, sighup);
	}
	for (ptr = argv[optind++]; *ptr;ptr++);
	ptr = argv[optind - 1];
	if (*ptr != '/') {
		if (!stralloc_copys(&context_file, controldir) ||
				!stralloc_cats(&context_file, "/"))
			die_nomem();
	}
	if (!stralloc_cats(&context_file, ptr) ||
			!stralloc_0(&context_file))
		die_nomem();
	load_context();
#if defined(LIBC_HAS_IP6) && defined(IPV6)
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY EAI_FAMILY
#endif
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
#if defined(LIBC_HAS_IP6)
		if ((inaddr = inet_addr(ipaddr)) != INADDR_NONE) {
			sin.sa4.sin_port = htons(port);
			byte_copy((char *) &sin.sa4.sin_addr, 4, (char *) &inaddr);
			if (bind(s, (struct sockaddr *) &sin.sa4, sizeof(sin)) == -1)
				strerr_die6sys(111, FATAL, "bind4: ", ipaddr, ":", a_port, ": ");
		} else {
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_DGRAM;
			if ((len = getaddrinfo(ipaddr, a_port, &hints, &res0))) {
				freeaddrinfo(res0);
				errno = EINVAL;
				strerr_die7x(111, FATAL, "getadrinfo: ", ipaddr, ": ", a_port, ":", (char *) gai_strerror(len));
			}
			for (res = res0; res; res = res->ai_next) {
				if (bind(s, res->ai_addr, res->ai_addrlen) == 0)
					break;
			}
			freeaddrinfo(res0);
		}
#else
		if ((inaddr = inet_addr(ipaddr)) != INADDR_NONE) {
			sin.sa4.sin_port = htons(port);
			byte_copy((char *) &sin.sa4.sin_addr, 4, (char *) &inaddr);
		} else {
			if (!(hp = gethostbyname(ipaddr)))
				strerr_die5x(111, FATAL, "gethostbyname: ", ipaddr, ": ", (char *) hstrerror(h_errno));
			byte_copy((char *) &sin.sa4.sin_addr, hp->h_length, hp->h_addr);
		}
		if (bind(s, (struct sockaddr *) &sin.sa4, sizeof(sin)) == -1)
			strerr_die6sys(111, FATAL, "bind4: ", ipaddr, ":", a_port, ": ");
#endif
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
		/*
		 * Keep on incrementing bufsize till it is
		 * possible to fetch the entire message
		 * in one operation. This will allow the
		 * client to send the enter rcpt list in
		 * one operation.
		 *
		 * buf_len < bufsize - increase buffer size if packet cannot be read in
		 *                     one operation
		 * buf_len / bufsize - decrease buffer size if allocated size is more than 2
		 *                     times of required size
		 */
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
		n = MAXGREYDATASIZE;
#endif
		fromlen = sizeof(from);
		if ((n = recvfrom(s, rdata, n, 0, (struct sockaddr *) &from.sa, (socklen_t *)&fromlen)) == -1) {
			if (errno == error_intr)
				continue;
			strerr_die2sys(111, FATAL, "recvfrom: ");
		}
		save = 1;
		if (verbose) {
			out("qmail-greyd IP: ", NULL);
#if defined(LIBC_HAS_IP6) && defined(IPV6)
			if (noipv6)
				out(inet_ntoa(from.sa4.sin_addr), NULL);
			else {
				static char     addrBuf[INET6_ADDRSTRLEN];
				if (from.sa.sa_family == AF_INET) {
					out((char *) inet_ntop(AF_INET,
						(void *) &from.sa4.sin_addr, addrBuf,
						INET_ADDRSTRLEN), NULL);
				} else
				if (from.sa.sa_family == AF_INET6) {
					out((char *) inet_ntop(AF_INET6,
						(void *) &from.sa6.sin6_addr, addrBuf,
						INET6_ADDRSTRLEN), NULL);
				} else
				if (from.sa.sa_family == AF_UNSPEC)
					out("::1", NULL);
			}
#else
			out(inet_ntoa(from.sa4.sin_addr), NULL);
#endif
		}
		/*-
		 * greylist(3) protocol
		 * packet structure -
		 * I192.168.2.0\0Fmbhangui@gmail.com\0Tpostmaster@indimail.org\0root@indimail.org\0\0
		 */
		switch (rdata[0])
		{
		case 'I': /*- process exactly like greydaemon */
			out(" [", NULL);
			for (state = 0, ptr = rdata, rcpt_head = (char *) 0, rcptlen = 0;ptr < rdata + n - 1;) {
				switch (*ptr)
				{
					case 'I':
						out("Remote IP: ", NULL);
						client_ip = ptr + 1;
						state++;
						break;
					case 'F':
						out("FROM: ", NULL);
						rpath = ptr + 1;
						state++;
						break;
					case 'T':
						out("RCPT: ", NULL);
						if (!rcpt_head)
							rcpt_head = ptr;
						break;
				} /*- switch (*ptr) */
				out(ptr + 1, NULL);
				len = str_len(ptr) + 1;
				if (*ptr == 'T')
					rcptlen += len;
				ptr += len; /*- skip past \0 */
				if (ptr < (rdata + n - 2))
					out(" ", NULL);
				if (ptr == rdata + n)
					state = 0;
			} /*- for (ptr = rdata;ptr < rdata + n - 1;) */
			strnum[fmt_ulong(strnum, (unsigned long) n)] = 0;
			out("] bufsiz=", strnum, ", rcptlen=", NULL);
			strnum[fmt_ulong(strnum, (unsigned long) rcptlen)] = 0;
			out(strnum, NULL);
			if (state != 2 || !rcptlen) {
				out(", Response: RECORD INVALID\n", NULL);
				flush();
				break;
			} else
				n = send_response(s, &from, fromlen, client_ip, rpath, rcpt_head, rcptlen,
					min_resend, resend_window, free_interval, &grey, &rec_added);
			if (n == -2)
				logerr("qmail-greyd: invalid send_response - report bug to author\n", NULL);
			else
			if (n == -3)
				logerr("qmail-greyd: invalid IP address format\n", NULL);
			else
			if (n == -4)
				logerr("qmail-greyd: copy failed\n", NULL);
			else
			if (n < 0)
				strerr_warn2(WARN, "sendto failed: ", &strerr_sys);
			logflush();
			if (grey) {
				strnum[fmt_ulong(strnum, (unsigned long) grey->attempts)] = 0;
				out(", attempts=", strnum, ", Reponse: ", print_status(grey->status), "\n", NULL);
			} else
				out(", Reponse: ", n == -3 ? print_status(RECORD_IPV6) : print_status(RECORD_WHITE), "\n", NULL);
			flush();
			if (rec_added && grey && create_hash(grey)) /*- add the record to hash list if new. group on ip_str if not new */
				die_nomem();
			break;
		/*-
		 * maybe we should fork here
		 * This cases are left for future
		 * implentation to manipulate in-memory
		 * database of triplet and whitelisted IPs
		 */
		case 'a': /*- add a greylist  ip */
			break;
		case 'A': /*- add a whitelist ip */
			break;
		case 'm': /*- modify a greylist  ip */
			break;
		case 'M': /*- modify a whitelist ip */
			break;
		case 'd': /*- delete a greylist  ip */
			break;
		case 'D': /*- delete a whitelist ip */
			break;
		case 'q': /*- query a greylist  record */
			break;
		case 'Q': /*- query a whitelist record */
			break;
		default:
			out(", Response: RECORD INVALID\n", NULL);
			break;
		} /*- switch (rdata[0]) */
		flush();
	} /*- for (buf_len = 0, rdata_len = 0;;) { */
	return (0);
}

void
getversion_qmail_greyd_c()
{
	const char     *x = "$Id: qmail-greyd.c,v 1.37 2024-02-08 22:02:15+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: qmail-greyd.c,v $
 * Revision 1.37  2024-02-08 22:02:15+05:30  Cprogrammer
 * fix potential use after free()
 *
 * Revision 1.36  2023-07-07 10:40:56+05:30  Cprogrammer
 * use NULL instead of 0 for null pointer
 *
 * Revision 1.35  2023-01-15 23:14:40+05:30  Cprogrammer
 * logerr(), out() changed to have varargs
 *
 * Revision 1.34  2023-01-03 23:55:08+05:30  Cprogrammer
 * removed __USE_GNU
 *
 * Revision 1.33  2022-10-30 20:22:09+05:30  Cprogrammer
 * replaced cdb_match() with cdb_matchaddr() in cdb_match.c
 *
 * Revision 1.32  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.31  2021-06-12 18:07:02+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.30  2020-09-16 19:04:22+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.29  2020-07-04 21:43:34+05:30  Cprogrammer
 * removed usage of INET6 define
 *
 * Revision 1.28  2018-05-30 23:26:12+05:30  Cprogrammer
 * moved noipv6 variable to variables.c
 *
 * Revision 1.27  2018-05-29 22:11:07+05:30  Cprogrammer
 * removed call to gethostbyname() in ipv6 code
 *
 * Revision 1.26  2018-04-26 00:31:09+05:30  Cprogrammer
 * fixed verbosity of log messages
 *
 * Revision 1.25  2018-04-25 22:48:58+05:30  Cprogrammer
 * fixed display of options and verbosity of messages
 *
 * Revision 1.24  2017-11-26 12:58:39+05:30  Cprogrammer
 * fixed expire logic
 *
 * Revision 1.23  2017-11-25 18:34:49+05:30  Cprogrammer
 * initialize h_allocated. Fixed signal handler for SIGUSR2, expire records when loading context file
 *
 * Revision 1.22  2017-11-24 02:58:37+05:30  Cprogrammer
 * added verbose variable to suppress few messages
 *
 * Revision 1.21  2017-11-22 22:38:13+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.20  2017-11-21 18:49:24+05:30  Cprogrammer
 * added documentation
 *
 * Revision 1.19  2017-10-28 11:39:46+05:30  Cprogrammer
 * fixed search record failure when linked list contained just 1 entry
 *
 * Revision 1.18  2017-04-16 19:52:13+05:30  Cprogrammer
 * fixed hsearch() logic
 *
 * Revision 1.17  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.16  2016-04-15 15:45:14+05:30  Cprogrammer
 * set noipv6 if LIBC_HAS_IP6 is not defined
 *
 * Revision 1.15  2016-04-10 22:20:14+05:30  Cprogrammer
 * fixed incorrect formatting of context filename
 *
 * Revision 1.14  2016-04-10 13:07:56+05:30  Cprogrammer
 * added missing flush() statement to flush logs
 *
 * Revision 1.13  2016-04-03 09:38:31+05:30  Cprogrammer
 * port was being used non-initialized for non-ipv6 code
 *
 * Revision 1.12  2015-12-31 08:35:41+05:30  Cprogrammer
 * added IPV6 code
 *
 * Revision 1.11  2015-08-24 19:07:18+05:30  Cprogrammer
 * replace ip_scan() with ip4_scan(), replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.10  2011-07-29 09:29:30+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.9  2010-07-21 09:13:47+05:30  Cprogrammer
 * use CONTROLDIR environment variable instead of a hardcoded control directory
 *
 * Revision 1.8  2009-10-28 13:34:51+05:30  Cprogrammer
 * fix segmentation fault due to free()
 *
 * Revision 1.7  2009-09-07 13:58:02+05:30  Cprogrammer
 * fix compilation if USE_HASH was not defined
 *
 * Revision 1.6  2009-09-07 10:20:26+05:30  Cprogrammer
 * use select to perform non-blocking recvfrom()
 *
 * Revision 1.5  2009-09-05 23:10:13+05:30  Cprogrammer
 * added option to use hash for faster search
 *
 * Revision 1.4  2009-09-03 20:53:16+05:30  Cprogrammer
 * fixed logic for expiring records
 *
 * Revision 1.3  2009-09-01 22:01:51+05:30  Cprogrammer
 * fixed problem with format of context file
 *
 * Revision 1.2  2009-08-31 12:59:46+05:30  Cprogrammer
 * changed member 'ip' in struct greylst to ip_addr
 * fix allocation bug (changed realloc to malloc in copy_grey)
 * speed improvements in search_record
 *
 * Revision 1.1  2009-08-29 20:42:59+05:30  Cprogrammer
 * Initial revision
 *
 */
