/*
 * $Log: qmail-greyd.c,v $
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
 * fix compilation if USE_HAS was not defined
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
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include "sgetopt.h"
#include "auto_qmail.h"
#include "ip.h"
#include "sig.h"
#include "getln.h"
#include "fmt.h"
#include "str.h"
#include "subfd.h"
#include "byte.h"
#include "strerr.h"
#include "scan.h"
#include "env.h"
#include "stralloc.h"
#include "constmap.h"
#include "control.h"
#include "error.h"
#include "uint32.h"
#include "open.h"
#include "cdb.h"
#include "tablematch.h"
#ifndef NETQMAIL /*- netqmail does not have configurable control directory */
#include "variables.h"
#endif
#include "greylist.h"

#define FATAL "qmail-greyd: fatal: "
#define WARN  "qmail-greyd: warning: "

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

#ifdef USE_HASH
#include <search.h>
#define BLOCK_SIZE 32768
#endif

struct greylst
{
	ip_addr         ip;
#ifdef USE_HASH
	char            ip_str[16];
#endif
	char           *rpath;
	char           *rcpt; /* Trcpt1\0Trcpt2\0.....Trcptn\0 */
	unsigned int    rcptlen;
	time_t          timestamp;
	int             attempts;
	char            status;
	struct greylst *prev, *next;
#ifdef USE_HASH
	struct greylst *ip_prev, *ip_next;
#endif
};
struct greylst *head;
struct greylst *tail;
int             grey_count, hcount;
#ifdef USE_HASH
int             hash_size, h_allocated = 0;
#endif
unsigned long   timeout;
char           *whitefn = 0;
stralloc        context_file = { 0 };

struct netspec
{
	unsigned int    min;
	unsigned int    max;
};

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

/*
 * Convert Classless Inter-Domain Routing addresses into range
 * Range can filled in struct netspec
 */
