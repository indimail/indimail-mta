/*
 * $Log: qmail-lspawn.c,v $
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
#include "hasindimail.h"
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
#include "auto_uids.h"
#include "qlx.h"
#include "open.h"
#include "byte.h"
#include "env.h"
#include "str.h"

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
	if (wait_crashed(wstat))
	{
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
	for (i = 0; i < len; ++i)
	{
		if (!s[i])
			break;
	}
	substdio_put(ss, s, i);
}

stralloc        lower = { 0 };
stralloc        nughde = { 0 };
stralloc        wildchars = { 0 };

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

	if (!stralloc_copys(&lower, "!"))
		_exit(QLX_NOMEM);
	if (!stralloc_cats(&lower, local))
		_exit(QLX_NOMEM);
	if (!stralloc_0(&lower))
		_exit(QLX_NOMEM);
	case_lowerb(lower.s, lower.len);
	if (!stralloc_copys(&nughde, ""))
		_exit(QLX_NOMEM);
	fd = open_read("users/cdb");
	if (fd == -1)
		if (errno != error_noent)
			_exit(QLX_CDB);
	if (fd != -1)
	{
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
		do
		{
			/*- i > 0 */
			if (!flagwild || (i == 1) || (byte_chr(wildchars.s, wildchars.len, lower.s[i - 1]) < wildchars.len))
			{
				if ((r = cdb_seek(fd, lower.s, i, &dlen)) == -1)
					_exit(QLX_CDB);
				if (r == 1)
				{
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
	args[0] = "bin/qmail-getpw";
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
	if (wait_pid(&gpwstat, gpwpid) != -1)
	{
		if (wait_crashed(gpwstat))
			_exit(QLX_SYS);
		if (wait_exitcode(gpwstat) != 0)
			_exit(wait_exitcode(gpwstat));
	}
}

int
spawn(fdmess, fdout, msgsize, sender, qqeh, recip, at)
	int             fdmess;
	int             fdout;
	unsigned long   msgsize;
	char           *sender;
	char           *qqeh;
	char           *recip;
	int             at;
{
	int             f;
	char           *ptr;
#ifdef INDIMAIL
	register char  *cptr;
	char            user[1024], pwstruct[1024];
	struct passwd  *pw;
#endif

#ifdef INDIMAIL
/*
 * saldo-biuro.com.pl-test1@saldo-biuro.com.pl
 */
	if (env_get("AUTHSELF") && isvirtualdomain(recip + at + 1) && !vauth_open((char *) 0))
	{
		for (f = at - 1;f && recip[f] != '-';f--);
		if (!env_unset("PWSTRUCT"))
			return (-1);
		for(cptr = user, ptr = recip + f + 1;*ptr && *ptr != '@';*cptr++ = *ptr++);
		*cptr = 0;
		if ((pw = (struct passwd *) vauth_getpw(user, recip + at + 1)))
		{
			snprintf(pwstruct, sizeof(pwstruct), "PWSTRUCT=%s@%s:%s:%d:%d:%s:%s:%s:%d", 
				pw->pw_name,
				recip + at + 1,
				pw->pw_passwd,
				pw->pw_uid,
				pw->pw_gid,
				pw->pw_gecos,
				pw->pw_dir,
				pw->pw_shell, is_inactive);
			if (!env_put(pwstruct))
				return (-1);
		}  else
		{
			if (userNotFound)
			{
				snprintf(pwstruct, sizeof(pwstruct), "PWSTRUCT=No such user %s@%s", user, recip + at + 1);
				if (!env_put(pwstruct))
					return (-1);
			} else
			{
				vclose();
				return (-2);
			}
		}
	}
#endif
	if (!(f = fork()))
	{
		char           *(args[12]);
		unsigned long   u;
		int             n;
		int             uid;
		int             gid;
		char           *x;
		unsigned int    xlen;

		recip[at] = 0;
		if (!recip[0])
			_exit(0);/*- <> */
		if (chdir(auto_qmail) == -1)
			_exit(QLX_USAGE);
		nughde_get(recip);
		x = nughde.s;
		xlen = nughde.len;
		args[0] = "bin/qmail-local";
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
	static char    *x = "$Id: qmail-lspawn.c,v 1.17 2009-12-10 10:56:06+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#else
	x++;
#endif
}
