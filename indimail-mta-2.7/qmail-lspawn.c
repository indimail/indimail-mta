/*
 * $Log: qmail-lspawn.c,v $
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
 * added msgsize as argument to spawn()
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
#include <pwd.h>
#include <unistd.h>
#include "fd.h"
#include "wait.h"
#include "prot.h"
#include "substdio.h"
#include "stralloc.h"
#include "scan.h"
#include "exit.h"
#include "error.h"
#include "cdb.h"
#include "case.h"
#include "slurpclose.h"
#include "auto_qmail.h"
#include "auto_assign.h"
#include "indimail_stub.h"
#include "auto_uids.h"
#include "qlx.h"
#include "open.h"
#include "byte.h"
#include "env.h"
#include "str.h"
#include "fmt.h"

char           *aliasempty;

void
initialize(argc, argv)
	int             argc;
	char          **argv;
{
	aliasempty = argv[1];
	if (!aliasempty)
		_exit(100);
}

int             truncreport = 3000;

void
report(ss, wstat, s, len)
	substdio       *ss;
	int             wstat;
	char           *s;
	int             len;
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

stralloc        lower = { 0 };
stralloc        nughde = { 0 };
stralloc        wildchars = { 0 };
stralloc        cdbfile = { 0 };

char           *cdbdir;

void
nughde_get(local)
	char           *local;
{
	char           *(args[3]);
	int             pi[2];
	int             gpwpid;
	int             gpwstat;
	int             r;
	int             fd;
	int             flagwild;

	if (!cdbdir) {
		if (!(cdbdir = env_get("ASSIGNDIR")))
			cdbdir = auto_assign;
	}
	if (!stralloc_copys(&cdbfile, cdbdir))
		_exit(QLX_NOMEM);
	if (cdbfile.s[cdbfile.len - 1] != '/' && !stralloc_cats(&cdbfile, "/"))
		_exit(QLX_NOMEM);
	if (!stralloc_catb(&cdbfile, "cdb", 4))
		_exit(QLX_NOMEM);
	if (!stralloc_copys(&lower, "!"))
		_exit(QLX_NOMEM);
	if (!stralloc_cats(&lower, local))
		_exit(QLX_NOMEM);
	if (!stralloc_0(&lower))
		_exit(QLX_NOMEM);
	case_lowerb(lower.s, lower.len);
	if (!stralloc_copys(&nughde, ""))
		_exit(QLX_NOMEM);
	if ((fd = open_read(cdbfile.s)) == -1)
		if (errno != error_noent)
			_exit(QLX_CDB);
	if (fd != -1) {
		uint32          dlen;
		unsigned int    i;

		if ((r = cdb_seek(fd, "", 0, &dlen)) != 1)
			_exit(QLX_CDB);
		if (!stralloc_ready(&wildchars, (unsigned int) dlen))
			_exit(QLX_NOMEM);
		wildchars.len = dlen;
		if (cdb_bread(fd, wildchars.s, wildchars.len) == -1)
			_exit(QLX_CDB);
		i = lower.len;
		flagwild = 0;
		do {
			/*- i > 0 */
			if (!flagwild || (i == 1) || (byte_chr(wildchars.s, wildchars.len, lower.s[i - 1]) < wildchars.len)) {
				if ((r = cdb_seek(fd, lower.s, i, &dlen)) == -1)
					_exit(QLX_CDB);
				if (r == 1) {
					if (!stralloc_ready(&nughde, (unsigned int) dlen))
						_exit(QLX_NOMEM);
					nughde.len = dlen;
					if (cdb_bread(fd, nughde.s, nughde.len) == -1)
						_exit(QLX_CDB);
					if (flagwild)
						if (!stralloc_cats(&nughde, local + i - 1))
							_exit(QLX_NOMEM);
					if (!stralloc_0(&nughde))
						_exit(QLX_NOMEM);
					close(fd);
					return;
				}
			}
			--i;
			flagwild = 1;
		}
		while (i);
		close(fd);
	}
	if (pipe(pi) == -1)
		_exit(QLX_SYS);
	args[0] = "sbin/qmail-getpw";
	args[1] = local;
	args[2] = 0;
	switch (gpwpid = vfork())
	{
	case -1:
		_exit(QLX_SYS);
	case 0:
		if (prot_gid(auto_gidn) == -1)
			_exit(QLX_USAGE);
		if (prot_uid(auto_uidp) == -1)
			_exit(QLX_USAGE);
		close(pi[0]);
		if (fd_move(1, pi[1]) == -1)
			_exit(QLX_SYS);
		execv(*args, args);
		_exit(QLX_EXECPW);
	}
	close(pi[1]);
	if (slurpclose(pi[0], &nughde, 128) == -1)
		_exit(QLX_SYS);
	if (wait_pid(&gpwstat, gpwpid) != -1) {
		if (wait_crashed(gpwstat))
			_exit(QLX_SYS);
		if (wait_exitcode(gpwstat) != 0)
			_exit(wait_exitcode(gpwstat));
	}
}

