/*
 * $Log: qregex.c,v $
 * Revision 1.30  2020-09-16 19:06:06+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.29  2020-04-09 16:01:17+05:30  Cprogrammer
 * collapsed multiple stralloc statements
 *
 * Revision 1.28  2018-07-02 00:31:24+05:30  Cprogrammer
 * use addr->len - 1 instead of str_len(addr->s)
 *
 * Revision 1.27  2018-02-11 21:21:05+05:30  Cprogrammer
 * use USE_SQL to compile sql support
 *
 * Revision 1.26  2018-01-09 11:55:42+05:30  Cprogrammer
 * compile sql code with USE_MYSQL definition
 *
 * Revision 1.25  2016-06-14 09:10:09+05:30  Cprogrammer
 * updated with original authors and maintainers
 *
 * Revision 1.24  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.23  2009-09-07 16:02:31+05:30  Cprogrammer
 * removed compilation warning
 *
 * Revision 1.22  2009-09-07 13:56:06+05:30  Cprogrammer
 * disable sqlmatch() if INDIMAIL is not defined
 *
 * Revision 1.21  2009-05-01 10:45:58+05:30  Cprogrammer
 * added errstr argument to matchregex(), cdbmatch(), sqlmatch()
 * use constants from qregex.h when returning errors
 *
 * Revision 1.20  2009-04-30 18:50:57+05:30  Cprogrammer
 * functions cdbmatch() and sqlmatch() not to be invoked if filename is null
 *
 * Revision 1.19  2009-04-30 16:14:28+05:30  Cprogrammer
 * moved out matchregex() function
 * conditional compilation of sqlmatch()
 *
 * Revision 1.18  2009-04-30 15:41:03+05:30  Cprogrammer
 * added sqlmatch() for qmail control files in MySQL
 *
 * Revision 1.17  2009-04-30 11:47:37+05:30  Cprogrammer
 * added cdbmatch() function
 *
 * Revision 1.16  2009-04-29 10:56:25+05:30  Cprogrammer
 * use str_len() for key length
 *
 * Revision 1.15  2009-04-29 08:59:31+05:30  Cprogrammer
 * added function to search cdb
 *
 * Revision 1.14  2009-04-29 08:23:29+05:30  Cprogrammer
 * simplified wildmat_match() and regexec_match()
 *
 * Revision 1.13  2007-12-20 13:51:04+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.12  2005-08-23 17:41:36+05:30  Cprogrammer
 * regex to be turned on only of QREGEX is defined to non-zero value
 *
 * Revision 1.11  2005-04-02 19:07:25+05:30  Cprogrammer
 * use internal wildmat version
 *
 * Revision 1.10  2005-01-22 00:39:04+05:30  Cprogrammer
 * added missing error handling
 *
 * Revision 1.9  2004-10-22 20:29:45+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.8  2004-09-21 23:48:18+05:30  Cprogrammer
 * made matchregex() visible
 * introduced dotChar (configurable dot char)
 *
 * Revision 1.7  2004-02-05 18:48:48+05:30  Cprogrammer
 * changed curregex to static
 *
 * Revision 1.6  2003-12-23 23:22:53+05:30  Cprogrammer
 * implicitly use wildcard if address starts with '@'
 *
 * Revision 1.5  2003-12-22 18:33:12+05:30  Cprogrammer
 * added address_match()
 *
 * Revision 1.4  2003-12-22 13:21:08+05:30  Cprogrammer
 * added text and pattern as part of error message
 *
 * Revision 1.3  2003-12-22 10:04:04+05:30  Cprogrammer
 * conditional compilation of qregex
 *
 * Revision 1.2  2003-12-21 15:32:18+05:30  Cprogrammer
 * added regerror
 *
 * Revision 1.1  2003-12-20 13:17:16+05:30  Cprogrammer
 * Initial revision
 *
 * qregex (v2)
 * $Id: qregex.c,v 1.30 2020-09-16 19:06:06+05:30 Cprogrammer Exp mbhangui $
 *
 * Author  : Evan Borgstrom (evan at unixpimps dot org)
 * Created : 2001/12/14 23:08:16
 * Modified: $Date: 2020-09-16 19:06:06+05:30 $
 * Revision: $Revision: 1.30 $
 *
 * Do POSIX regex matching on addresses for anti-relay / spam control.
 * It logs to the maillog
 * See the qregex-readme file included with this tarball.
 * If you didn't get this file in a tarball please see the following URL:
 *  http://www.unixpimps.org/software/qregex
 *
 * qregex.c is released under a BSD style copyright.
 * See http://www.unixpimps.org/software/qregex/copyright.html
 *
 *
 * CONTACT:
 *  qregex is maintained by:
 *	Andrew St. Jean
 *	andrew@arda.homeunix.net
 *	www.arda.homeunix.net/store/qmail/
 *
 * Contributers to qregex:
 *	Jeremy Kitchen	
 *	kitchen at scriptkitchen dot com
 *	http://www.scriptkitchen.com/qmail

 *	Alex Pleiner
 *	alex@zeitform.de
 *	zeitform Internet Dienste
 *	http://www.zeitform.de/
 *
 *	Thanos Massias
 *
 * Original qregex patch written by:
 *	Evan Borgstrom
 *	evan at unixpimps dot org
 * Note: this revision follows the coding guidelines set forth by the rest of
 *       the qmail code and that described at the following URL.
 *       http://cr.yp.to/qmail/guarantee.html
 * 
 */
#include "case.h"
#include "scan.h"
#include "stralloc.h"
#include "constmap.h"
#include "byte.h"
#include "str.h"
#include "env.h"
#include "cdb.h"
#include "uint32.h"
#include "open.h"
#include "error.h"
#include "control.h"
#include "matchregex.h"
#include "qregex.h"
#include "auto_control.h"
#include "variables.h"
#include <sys/types.h>
#include <unistd.h>

