/*
 * $Id: cdb_match.c,v 1.1 2022-10-30 22:14:47+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <stralloc.h>
#include <open.h>
#include <env.h>
#include <error.h>
#include <cdb.h>
#include <case.h>
#include <byte.h>
#include "variables.h"
#include "auto_control.h"
#include "cdb_match.h"

int
cdb_matchaddr(const char *fn, const char *addr, int len)
{
	static stralloc controlfile = {0};
	static stralloc temp = { 0 };
	uint32          dlen;
	int             fd_cdb, cntrl_ok;

	if (!len || !*addr || !fn)
		return CDB_NOMATCH;
	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&controlfile, controldir) ||
			!stralloc_cats(&controlfile, "/") ||
			!stralloc_cats(&controlfile, fn) ||
			!stralloc_cats(&controlfile, ".cdb") ||
			!stralloc_0(&controlfile))
		return CDB_MEM_ERR;
	if ((fd_cdb = open_read(controlfile.s)) == -1) {
		if (errno != error_noent)
			return CDB_FILE_ERR;
		/*- cdb missing or entry missing */
		return (0);
	}
	if (!stralloc_copyb(&temp, addr, len)) {
		close(fd_cdb);
		return CDB_MEM_ERR;
	}
	if ((cntrl_ok = cdb_seek(fd_cdb, temp.s, len, &dlen)) == -1) {
		close(fd_cdb);
		return CDB_LSEEK_ERR;
	}
	close(fd_cdb);
	return (cntrl_ok ? CDB_FOUND : CDB_NOMATCH);
}

int
cdb_match(const char *fn, const char *addr, int len, char **result)
{
	static stralloc controlfile = {0}, wildchars = {0}, lower = {0},
					tmpbuf = {0};
	uint32          dlen;
	int             fd_cdb, i, r, flagwild;

	*result = (char *) 0;
	if (!len || !*addr || !fn)
		return CDB_NOMATCH;
	if(!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (!stralloc_copys(&controlfile, controldir) || !stralloc_cats(&controlfile, "/")
			|| !stralloc_cats(&controlfile, fn) || !stralloc_0(&controlfile))
		return CDB_MEM_ERR;
	if ((fd_cdb = open_read(controlfile.s)) == -1) {
		if (errno != error_noent)
			return CDB_FILE_ERR;
		/*- cdb missing or entry missing */
		return CDB_NOMATCH;
	}
	if (!stralloc_copyb(&lower, "!", 1) ||
			!stralloc_catb(&lower, addr, len) ||
			!stralloc_0(&lower)) {
		close(fd_cdb);
		return CDB_MEM_ERR;
	}
	case_lowerb(lower.s, lower.len);
	if ((r = cdb_seek(fd_cdb, "", 0, &dlen)) != 1) {
		close(fd_cdb);
		return CDB_LSEEK_ERR;
	}
	if (!stralloc_ready(&wildchars, (unsigned int) dlen)) {
		close(fd_cdb);
		return CDB_MEM_ERR;
	}
	wildchars.len = dlen;
	if (cdb_bread(fd_cdb, wildchars.s, wildchars.len) == -1) {
		close(fd_cdb);
		return CDB_READ_ERR;
	}
	i = lower.len;
	flagwild = 0;
	do {
		/*- i > 0 */
		if (!flagwild || (i == 1) || (byte_chr(wildchars.s, wildchars.len, lower.s[i - 1]) < wildchars.len)) {
			if ((r = cdb_seek(fd_cdb, lower.s, i, &dlen)) == -1) {
				close(fd_cdb);
				return CDB_LSEEK_ERR;
			}
			if (r == 1) {
				if (!stralloc_ready(&tmpbuf, (unsigned int) dlen)) {
					close(fd_cdb);
					return CDB_MEM_ERR;
				}
				tmpbuf.len = dlen;
				if (cdb_bread(fd_cdb, tmpbuf.s, dlen) == -1) {
					close(fd_cdb);
					return CDB_READ_ERR;
				}
				close(fd_cdb);
				if (flagwild && !stralloc_catb(&tmpbuf, addr + i - 1, len - i + 1))
					return CDB_MEM_ERR;
				if (!stralloc_0(&tmpbuf))
					return CDB_MEM_ERR;
				*result = tmpbuf.s;
				return CDB_FOUND;
			}
		}
		--i;
		flagwild = 1;
	} while (i);
	return CDB_NOMATCH;
}

#ifndef lint
void
getversion_cdb_match_c()
{
	const char     *x = "$Id: cdb_match.c,v 1.1 2022-10-30 22:14:47+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif

/*
 * $Log: cdb_match.c,v $
 * Revision 1.1  2022-10-30 22:14:47+05:30  Cprogrammer
 * Initial revision
 *
 */
