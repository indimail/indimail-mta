/*
 * $Log: readsubdir.c,v $
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
#include "readsubdir.h"
#include "fmt.h"
#include "scan.h"
#include "str.h"

void
readsubdir_init(readsubdir *rs, char *name, void (*pause)())
{
	rs->name = name;
	rs->pause = pause;
	rs->dir = 0;
	rs->pos = 0;
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
	namepos[len++] = '/';
	len += fmt_ulong(namepos + len, (unsigned long) rs->pos);
	namepos[len] = 0;
	while (!(rs->dir = opendir(namepos)))
		rs->pause(namepos);
	return;
}

char           *
readsubdir_name(readsubdir *rs)
{
	return namepos + rslen + 1;
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
	static char    *x = "$Id: readsubdir.c,v 1.5 2021-05-12 17:50:21+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
