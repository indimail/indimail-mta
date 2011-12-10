/*
 * $Log: inc_dir.c,v $
 * Revision 1.3  2001-11-24 12:19:11+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:00+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: inc_dir.c,v 1.3 2001-11-24 12:19:11+05:30 Cprogrammer Stab mbhangui $";
#endif

static char next_char(char, int, int);

char           *
inc_dir(vdir_type * vdir, int in_level)
{
	if (vdir->the_dir[vdir->level_mod[in_level]] == dirlist[vdir->level_end[in_level]])
	{
		vdir->the_dir[vdir->level_mod[in_level]] = dirlist[vdir->level_start[in_level]];
		vdir->level_index[in_level] = vdir->level_start[in_level];
		if (in_level > 0)
			inc_dir(vdir, in_level - 1);
	} else
	{
		vdir->the_dir[vdir->level_mod[in_level]] =
			next_char(vdir->the_dir[vdir->level_mod[in_level]], vdir->level_start[in_level], vdir->level_end[in_level]);
		++vdir->level_index[in_level];
	}
	return (vdir->the_dir);
}

static char
next_char(char in_char, int in_start, int in_end)
{
	int             i;

	for (i = in_start; i < in_end + 1 && dirlist[i] != in_char; ++i);
	++i;
	if (i >= in_end + 1)
		i = in_start;
	return (dirlist[i]);
}

void
getversion_inc_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