stralloc        pwstruct = { 0 };

static char     strnum[FMT_ULONG];

int
copy_pwstruct(struct passwd *pw, char *recip, int at)
{
	if (!stralloc_copyb(&pwstruct, "PWSTRUCT=", 9))
		return (-1);
	else
	if (!stralloc_cats(&pwstruct, pw->pw_name))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, "@"))
		return (-1);
	else
	if (!stralloc_cats(&pwstruct, recip + at + 1))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	else
	if (!stralloc_cats(&pwstruct, pw->pw_passwd))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	strnum[fmt_uint(strnum, pw->pw_uid)] = 0;
	if (!stralloc_cats(&pwstruct, strnum))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	strnum[fmt_uint(strnum, pw->pw_gid)] = 0;
	if (!stralloc_cats(&pwstruct, strnum))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	else
	if (!stralloc_cats(&pwstruct, pw->pw_gecos))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	else
	if (!stralloc_cats(&pwstruct, pw->pw_dir))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	else
	if (!stralloc_cats(&pwstruct, pw->pw_shell))
		return (-1);
	else
	if (!stralloc_append(&pwstruct, ":"))
		return (-1);
	strnum[fmt_uint(strnum, is_inactive)] = 0;
	if (!stralloc_cats(&pwstruct, strnum))
		return (-1);
	else
	if (!stralloc_0(&pwstruct))
		return (-1);
	return (0);
}

stralloc        user = { 0 };
stralloc        save = { 0 };

