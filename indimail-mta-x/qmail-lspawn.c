/*
 * $Id: qmail-lspawn.c,v 1.45 2023-10-03 22:48:03+05:30 Cprogrammer Exp mbhangui $
 */
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <fd.h>
#include <wait.h>
#include <prot.h>
#include <substdio.h>
#include <stralloc.h>
#include <scan.h>
#include <error.h>
#include <cdb.h>
#include <case.h>
#include <open.h>
#include <byte.h>
#include <env.h>
#include <str.h>
#include <fmt.h>
#include <setuserid.h>
#include "auto_assign.h"
#include "auto_control.h"
#include "auto_uids.h"
#include "auto_prefix.h"
#include "slurpclose.h"
#include "qlx.h"
#include "variables.h"
#include "indimail_stub.h"

static char    *aliasempty;
static int      local_user;
static stralloc q, user, pwstruct;
static char     strnum[FMT_ULONG];
#ifdef ENABLE_VIRTUAL_PKG
static stralloc save;
#endif

static char *setup_qlargs()
{
	static char    *qlargs;

	if (!qlargs)
		qlargs= env_get("QMAILLOCAL");
	if (!qlargs) {
		if (!stralloc_copys(&q, auto_prefix) ||
				!stralloc_catb(&q, "/sbin/qmail-local", 17) ||
				!stralloc_0(&q))
			_exit (QLX_NOMEM);
		qlargs = q.s;
	}
	return qlargs;
}

static char *setup_qgargs()
{
	static char    *qgargs;

	if (!qgargs)
		qgargs= env_get("QMAILGETPW");
	if (!qgargs) {
		if (!stralloc_copys(&q, auto_prefix) ||
				!stralloc_catb(&q, "/sbin/qmail-getpw", 17) ||
				!stralloc_0(&q))
			_exit (QLX_NOMEM);
		qgargs = q.s;
	}
	return qgargs;
}

void
initialize_SPAWN(int argc, char **argv)
{
	aliasempty = argv[1];
	if (!aliasempty)
		_exit (100);
}

int             truncreport_SPAWN = 3000;

void
report_SPAWN(substdio *ss, int wstat, char *s, int len)
{
	int             i;
	if (wait_crashed(wstat)) {
		substdio_puts(ss, "Zqmail-local crashed.\n");
		return;
	}
	switch (wait_exitcode(wstat))
	{
	case QLX_CDB:
		substdio_puts(ss, "ZTrouble reading users/cdb in qmail-lspawn.\n");
		return;
	case QLX_NOMEM:
		substdio_puts(ss, "ZOut of memory in qmail-lspawn.\n");
		return;
	case QLX_SYS:
		substdio_puts(ss, "ZTemporary failure in qmail-lspawn.\n");
		return;
	case QLX_NOALIAS:
		substdio_puts(ss, "ZUnable to find alias user!\n");
		return;
	case QLX_ROOT:
		substdio_puts(ss, "ZNot allowed to perform deliveries as root.\n");
		return;
	case QLX_DIR:
		substdio_puts(ss, "ZUnable to cd to root directory.\n");
		return;
	case QLX_USAGE:
		substdio_puts(ss, "ZInternal qmail-lspawn bug.\n");
		return;
	case QLX_NFS:
		substdio_puts(ss, "ZNFS failure in qmail-local.\n");
		return;
	case QLX_EXECHARD:
		substdio_puts(ss, "DUnable to run qmail-local.\n");
		return;
	case QLX_EXECSOFT:
		substdio_puts(ss, "ZUnable to run qmail-local.\n");
		return;
	case QLX_EXECPW:
		substdio_puts(ss, "ZUnable to run qmail-getpw.\n");
		return;
	case QLX_UIDGID:
		substdio_puts(ss, "ZUnable to set proper uid, gid");
		if (user.len) {
			substdio_puts(ss, " for ");
			substdio_put(ss, user.s, user.len - 1);
			substdio_put(ss, ".\n", 2);
		} else
			substdio_put(ss, ".\n", 2);
		return;
	case 111:
	case 71:
	case 74:
	case 75:
		substdio_put(ss, "Z", 1);
		break;
	case 0:
		substdio_put(ss, "K", 1);
		break;
	case 100:
	default:
		substdio_put(ss, "D", 1);
		break;
	}
	for (i = 0; i < len; ++i) {
		if (!s[i])
			break;
	}
	substdio_put(ss, s, i);
}

