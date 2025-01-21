/*
 * $Id: readsubdir.c,v 1.9 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include <stddef.h>
#include "readsubdir.h"
#include "fmt.h"
#include "scan.h"
#include "str.h"

void
readsubdir_init(readsubdir *rs, const char *name, int flagsplit, void (*pause)(const char *))
{
	rs->name = name;
	rs->pause = pause;
	rs->dir = 0;
	rs->pos = 0;
	rs->split = flagsplit;
}

static char     namepos[FMT_ULONG + 4 + READSUBDIR_NAMELEN];
static int      rslen;
extern int      conf_split;

void
opensubdir(readsubdir *rs)
{
	unsigned int    len;

	len = 0;
	len += fmt_str(namepos + len, rs->name);
	if (rs->split) {
		namepos[len++] = '/';
		len += fmt_ulong(namepos + len, (unsigned long) rs->pos);
	}
	namepos[len] = 0;
	while (!(rs->dir = opendir(namepos)))
		rs->pause(namepos);
	return;
}

char           *
readsubdir_name(readsubdir *rs)
{

	return rs->split ? namepos + rslen + 1 : (char *) NULL;
}

int
readsubdir_next(readsubdir *rs, unsigned long *id)
{
	direntry       *d;
	unsigned int    len;

	if ((rslen = str_len(rs->name)) > READSUBDIR_NAMELEN)
		return -1;
	for (;;) {
		if (!rs->dir)
			opensubdir(rs);
		if (!(d = readdir(rs->dir))) {
			closedir(rs->dir);
			rs->dir = 0;
			if (!rs->split)
				return 0;
			rs->pos++;
			if (rs->pos == conf_split)
				return 0; /*- no more files in any directory */
			continue;
		}
		if (str_equal(d->d_name, ".") || str_equal(d->d_name, ".."))
			continue;
		len = scan_ulong(d->d_name, id);
		if (!len || d->d_name[len])
			return -2; /*- invalid filename */
		return 1;
	}
}

void
getversion_readsubdir_c()
{
	const char     *x = "$Id: readsubdir.c,v 1.9 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: readsubdir.c,v $
 * Revision 1.9  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.8  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2022-01-30 09:38:02+05:30  Cprogrammer
 * allow configurable big/small todo/intd
 *
 * Revision 1.5  2021-05-12 17:50:21+05:30  Cprogrammer
 * added readsubdir_name()
 *
 * Revision 1.4  2021-05-12 15:43:18+05:30  Cprogrammer
 * reduce calls required to get next entry
 *
 * Revision 1.3  2004-10-22 20:29:54+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:22:30+05:30  Cprogrammer
 * added RCS log
 *
 */
