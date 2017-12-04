/*
 * $Log: inc_dir_control.c,v $
 * Revision 2.1  2010-08-08 13:01:38+05:30  Cprogrammer
 * made users_per_level configurable
 *
 * Revision 1.4  2002-03-03 17:13:54+05:30  Cprogrammer
 * replaced strcat with scat
 *
 * Revision 1.3  2001-11-24 12:19:12+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:01+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: inc_dir_control.c,v 2.1 2010-08-08 13:01:38+05:30 Cprogrammer Stab mbhangui $";
#endif

int
inc_dir_control(vdir_type * vdir, int max_users_per_level)
{
	int             users_per_level;

	++vdir->cur_users;
	users_per_level = max_users_per_level ? max_users_per_level : MAX_USERS_PER_LEVEL;
	if (vdir->cur_users % users_per_level == 0)
	{
		if (!*(vdir->the_dir))
		{
			vdir->the_dir[0] = dirlist[vdir->level_start[0]];
			vdir->the_dir[1] = 0;
			return (0);
		}

		if (vdir->level_index[vdir->level_cur] == vdir->level_end[vdir->level_cur])
		{
			switch (vdir->level_cur)
			{
			case 0:
				inc_dir(vdir, vdir->level_cur);
				++vdir->level_cur;
				scat(vdir->the_dir, "/", MAX_DIR_NAME);
				break;
			case 1:
				if (vdir->level_index[0] == vdir->level_end[0] && vdir->level_index[1] == vdir->level_end[1])
				{
					inc_dir(vdir, vdir->level_cur);
					++vdir->level_cur;
					scat(vdir->the_dir, "/", MAX_DIR_NAME);
				}
				break;
			}
		}
		inc_dir(vdir, vdir->level_cur);
	}
	return (0);
}

void
getversion_inc_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
