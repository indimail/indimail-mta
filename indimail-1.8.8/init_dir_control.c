/*
 * $Log: init_dir_control.c,v $
 * Revision 1.1  2001-12-02 12:12:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: init_dir_control.c,v 1.1 2001-12-02 12:12:43+05:30 Cprogrammer Stab mbhangui $";
#endif

void
init_dir_control(vdir_type *vbuf)
{
	int             i;

	vbuf->cur_users = 0;
	vbuf->level_cur = 0;
	vbuf->level_max = MAX_DIR_LEVELS;
	for (i = 0; i < MAX_DIR_LEVELS; ++i)
	{
		vbuf->level_start[i] = 0;
		vbuf->level_end[i] = MAX_DIR_LIST - 1;
		vbuf->level_index[i] = 0;
		vbuf->level_mod[i] = i * 2;
	}
	vbuf->the_dir[0] = 0;
	return;
}

void
getversion_init_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