static stralloc lower = { 0 };
static stralloc nughde = { 0 };
static stralloc wildchars = { 0 };
static stralloc cdbfile = { 0 };
#ifdef ENABLE_VIRTUAL_PKG
static stralloc libfn = { 0 };
#endif

char           *cdbdir;

/*
 * nughde - name, uid, gid, homedir, dash, ext
 */
void
nughde_get(char *local)
{
	char           *(args[3]);
	char           *x;
	int             pi[2];
	int             gpwpid, gpwstat, r, fd, flagwild;

	local_user = 0;
	if (!cdbdir) {
		if (!(cdbdir = env_get("ASSIGNDIR")))
			cdbdir = auto_assign;
	}
	if (!stralloc_copys(&cdbfile, cdbdir) ||
			(cdbfile.s[cdbfile.len - 1] != '/' && !stralloc_cats(&cdbfile, "/")))
		_exit (QLX_NOMEM);
	if (!stralloc_catb(&cdbfile, "cdb", 4) ||
			!stralloc_copys(&lower, "!") ||
			!stralloc_cats(&lower, local) ||
			!stralloc_0(&lower))
		_exit (QLX_NOMEM);
	case_lowerb(lower.s, lower.len);
	if (!stralloc_copys(&nughde, ""))
		_exit (QLX_NOMEM);
	if ((fd = open_read(cdbfile.s)) == -1)
		if (errno != error_noent)
			_exit (QLX_CDB);
	if (fd != -1) {
		uint32          dlen;
		unsigned int    i;

		if ((r = cdb_seek(fd, "", 0, &dlen)) != 1)
			_exit (QLX_CDB);
		if (!stralloc_ready(&wildchars, (unsigned int) dlen))
			_exit (QLX_NOMEM);
		wildchars.len = dlen;
		if (cdb_bread(fd, wildchars.s, wildchars.len) == -1)
			_exit (QLX_CDB);
		i = lower.len;
		flagwild = 0;
		do {
			/*- i > 0 */
			if (!flagwild || (i == 1) || (byte_chr(wildchars.s, wildchars.len, lower.s[i - 1]) < wildchars.len)) {
				if ((r = cdb_seek(fd, lower.s, i, &dlen)) == -1)
					_exit (QLX_CDB);
				if (r == 1) {
					if (!stralloc_ready(&nughde, (unsigned int) dlen))
						_exit (QLX_NOMEM);
					nughde.len = dlen;
					if (cdb_bread(fd, nughde.s, nughde.len) == -1)
						_exit (QLX_CDB);
					if (flagwild)
						if (!stralloc_cats(&nughde, local + i - 1))
							_exit (QLX_NOMEM);
					if (!stralloc_0(&nughde))
						_exit (QLX_NOMEM);
					close(fd);
					return;
				}
			}
			--i;
			flagwild = 1;
		} while (i);
		close(fd);
	}
	if (pipe(pi) == -1)
		_exit (QLX_SYS);
	switch (gpwpid = vfork())
	{
	case -1:
		_exit (QLX_SYS);
	case 0:
		if (prot_gid(auto_gidn) == -1) {
			if (!stralloc_copys(&user, local) ||
					!stralloc_0(&user))
				_exit (QLX_NOMEM);
			_exit (QLX_UIDGID);
		}
		if (prot_uid(auto_uidp) == -1) {
			if (!stralloc_copys(&user, local) ||
					!stralloc_0(&user))
				_exit (QLX_NOMEM);
			_exit (QLX_UIDGID);
		}
		close(pi[0]);
		if (fd_move(1, pi[1]) == -1)
			_exit (QLX_SYS);
		x = setup_qgargs();
		args[0] = x;
		args[1] = local;
		args[2] = 0;
		execv(x, args);
		_exit (QLX_EXECPW);
	}
	close(pi[1]);
	if (slurpclose(pi[0], &nughde, 128) == -1)
		_exit (QLX_SYS);
	if (wait_pid(&gpwstat, gpwpid) != -1) {
		if (wait_crashed(gpwstat))
			_exit (QLX_SYS);
		if (wait_exitcode(gpwstat) != 0)
			_exit (wait_exitcode(gpwstat));
	}
	local_user = 1;
}