int
cidr2IPrange(char *ipaddr, int mask, struct netspec *spec)
{
	ip_addr         ip;

	if (!ip4_scan(ipaddr, &ip))
	{
		logerr("failed to scan ip");
		logerr("[");
		logerr(ipaddr);
		logerrf("\n");
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
whitelist_init(char *arg)
{
	out("initializing whitelist\n");
	flush();
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
#endif
		die_control(arg);
	if (whitelistok && !constmap_init(&mapwhite, whitelist.s, whitelist.len, 0))
		die_nomem();
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
#ifdef NETQMAIL
	if (!stralloc_copys(&controlfile, "control"))
#else
	if (!stralloc_copys(&controlfile, controldir))
#endif
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
		/*
		 * cdb missing or entry missing
		 */
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
ip_match(char *fn, stralloc *ipaddr, stralloc *content, struct constmap *ptrmap,
	char **errStr)
{
	int             x, len, mask;
	struct netspec  netspec;
	unsigned int    tmp_ip;
	ip_addr         ip;
	char           *ptr;

	if (errStr)
		*errStr = 0;
	if (fn && (x = cdb_match(fn, ipaddr->s, ipaddr->len - 1)))
		return (x);
	else
	if (ptrmap && constmap(ptrmap, ipaddr->s, ipaddr->len - 1))
		return 1;
	for (len = 0, ptr = content->s;len < content->len;) {
		x = str_chr(ptr, '/');
		if (ptr[x]) {
			ptr[x] = 0;
			scan_int(ptr + 1, &mask);
			if (cidr2IPrange(ptr, mask, &netspec) == -1) {
				if (errStr) 
					*errStr = error_str(errno);
				ptr[x] = '/';
				logerr("invalid IP in CIDR format: ");
				logerr(ptr);
				logerrf("\n");
				return (-1);
			}
			ptr[x] = '/';
			if (!ip4_scan(ipaddr->s, &ip)) /*- record contains invalid IP */
			{
				logerr("failed to scan ip");
				logerr("[");
				logerr(ipaddr->s);
				logerrf("\n");
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
is_white(char *ip)
{
	char           *errStr = 0;
	static stralloc ipaddr = { 0 };

	if (!stralloc_copys(&ipaddr, ip))
		die_nomem();
	if (!stralloc_0(&ipaddr))
		die_nomem();
	switch (ip_match("whitelist.cdb", &ipaddr, whitelistok ? &whitelist : 0, 
			whitelistok ? &mapwhite : 0, &errStr)) {
	case 1:
		return (1);
	case 0:
		return (0);
	default:
		substdio_flush(subfdout);
		logerr("error in ip_match: ");
		logerr(errStr);
		logerrf("\n");
		_exit (1);
	}
	/*- Not reached */
	return (0);
}

int
copy_grey(struct greylst *ptr, char *ipaddr, char *rpath, char *rcpt, int rcptlen)
{
	int             len;

	if (!ip4_scan(ipaddr, &ptr->ip))
	{
		logerr("failed to scan ip");
		logerr("[");
		logerr(ipaddr);
		logerrf("\n");
		return (-1);
	}
	len = str_len(rpath);
	if (!(ptr->rpath = (char *) malloc(len + 1)))
		die_nomem();
	if (str_copy(ptr->rpath, rpath) != len)
	{
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
print_record(char *ip, char *rpath, char *rcpt, int rcptlen, time_t timestamp,
	char status, int operation)
{
	char            strnum[FMT_ULONG];
	char           *ptr;

	strnum[fmt_ulong(strnum, (unsigned long) timestamp)] = 0;
	out(strnum);
	out(" IP: ");
	out(ip);
	out(" FROM: ");
	out(rpath); /*- from */
	out(" RCPT: [");
	for (ptr = rcpt;ptr < rcpt + rcptlen;) {
		out(ptr + 1);
		ptr += (str_len(ptr) + 1);
		if (ptr < rcpt + rcptlen)
			out(" ");
		else
			out("]");
	}
	out(" status=");
	strnum[fmt_ulong(strnum, (unsigned long) status)] = 0;
	out(strnum);
	out(" rcptlen=");
	strnum[fmt_ulong(strnum, (unsigned long) rcptlen)] = 0;
	out(strnum);
	out(operation ? " expired\n" : "\n");
	return;
}

int
compare_ip(unsigned char *ip1, unsigned char *ip2)
{
	register int    i;

	for (i = 0;i < 4;i++)
	{
		if (ip1[i] != ip2[i])
			return (ip1[i] < ip2[i] ? -1 : 1);
	}
	return (0);
}

#ifdef USE_HASH
int
create_hash(struct greylst *curr)
{
	struct greylst *ip_ptr, *ptr;
	ENTRY           e, *ep;
	char            strnum[FMT_ULONG];

	if (hash_size <= 0)
		return (0);
	if (!curr) {
		hdestroy();
		hcount = 0;
	}
	if (!(hcount % hash_size)) {
		if (h_allocated++)
		{
			hdestroy();
			hcount = 0;
			curr = 0;
			logerr("WARNING!! recreating hash, size=");
		} else
			logerr("creating hash, size=");
		strnum[fmt_ulong(strnum, (unsigned long) (2 * hash_size * h_allocated))] = 0;
		logerr(strnum);
		logerrf("\n");
		if (!hcreate(h_allocated * (2 * hash_size)))
			strerr_die2sys(111, FATAL, "unable to create hash table: ");
	}
	for (ptr = (curr ? curr : head);ptr;ptr = ptr->next) {
		e.key = ptr->ip_str;
		if (!(ep = hsearch(e, FIND))) {
			e.data = ptr;
			if (!(ep = hsearch(e, ENTER))) {
				logerrf("unable to add to hash table\n");
				return (-1);
			} else
				hcount++;
		} else {
			for (ip_ptr = (struct greylst *) ep->data;ip_ptr && ip_ptr->ip_next;ip_ptr = ip_ptr->ip_next);
			if (!ip_ptr->ip_next) {
				ip_ptr->ip_next = ptr;
				ptr->ip_next = 0;
				ptr->ip_prev = ip_ptr;
			}
		}
		if (curr)
			break;
	}
	return (0);
}
#endif

void
expire_records(time_t cur_time)
{
	struct greylst *ptr;
	time_t          start;
	char            ip_str[16];

	out("expiring records\n");
	flush();
	/*- find the first record that is not expired */
	start = cur_time - timeout;
	for (ptr = head;ptr && ptr->timestamp < start;ptr = ptr->next) {
		ip_str[ip4_fmt(ip_str, &ptr->ip)] = 0;
		print_record(ip_str, ptr->rpath, ptr->rcpt, ptr->rcptlen, ptr->timestamp,
			ptr->status, 1);
		grey_count--;
		head = ptr->next;
		head->prev = 0;
		free(ptr->rpath);
		free(ptr->rcpt);
		free(ptr);
	}
#ifdef USE_HASH
	if (hash_size > 0 && create_hash(0))
		die_nomem();
#endif
	return;
}

int
seq_search(ip_addr r_ip, struct greylst *ptr, char *rpath, char *rcpt, int rcptlen,
	int min_resend, int resend_win, time_t cur_time, struct greylst **store)
{
	int             rpath_len;

	for (rpath_len = str_len(rpath);;) { /*- sequential search */
		if (!compare_ip(ptr->ip.d, r_ip.d) && !str_diffn(ptr->rpath, rpath, rpath_len)
			&& (rcptlen == ptr->rcptlen && !byte_diff(ptr->rcpt, ptr->rcptlen, rcpt)))
		{ /*- found */
			*store = ptr;
			ptr->attempts++;
			/*- # not older than timeout days */
			if ((ptr->status == RECORD_GREY || ptr->status == RECORD_OK)
				&& (ptr->timestamp > cur_time - timeout))
			{
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
		if (!ptr->next)
			break;
		ptr = ptr->next;
	} /* for (ip_len = str_len(ip), rpath_len = str_len(rpath);;) { */
	return (RECORD_NEW);
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
search_record(char *remoteip, char *rpath, char *rcpt, int rcptlen, int min_resend,
	int resend_win, int fr_int, struct greylst **store)
{
	struct greylst *ptr;
	time_t          cur_time, start;
	ip_addr         r_ip;
#ifdef USE_HASH
	ENTRY           e, *ep;
#else
	int             rpath_len;
#endif

	*store = (struct greylst *) 0;
	if (whitelistok && is_white(remoteip))
		return (RECORD_WHITE);
	if (!head)
		return (RECORD_NEW);
	cur_time = time(0);
	if (cur_time - timeout > head->timestamp) {
		/*- records waiting to be expired */
		if (!(cur_time % fr_int)) {
			/*-expire records */
			expire_records(cur_time);
			if (!head)
				return (RECORD_NEW);
			ptr = head;
		} else {
			start = cur_time - timeout;
			for (ptr = head;ptr && ptr->timestamp < start; ptr = ptr->next);
			if (!ptr)
				return (RECORD_NEW);
		}
	} else
		ptr = head;
	if (!ip4_scan(remoteip, &r_ip))
	{
		logerr("failed to scan ip");
		logerr("[");
		logerr(remoteip);
		logerrf("\n");
		return (0);
	}
#ifdef USE_HASH
	if (!hash_size)
		return (seq_search(r_ip, ptr, rpath, rcpt, rcptlen, min_resend, resend_win, cur_time, store));
	e.key = remoteip;
	if (!(ep = hsearch(e, FIND)))
		return (RECORD_NEW);
	ptr = (struct greylst *) ep->data;
	if (!compare_ip(ptr->ip.d, r_ip.d) && !str_diffn(ptr->rpath, rpath, str_len(rpath))
		&& (rcptlen == ptr->rcptlen && !byte_diff(ptr->rcpt, ptr->rcptlen, rcpt)))
	{ /*- found */
		*store = ptr;
		ptr->attempts++;
		/*- # not older than timeout days */
		if ((ptr->status == RECORD_GREY || ptr->status == RECORD_OK)
			&& (ptr->timestamp > cur_time - timeout))
		{
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
#else
	return (seq_search(r_ip, ptr, rpath, rcpt, rcptlen, min_resend, resend_win, cur_time, store));
#endif
}

struct greylst *
add_record(char *ip, char *rpath, char *rcpt, int rcptlen, struct greylst **grey)
{
	struct greylst *ptr;

	if (!head) {
		if (!(ptr = (struct greylst *) malloc(sizeof(struct greylst))))
			die_nomem();
#ifdef USE_HASH
		ptr->prev = ptr->next = ptr->ip_next = ptr->ip_prev = 0;
#else
		ptr->prev = ptr->next = 0;
#endif
		head = tail = ptr;
	} else {
		if (!(ptr = (struct greylst *) malloc(sizeof(struct greylst))))
			die_nomem();
		ptr->prev = tail;
		tail->next = ptr;
		tail = ptr;
#ifdef USE_HASH
		ptr->ip_prev = ptr->ip_next = 0;
#endif
	}
#ifdef USE_HASH
	if (hash_size > 0) {
		byte_copy(ptr->ip_str, str_len(ip) + 1, ip);
		if (create_hash(ptr))
			die_nomem();
	}
#endif
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
send_response(int s, struct sockaddr_in *from, int fromlen, char *ip, char *rpath,
	char *rcpt, int rcptlen, int min_resend, int resend_win, int fr_int,
	struct greylst **grey)
{
	char           *resp;
	int             i, n = 0;
	unsigned char   ibuf[sizeof(struct in6_addr)];

	*grey = (struct greylst *) 0;
	if ((i = inet_pton(AF_INET6, ip, ibuf)) == 1) {
		resp = "\1\4";
		n = -3; /*- inet6 address */
	} else
	if (i == -1)
	{
		resp = "\0\4";
		n = -1; /*- system error */
	} else /*- Not in ip6 presentation format */
	switch (search_record(ip, rpath, rcpt, rcptlen, min_resend, resend_win, fr_int, grey))
	{
	case RECORD_NEW:
		if (!add_record(ip, rpath, rcpt, rcptlen, grey))
		{
			logerrf("unable to add record\n");
			resp = "\0\4";
			n = -4;
		} else
			resp = "\0\1";
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

void
save_context()
{
	struct greylst *ptr;
	int             context_fd;
	char            ip_str[16];
	char            rcptlen[FMT_ULONG], timestamp[FMT_ULONG];

	if (!grey_count)
		return;
	out("saving context file\n");
	flush();
	if ((context_fd = open(context_file.s, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		strerr_die4sys(111, FATAL, "unable to open file: ", context_file.s, ": ");
	for (ptr = head;ptr;ptr = ptr->next)
	{
		rcptlen[fmt_ulong(rcptlen, ptr->rcptlen)] = 0;
		timestamp[fmt_ulong(timestamp, ptr->timestamp)] = 0;
		if (write_file(context_fd, ip_str, ip4_fmt(ip_str, &ptr->ip)) == -1
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
	return;
}

stralloc        context = { 0 };
stralloc        line = { 0 };

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

	out("loading context\n");
	flush();
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
			flush();
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
		if (timestamp < cur_time - timeout) {
			print_record(ip, rpath, rcpt, rcptlen, timestamp, status, 1);
			continue;
		} else
			print_record(ip, rpath, rcpt, rcptlen, timestamp, status, 0);
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
#ifdef USE_HASH
		if (hash_size > 0) {
			byte_copy(ptr->ip_str, str_len(ip) + 1, ip);
			if (create_hash(ptr))
				die_nomem();
		}
#endif
		grey_count++;
		ptr->timestamp = timestamp;
		ptr->status = status;
		if (copy_grey(ptr, ip, rpath, rcpt, rcptlen)) {
			flush();
			free(ptr);
			return;
		}
		if (!match) {
			flush();
			close(fd);
			return;
		}
	} /*- for (;;) { */
	flush();
	close(fd);
	return;
}

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
sigusr2()
{
	sig_block(SIGUSR1);
	expire_records(time(0));
	sig_unblock(SIGUSR1);
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
	switch (status) {
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
char           *usage =
				"usage: qmail-greyd [options] ipaddr context_file\n"
				"Options [ wstgmf ]\n"
				"        [ -w whitelist ]\n"
				"        [ -t timeout (days) ]\n"
				"        [ -g resend window (hours) ]\n"
				"        [ -m minimum resend time (min) ]\n"
				"        [ -f free interval (min) ]\n"
				"        [ -s save interval (min) ]";
int
main(int argc, char **argv)
{
	int             s, buf_len, rdata_len, n, port, opt, fromlen, rcptlen, len;
	struct sockaddr_in sin, from;
	struct hostent *hp;
	struct greylst *grey;
	unsigned long   resend_window, min_resend, save_interval, free_interval;
	char           *ptr, *ipaddr = 0, *client_ip = 0, *rpath = 0, *rcpt_head = 0;
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
#ifdef USE_HASH
	hash_size = BLOCK_SIZE;
	while ((opt = getopt(argc, argv, "w:s:t:g:m:h:")) != opteof) {
#else
	while ((opt = getopt(argc, argv, "w:s:t:g:m:")) != opteof) {
#endif
		switch (opt) {
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
#ifdef USE_HASH
		case 'h':
			scan_int(optarg, &hash_size);
			break;
#endif
		case 'm':
			scan_ulong(optarg, &min_resend);
			min_resend *= 60; /*- convert to seconds */
			break;
		case 'f':
			scan_ulong(optarg, &free_interval);
			free_interval *= 60; /*- convert to seconds */
			break;
		default:
			strerr_die1x(100, usage);
			break;
		}
	} /*- while ((opt = getopt(argc, argv, "dw:s:t:g:m:")) != opteof) */
	if (optind + 2 != argc)
		strerr_die1x(100, usage);
	ipaddr = argv[optind++];
	if (*ipaddr == '*')
		ipaddr = INADDR_ANY;
	for (ptr = ipaddr, port = 1999;*ptr;ptr++) {
		if (*ptr == ':') {
			*ptr = 0;
			scan_int(ptr + 1, &port);
			break;
		}
	}
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir: ", auto_qmail, ": ");
#ifndef NETQMAIL
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = "control";
#endif
	if (whitefn) {
		whitelist_init(whitefn);
		sig_catch(SIGHUP, sighup);
	}
#ifndef NETQMAIL
	if (!stralloc_copys(&context_file, controldir))
#else
	if (!stralloc_copys(&context_file, "control"))
#endif
		die_nomem();
	if (!stralloc_cats(&context_file, "/"))
		die_nomem();
	if (!stralloc_cats(&context_file, argv[optind++]))
		die_nomem();
	if (!stralloc_0(&context_file))
		die_nomem();
	load_context();
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		strerr_die2sys(111, FATAL, "unable to create socket: ");
	if (dup2(s, 0))
		strerr_die2sys(111, FATAL, "unable to dup socket: ");
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if ((inaddr = inet_addr(ipaddr)) != INADDR_NONE)
		byte_copy((char *) &sin.sin_addr, 4, (char *) &inaddr);
	else {
		if (!(hp = gethostbyname(ipaddr))) {
			errno = EINVAL;
			strerr_die4sys(111, FATAL, "gethostbyname: ", ipaddr, ": ");
		} else
			byte_copy((char *) &sin.sin_addr, hp->h_length, hp->h_addr);
	}
	if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) == -1)
		strerr_die6sys(111, FATAL, "gethostbyname: ", ipaddr, ":", ptr + 1, ": ");
	sig_catch(SIGTERM, sigterm);
	sig_catch(SIGUSR1, sigusr1);
	sig_catch(SIGUSR2, sigusr2);
	out("Ready for connections, grey_count=");
	strnum[fmt_ulong(strnum, (unsigned long) grey_count)] = 0;
	out(strnum);
#ifdef USE_HASH
	if (hash_size > 0) {
		out(", hcount=");
		strnum[fmt_ulong(strnum, (unsigned long) hcount)] = 0;
		out(strnum);
	}
	out(", hash_size=");
	strnum[fmt_ulong(strnum, (unsigned long) (h_allocated * 2 * hash_size))] = 0;
	out(strnum);
#endif
	out("\n");
	flush();
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
		if ((n = recvfrom(s, rdata, n, 0, (struct sockaddr *) &from, (socklen_t *)&fromlen)) == -1) {
			if (errno == error_intr)
				continue;
			strerr_die2sys(111, FATAL, "recvfrom: ");
		}
		save = 1;
		out("qmail-greyd IP: ");
		out(inet_ntoa(from.sin_addr));
		/*- 
		 * greylist(3) protocol
		 * packet structure -
		 * I192.168.2.0\0Fmbhangui@gmail.com\0Tpostmaster@indimail.org\0root@indimail.org\0\0
		 */
		switch (rdata[0])
		{
		case 'I': /*- process exactly like greydaemon */
			out(" [");
			for (ptr = rdata, rcpt_head = (char *) 0, rcptlen = 0;ptr < rdata + n - 1;) {
				switch (*ptr) {
					case 'I':
						out("Remote IP: ");
						client_ip = ptr + 1;
						break;
					case 'F':
						out("FROM: ");
						rpath = ptr + 1;
						break;
					case 'T':
						out("RCPT: ");
						if (!rcpt_head)
							rcpt_head = ptr;
						break;
				} /*- switch (*ptr) */
				out(ptr + 1);
				len = str_len(ptr) + 1;
				if (*ptr == 'T')
					rcptlen += len;
				ptr += len;
				if (ptr < (rdata + n - 2))
					out(" ");
			} /*- for (ptr = rdata;ptr < rdata + n - 1;) */
			out("] bufsiz=");
			strnum[fmt_ulong(strnum, (unsigned long) n)] = 0;
			out(strnum);
			out(", rcptlen=");
			strnum[fmt_ulong(strnum, (unsigned long) rcptlen)] = 0;
			out(strnum);
			n = send_response(s, &from, fromlen, client_ip, rpath, rcpt_head, rcptlen,
					min_resend, resend_window, free_interval, &grey);
			if (n == -2) {
				logerrf("qmail-greyd: invalid send_response - report bug to author\n");
			} else
			if (n == -3) {
				logerrf("qmail-greyd: invalid IP address format\n");
			} else
			if (n == -4) {
				logerrf("qmail-greyd: copy failed\n");
			} else
			if (n < 0)
				strerr_warn2(WARN, "sendto failed: ", &strerr_sys);
			if (grey) {
				out(", attempts=");
				strnum[fmt_ulong(strnum, (unsigned long) grey->attempts)] = 0;
				out(strnum);
				out(", Reponse: ");
				out(print_status(grey->status));
			} else {
				out(", Reponse: ");
				n == -3 ? out(print_status(RECORD_IPV6)) : out(print_status(RECORD_WHITE));
			}
			out("\n");
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
			break;
		} /*- switch (rdata[0]) */
		flush();
	} /*- for (i = 0, j = 0;;) */
	return (0);
}

void
getversion_qmail_greyd_c()
{
	static char    *x = "$Id: qmail-greyd.c,v 1.10 2011-07-29 09:29:30+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