int
spawn(fdmess, fdout, msgsize, sender, qqeh, recip_t, at_t)
	int             fdmess;
	int             fdout;
	unsigned long   msgsize;
	char           *sender;
	char           *qqeh;
	char           *recip_t;
	int             at_t;
{
	int             f, len, at = at_t;
	char           *ptr, *libptr, *tptr, *recip = recip_t;
	/*- indimail */
	struct passwd  *pw;
	extern void    *phandle;
	void            (*vclose) (void);
	int             (*isvirtualdomain) (char *);
	int             (*vauth_open) (char *);
	int            *u_not_found, *i_inactive;
	struct passwd*  (*vauth_getpw) (char *, char *);

	if (!env_unset("QMAILREMOTE"))
		_exit(-1);
	/*- indimail */
	if (!(libptr = env_get("VIRTUAL_PKG_LIB")))
		libptr = "libindimail";
	loadLibrary(&phandle, libptr, &f, 0);
	if (f)
		_exit(-3);
	if (!env_get("AUTHSELF") || !phandle)
		goto noauthself;
	if (!(isvirtualdomain = getlibObject(libptr, &phandle, "isvirtualdomain", 0)))
		_exit(-3);
	else
	if (!(vauth_open = getlibObject(libptr, &phandle, "vauth_open", 0)))
		_exit(-3);
	else
	if (!(vauth_getpw = getlibObject(libptr, &phandle, "vauth_getpw", 0)))
		_exit(-3);
	else
	if (!(vclose = getlibObject(libptr, &phandle, "vclose", 0)))
		_exit(-3);
	/*-
	 * recip = example1-example2.com-some_user@example1-example2.com
	 * recip + at + 1 = example1-example2.com
	 */
	if (env_get("QUERY_CACHE")) {
		if ((*isvirtualdomain) (recip + at + 1)) {
			if (!env_unset("PWSTRUCT"))
				return (-1);
			f = str_len(recip + at + 1);
			if ((pw = inquery(PWD_QUERY, recip + f + 1, 0))) {
				if (copy_pwstruct(pw, recip, at))
					return (-1);
				if (!env_put(pwstruct.s))
					return (-1);
			}
		}
	} else
	if ((*isvirtualdomain) (recip + at + 1) && !(*vauth_open) ((char *) 0)) {
		if (!env_unset("PWSTRUCT"))
			return (-1);
		f = str_len(recip + at + 1); /*- domain length */
		for (len = 0, ptr = recip + f + 1; *ptr && *ptr != '@'; ptr++, len++);
		if (len) {
			if (!stralloc_copyb(&user, recip + f + 1, len)) /*- copy user portion */
				return (-1);
			if (!stralloc_0(&user))
				return (-1);
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
				 */
				if (!stralloc_copys(&user, ptr)) /*- copy mailbox@localdomain */
					return (-1);
				if (!stralloc_0(&user))
					return (-1);
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
			if (!stralloc_copyb(&save, tptr + 1, f)) /*- copy domain */
				return (-1);
			if (!stralloc_append(&save, "-"))
				return (-1);
			if (!stralloc_catb(&save, ptr, len)) /*- copy user portion */
				return (-1);
			if (!stralloc_catb(&save, tptr, f + 1)) /*- copy @domain */
				return (-1);
			if (!stralloc_0(&save))
				return (-1);
			if (!stralloc_copyb(&user, ptr, len)) /*- copy user portion */
				return (-1);
			if (!stralloc_0(&user))
				return (-1);
			recip = save.s;
			at = f + 1 + len;
		}
		if (!(u_not_found = (int *) getlibObject(libptr, &phandle, "userNotFound", 0)))
			_exit(-3);
		if ((pw = (struct passwd *) (*vauth_getpw) (user.s, recip + at + 1))) {
			if (!(i_inactive = (int *) getlibObject(libptr, &phandle, "is_inactive", 0)))
				_exit(-3);
			is_inactive = *i_inactive;
			if (copy_pwstruct(pw, recip, at))
				return (-1);
			if (!env_put(pwstruct.s))
				return (-1);
		} else {
			if (*u_not_found) {
				if (!stralloc_copys(&pwstruct, "PWSTRUCT=No such user "))
					return (-1);
				else
				if (!stralloc_cats(&pwstruct, user.s))
					return (-1);
				else
				if (!stralloc_append(&pwstruct, "@"))
					return (-1);
				else
				if (!stralloc_cats(&pwstruct, recip + at + 1))
					return (-1);
				else
				if (!stralloc_0(&pwstruct))
					return (-1);
				else
				if (!env_put(pwstruct.s))
					return (-1);
			} else {
				vclose();
				return (-2);
			}
		}
	} /*- if ((*isvirtualdomain) (recip_t + at_t + 1) && !(*vauth_open) ((char *) 0)) */
/*- end indimail */
noauthself: /*- deliver to local user in control/locals */
	if (!(f = fork())) {
		char           *(args[12]);
		unsigned long   u;
		int             n;
		int             uid;
		int             gid;
		char           *x;
		unsigned int    xlen;

		closeLibrary(&phandle);
		recip[at] = 0;
		if (!recip[0])
			_exit(0);/*- <> */
		if (chdir(auto_qmail) == -1)
			_exit(QLX_USAGE);
		nughde_get(recip);
		x = nughde.s;
		xlen = nughde.len;
		args[0] = "sbin/qmail-local";
		args[1] = "--";
		args[2] = x; /*- user */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit(QLX_USAGE);
		x += n;
		xlen -= n;
		scan_ulong(x, &u);
		uid = u;
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit(QLX_USAGE);
		x += n;
		xlen -= n;
		scan_ulong(x, &u);
		gid = u;
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit(QLX_USAGE);
		x += n;
		xlen -= n;
		args[3] = x; /*- homedir */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit(QLX_USAGE);
		x += n;
		xlen -= n;
		args[4] = recip; /*- local */
		args[5] = x; /*- dash */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit(QLX_USAGE);
		x += n;
		xlen -= n;
		args[6] = x; /*- Ext */
		n = byte_chr(x, xlen, 0);
		if (n++ == xlen)
			_exit(QLX_USAGE);
		x += n;
		xlen -= n;
		args[7] = recip + at + 1;	/*- domain */
		args[8] = sender;			/*- Sender */
		args[9] = aliasempty;		/*- default-delivery */
		args[10] = qqeh;
		args[11] = 0;
		if (fd_move(0, fdmess) == -1)
			_exit(QLX_SYS);
		if (fd_move(1, fdout) == -1)
			_exit(QLX_SYS);
		if (fd_copy(2, 1) == -1)
			_exit(QLX_SYS);
		if (prot_gid(gid) == -1)
			_exit(QLX_USAGE);
		if (prot_uid(uid) == -1)
			_exit(QLX_USAGE);
		if (!getuid())
			_exit(QLX_ROOT);
		if (!(ptr = env_get("QMAILLOCAL")))
			execv(*args, args);
		else
			execv(ptr, args);
		if (error_temp(errno))
			_exit(QLX_EXECSOFT);
		_exit(QLX_EXECHARD);
	}
	return f;
}

void
getversion_qmail_lspawn_c()
{
	static char    *x = "$Id: qmail-lspawn.c,v 1.30 2019-05-26 12:32:02+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