void
copy_pwstruct(struct passwd *pw, char *recip, int at, int is_inactive)
{
	if (!stralloc_copyb(&pwstruct, "PWSTRUCT=", 9) ||
			!stralloc_cats(&pwstruct, pw->pw_name) ||
			!stralloc_append(&pwstruct, "@") ||
			!stralloc_cats(&pwstruct, recip + at + 1) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_passwd) ||
			!stralloc_append(&pwstruct, ":"))
		_exit (-1);
	strnum[fmt_uint(strnum, pw->pw_uid)] = 0;
	if (!stralloc_cats(&pwstruct, strnum) ||
			!stralloc_append(&pwstruct, ":"))
		_exit (-1);
	strnum[fmt_uint(strnum, pw->pw_gid)] = 0;
	if (!stralloc_cats(&pwstruct, strnum) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_gecos) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_dir) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_shell) ||
			!stralloc_append(&pwstruct, ":"))
		_exit (-1);
	strnum[fmt_uint(strnum, is_inactive)] = 0;
	if (!stralloc_cats(&pwstruct, strnum) ||
			!stralloc_0(&pwstruct))
		_exit (-1);
	return;
}

int
SPAWN(int fdmess, int fdout, unsigned long msgsize, char *sender, char *qqeh, char *recip_t, int at_t)
{
	int             f, at = at_t;
	char           *ptr, *recip = recip_t;
#ifdef ENABLE_VIRTUAL_PKG
	/*- indimail */
	char           *libptr, *tptr;
	int             len;
	struct passwd  *pw;
	extern void    *phandle;
	void            (*iclose) (void);
	int             (*isvirtualdomain) (char *);
	int             (*iopen) (char *);
	void *          (*inquery) (char, char *, char *);
	int            *u_not_found, *i_inactive;
	struct passwd*  (*sql_getpw) (char *, char *);
#endif

	if (!env_unset("QMAILREMOTE"))
		_exit (-1);
#ifdef ENABLE_VIRTUAL_PKG
	/*- indimail */
	if (!env_get("AUTHSELF"))
		goto noauthself;
	if (!(libptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!libfn.len) {
			if (!stralloc_copys(&libfn, controldir) ||
					(libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/")) ||
					!stralloc_catb(&libfn, "libindimail", 11) ||
					!stralloc_0(&libfn))
				_exit (-1);
		}
		libptr = libfn.s;
	} else
		libptr = "VIRTUAL_PKG_LIB";
	loadLibrary(&phandle, libptr, &f, 0);
	if (f)
		_exit (-3);
	if (!phandle)
		goto noauthself;
	if (!(isvirtualdomain = getlibObject(libptr, &phandle, "isvirtualdomain", 0)) ||
			!(iopen = getlibObject(libptr, &phandle, "iopen", 0)) ||
			!(sql_getpw = getlibObject(libptr, &phandle, "sql_getpw", 0)) ||
			!(iclose = getlibObject(libptr, &phandle, "iclose", 0)) ||
			!(inquery = getlibObject(libptr, &phandle, "inquery", 0)))
		_exit (-3);
	/*-
	 * recip = example1-example2.com-some_user@example1-example2.com
	 * recip + at + 1 = example1-example2.com
	 */
	if (env_get("QUERY_CACHE")) {
		if ((*isvirtualdomain) (recip + at + 1)) {
			if (!env_unset("PWSTRUCT"))
				_exit (-1);
			f = str_len(recip + at + 1);
			if ((pw = (*inquery) (PWD_QUERY, recip + f + 1, 0))) {
				if (!(i_inactive = (int *) getlibObject(libptr, &phandle, "is_inactive", 0)))
					_exit (-3);
				copy_pwstruct(pw, recip, at, *i_inactive);
				if (!env_put(pwstruct.s))
					_exit (-1);
			}
		}
	} else
	if ((*isvirtualdomain) (recip + at + 1) && !(*iopen) ((char *) 0)) {
		if (!env_unset("PWSTRUCT"))
			_exit (-1);
		f = str_len(recip + at + 1); /*- domain length */
		for (len = 0, ptr = recip + f + 1; *ptr && *ptr != '@'; ptr++, len++);
		if (len) {
			if (!stralloc_copyb(&user, recip + f + 1, len) || /*- copy user portion */
					!stralloc_0(&user))
				_exit (-1);
		} else { /*- NULL user (double bounce) */
			if (!(ptr = env_get("ROUTE_NULL_USER")))
				goto noauthself;
			else
			if (!*ptr)
				goto noauthself;
			for (len = 0, tptr = ptr; *tptr && *tptr != '@'; tptr++, len++);
			if (!((*isvirtualdomain) (tptr + 1))) { /*- local domain */
				/*-
				 * delivery to email to local domain
				 * mailbox@domain
				 * copy mailbox@localdomain
				 */
				if (!stralloc_copys(&user, ptr) || !stralloc_0(&user))
					_exit (-1);
				recip = user.s;
				at = len;
				goto noauthself;
			}
			/*-
			 * delivery to email to virtual domain.
			 * convert mailbox@domain
			 *   to
			 * virtual_domain-mailbox@virtual_domain
			 */
			f = str_len(tptr + 1); /*- domain length */
			if (!stralloc_copyb(&save, tptr + 1, f) || /*- copy domain */
					!stralloc_append(&save, "-") ||
					!stralloc_catb(&save, ptr, len) || /*- copy user portion */
					!stralloc_catb(&save, tptr, f + 1) || /*- copy @domain */
					!stralloc_0(&save) ||
					!stralloc_copyb(&user, ptr, len) || /*- copy user portion */
					!stralloc_0(&user))
				_exit (-1);
			recip = save.s;
			at = f + 1 + len;
		}
		if ((pw = (struct passwd *) (*sql_getpw) (user.s, recip + at + 1))) {
			if (!(i_inactive = (int *) getlibObject(libptr, &phandle, "is_inactive", 0)))
				_exit (-3);
			copy_pwstruct(pw, recip, at, *i_inactive);
			if (!env_put(pwstruct.s))
				_exit (-1);
		} else {
			if (!(u_not_found = (int *) getlibObject(libptr, &phandle, "userNotFound", 0)))
				_exit (-3);
			if (*u_not_found) {
				if (!stralloc_copys(&pwstruct, "PWSTRUCT=No such user ") ||
						!stralloc_cats(&pwstruct, user.s) ||
						!stralloc_append(&pwstruct, "@") ||
						!stralloc_cats(&pwstruct, recip + at + 1) ||
						!stralloc_0(&pwstruct) ||
						!env_put(pwstruct.s))
					_exit (-1);
			} else {
				(*iclose) ();
				_exit (-2);
			}
		}
	} /*- if ((*isvirtualdomain) (recip_t + at_t + 1) && !(*iopen) ((char *) 0)) */
/*- end indimail */
noauthself: /*- deliver to local user in control/locals */
#endif
	if (!(f = fork())) {
		char           *(args[12]);
		unsigned long   u;
		int             n;
		uid_t           uid;
		gid_t           gid;
		char           *x;
		unsigned int    xlen;

#ifdef ENABLE_VIRTUAL_PKG
		closeLibrary(&phandle);
#endif
		recip[at] = 0;
		if (!recip[0])
			_exit (0);/*- <> */
		if (chdir("/") == -1)
			_exit (QLX_DIR);
		nughde_get(recip);
		x = nughde.s;
		xlen = nughde.len;
		if (!stralloc_copys(&q, auto_prefix) ||
				!stralloc_catb(&q, "/sbin/qmail-local", 17) ||
				!stralloc_0(&q))
			_exit (QLX_NOMEM);
		args[0] = q.s;
		args[1] = "--";
		args[2] = x; /*- n = user */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit (QLX_USAGE);
		x += n;
		xlen -= n;
		scan_ulong(x, &u);
		uid = u; /*- u = uid */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit (QLX_USAGE);
		x += n;
		xlen -= n;
		scan_ulong(x, &u);
		gid = u; /*- g = uid */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit (QLX_USAGE);
		x += n;
		xlen -= n;
		args[3] = x; /*- h = homedir */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit (QLX_USAGE);
		x += n;
		xlen -= n;
		args[4] = recip; /*- local */
		args[5] = x; /*- d = dash */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit (QLX_USAGE);
		x += n;
		xlen -= n;
		args[6] = x; /*- e = Ext */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit (QLX_USAGE);
		x += n;
		xlen -= n;
		args[7] = recip + at + 1;	/*- domain */
		args[8] = sender;			/*- Sender */
		args[9] = aliasempty;		/*- default-delivery */
		args[10] = qqeh;
		args[11] = 0;
		if (fd_move(0, fdmess) == -1 || fd_move(1, fdout) == -1 || fd_copy(2, 1) == -1)
			_exit (QLX_SYS);
		if (local_user == 1 && env_get("SETUSER_PRIVILEGES")) {
			if (setuser_privileges(uid, gid, args[2]) == -1) {
				if (!stralloc_copys(&user, args[2]) ||
						!stralloc_0(&user))
					_exit (QLX_NOMEM);
				_exit (QLX_UIDGID);
			}
		} else
		if (prot_gid(gid) == -1 || prot_uid(uid) == -1) {
			if (!stralloc_copys(&user, args[2]) ||
					!stralloc_0(&user))
				_exit (QLX_NOMEM);
			_exit (QLX_UIDGID);
		}
		if (!getuid())
			_exit (QLX_ROOT);
		ptr = setup_qlargs();
		execv(ptr, args);
		if (error_temp(errno))
			_exit (QLX_EXECSOFT);
		_exit (QLX_EXECHARD);
	}
	return f;
}

void
getversion_qmail_lspawn_c()
{
	static char    *x = "$Id: qmail-lspawn.c,v 1.45 2023-10-03 22:48:03+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: qmail-lspawn.c,v $
 * Revision 1.45  2023-10-03 22:48:03+05:30  Cprogrammer
 * use env variable QMAILGETPW to execute alternet qmail-getpw
 *
 * Revision 1.44  2023-09-30 19:46:05+05:30  Cprogrammer
 * skip setuser_privileges for non-etc-passwd users
 *
 * Revision 1.43  2023-09-19 22:32:07+05:30  Cprogrammer
 * set supplementary groups for user if SETUSER_PRIVILEGES is defined
 * New exit code QLX_UIDGID for uid, gid, group setting errors
 *
 * Revision 1.42  2022-03-20 12:29:02+05:30  Cprogrammer
 * bypass indimail if AUTHSELF is not set
 *
 * Revision 1.41  2022-03-16 19:57:57+05:30  Cprogrammer
 * made copy_pwstruct() return type void
 *
 * Revision 1.40  2022-03-05 13:31:55+05:30  Cprogrammer
 * use auto_prefix/sbin for qmail-getpw, qmail-local path
 *
 * Revision 1.39  2022-01-30 08:40:36+05:30  Cprogrammer
 * added case QLX_DIR for chdir failure
 *
 * Revision 1.38  2021-06-29 09:27:49+05:30  Cprogrammer
 * modularize spawn code
 *
 * Revision 1.37  2021-06-05 23:15:16+05:30  Cprogrammer
 * converted function prototypes to ansi
 *
 * Revision 1.36  2021-02-07 23:13:32+05:30  Cprogrammer
 * avoid use of compat functions
 *
 * Revision 1.35  2020-11-24 13:46:58+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.34  2020-03-31 13:17:52+05:30  Cprogrammer
 * fixed bug with setup_qlargs()
 *
 * Revision 1.33  2019-07-29 21:16:59+05:30  Cprogrammer
 * use setup_qlargs() to setup arguments once and cache the args
 *
 * Revision 1.32  2019-05-27 20:29:31+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.31  2019-05-27 12:38:14+05:30  Cprogrammer
 * set libfn with full path of libindimail control file
 *
 * Revision 1.30  2019-05-26 12:32:02+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.29  2019-04-20 19:52:02+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.28  2018-07-15 12:27:36+05:30  Cprogrammer
 * env variable ROUTE_NULL_USER to redirect double bounces to a mailbox
 * use -3 return for error loading libindimail
 *
 * Revision 1.27  2018-07-01 11:51:01+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 * get value of variables userNotFound, is_inactive using getlibObject()
 *
 * Revision 1.26  2018-06-29 23:54:11+05:30  Cprogrammer
 * fixed length of user variable
 *
 * Revision 1.25  2018-01-31 12:06:54+05:30  Cprogrammer
 * moved qmail-getpw, qmail-local to sbin
 *
 * Revision 1.24  2018-01-09 11:47:38+05:30  Cprogrammer
 * use loadLibrary() to load vauth_open(), vauth_getpw(), vclose(), isvirtualdomain() functions
 *
 * Revision 1.23  2017-04-11 03:45:52+05:30  Cprogrammer
 * unset QMAILREMOTE in parent
 *
 * Revision 1.22  2017-01-09 19:36:15+05:30  Cprogrammer
 * use postmaster@virtualdomain for null user (bounce)
 *
 * Revision 1.21  2016-05-18 15:16:10+05:30  Cprogrammer
 * use env variable ASSIGNDIR or auto_assign for users/cdb file
 *
 * Revision 1.20  2014-11-04 23:12:54+05:30  Cprogrammer
 * BUG - fixed incorrect parsing of users with hyphen in username
 *
 * Revision 1.19  2011-07-29 09:29:34+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.18  2011-06-19 09:08:42+05:30  Cprogrammer
 * unset QMAILREMOTE
 *
 * Revision 1.17  2009-12-10 10:56:06+05:30  Cprogrammer
 * return -2 for MySQL error
 *
 * Revision 1.16  2009-03-15 13:26:25+05:30  Cprogrammer
 * BUG - user was incorrectly extracted on domains having '-' in the name
 *
 * Revision 1.15  2007-12-20 13:54:10+05:30  Cprogrammer
 * Made QMAILLOCAL functionality generic
 *
 * Revision 1.14  2005-12-29 13:57:10+05:30  Cprogrammer
 * PWSTRUCT not set when user not found
 *
 * Revision 1.13  2005-06-17 21:52:05+05:30  Cprogrammer
 * removed compilation warning
 *
 * Revision 1.12  2004-10-22 20:28:24+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.11  2004-07-17 21:20:49+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 * Revision 1.10  2003-12-09 00:15:12+05:30  Cprogrammer
 * execute spawn-filter with argv[0] as qmail-local
 *
 * Revision 1.9  2003-12-07 13:03:49+05:30  Cprogrammer
 * added checks for return values of env_put() and env_unset()
 *
 * Revision 1.8  2003-11-29 23:39:15+05:30  Cprogrammer
 * QMAILLOCAL environment variable to execute different qmail-local
 *
 * Revision 1.7  2003-11-25 20:40:00+05:30  Cprogrammer
 * removed fork.h
 *
 * Revision 1.6  2003-10-23 01:23:59+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.5  2003-10-13 10:00:41+05:30  Cprogrammer
 * added msgsize as argument to spawn
 *
 * Revision 1.4  2003-07-20 17:06:04+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.3  2002-09-07 20:21:14+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.2  2002-09-04 01:49:56+05:30  Cprogrammer
 * conditional compilation of indimail code
 *
 * Revision 1.1  2002-08-15 14:43:42+05:30  Cprogrammer
 * Initial revision
 *
 */