static int      wildmat_match(stralloc *, struct constmap *, stralloc *);
static int      regex_match(stralloc *, stralloc *, char **);
int             wildmat_internal(char *, char *);

static char     dotChar = '@';

int
cdbmatch(char *fn, char *addr, int len, struct constmap *maprh, char **errStr)
{
	static stralloc controlfile = {0};
	static stralloc temp = { 0 };
	uint32          dlen;
	int             fd_cdb, cntrl_ok;

	if (!len || !*addr || !fn)
		return (0);
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (errStr)
		*errStr = 0;
	if (!stralloc_copys(&controlfile, controldir) || !stralloc_cats(&controlfile, "/")
			|| !stralloc_cats(&controlfile, fn) || !stralloc_cats(&controlfile, ".cdb")
			|| !stralloc_0(&controlfile)) {
		if (errStr) 
			*errStr = error_str(errno);
		return AM_MEMORY_ERR;
	}
	if ((fd_cdb = open_read(controlfile.s)) == -1) {
		if (errno != error_noent) {
			if (errStr) 
				*errStr = error_str(errno);
			return AM_FILE_ERR;
		}
		if (!maprh)
			return (0);
		/*- cdb missing or entry missing */
		if ((cntrl_ok = control_readfile(&temp, fn, 0)) == -1) {
			if (errStr) 
				*errStr = error_str(errno);
			return AM_FILE_ERR;
		}
		if (cntrl_ok == 1 && !constmap_init(maprh, temp.s, temp.len, 0)) {
			if (errStr) 
				*errStr = error_str(errno);
			return AM_MEMORY_ERR;
		}
		if (!stralloc_copyb(&temp, addr, len)) {
			if (errStr) 
				*errStr = error_str(errno);
			return AM_MEMORY_ERR;
		}
		return (cntrl_ok == 1 ? (constmap(maprh, temp.s, len) ? 1 : 0) : 0);
	}
	if (!stralloc_copyb(&temp, addr, len)) {
		if (errStr) 
			*errStr = error_str(errno);
		close(fd_cdb);
		return AM_MEMORY_ERR;
	}
	if ((cntrl_ok = cdb_seek(fd_cdb, temp.s, len, &dlen)) == -1) {
		if (errStr) 
			*errStr = error_str(errno);
		close(fd_cdb);
		return (AM_LSEEK_ERR);
	}
	close(fd_cdb);
	return (cntrl_ok ? 1 : 0);
}

void
setdotChar(char c)
{
	dotChar = c;
	return;
}

int
address_match(char *fn, stralloc *addr, stralloc *bhf, struct constmap *mapbhf,
	stralloc *wildcard, char **errStr)
{
	char           *ptr;
	int             x = 0;

	case_lowerb(addr->s, addr->len); /*- convert into lower case */
	if (errStr)
		*errStr = 0;
	if (fn && (x = cdbmatch(fn, addr->s, addr->len - 1, 0, errStr)))
		return (x);
#if defined(USE_SQL)
	if (fn && (x = sqlmatch(fn, addr->s, addr->len - 1, errStr)))
		return (x);
#endif
	if ((ptr = env_get("QREGEX")))
		scan_int(ptr, &x);
	if (ptr && x)
		return (regex_match(addr, bhf, errStr));
	else
		return (wildmat_match(addr, mapbhf, wildcard));
}

static int
wildmat_match(stralloc *addr, struct constmap *ptrmap, stralloc *wildcard)
{
	int             i = 0;
	int             j = 0;
	int             k = 0;
	char            subvalue;

	if (ptrmap) {
		if (constmap(ptrmap, addr->s, addr->len - 1))
			return 1;
		if ((j = byte_rchr(addr->s, addr->len, dotChar)) < addr->len) {
			if (constmap(ptrmap, addr->s + j, addr->len - j - 1))
				return 1;
		}
	}
	/*- Include control file control/xxxxpatterns and evaluate with Wildmat check */
	if (wildcard) {
		i = 0;
		for (j = 0; j < wildcard->len; ++j) {
			if (!wildcard->s[j]) {
				subvalue = wildcard->s[i] != '!';
				if (!subvalue)
					i++;
				if ((k != subvalue) && wildmat_internal(addr->s, wildcard->s + i))
					k = subvalue;
				i = j + 1;
			}
		}
		return k;
	}
	return (0);
}

static int
regex_match(stralloc *addr, stralloc *map, char **errStr)
{
	int             i = 0;
	int             j = 0;
	int             k = 0;
	int             negate = 0, match;
	static stralloc curregex = { 0 };

	match = 0;
	if (errStr)
		*errStr = 0;
	if (map) {
		while (j < map->len) {
			i = j;
			while ((map->s[i] != '\0') && (i < map->len))
				i++;
			if (map->s[j] == '!') {
				negate = 1;
				j++;
			}
			if (*(map->s + j) == dotChar) {
				if (!stralloc_copys(&curregex, ".*") || !stralloc_catb(&curregex, map->s + j, (i - j)))
					return (AM_MEMORY_ERR);
			} else
			if (!stralloc_copyb(&curregex, map->s + j, (i - j)))
				return (AM_MEMORY_ERR);
			if (!stralloc_0(&curregex))
				return (AM_MEMORY_ERR);
			if ((k = matchregex(addr->s, curregex.s, errStr)) == 1) {
				if (negate)
					return (0);
				match = 1;
			}
			j = i + 1;
			negate = 0;
		}
	}
	return (match);
}

void
getversion_qregex_c()
{
	static char    *x = "$Id: qregex.c,v 1.30 2020-09-16 19:06:06+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
