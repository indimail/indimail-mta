/*
 * $Log: print_control.c,v $
 * Revision 2.5  2010-08-15 15:53:50+05:30  Cprogrammer
 * display users per level
 *
 * Revision 2.4  2009-02-18 21:33:05+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.3  2009-02-06 11:40:01+05:30  Cprogrammer
 * ignore return value of fscanf
 *
 * Revision 2.2  2003-10-06 00:01:40+05:30  Cprogrammer
 * removed unecessary printing of error if .filesystem was absent
 *
 * Revision 2.1  2003-01-14 12:43:39+05:30  Cprogrammer
 * added silent option
 *
 * Revision 1.5  2001-12-03 00:32:02+05:30  Cprogrammer
 * formating change of printf
 *
 * Revision 1.4  2001-12-02 18:46:48+05:30  Cprogrammer
 * rewrite of print_control.
 * list of filesystems to be taken from file /var/indimail/domains/'domain_name'/.filesystems
 *
 * Revision 1.3  2001-11-24 12:19:51+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:45+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>

#ifndef	lint
static char     sccsid[] = "$Id: print_control.c,v 2.5 2010-08-15 15:53:50+05:30 Cprogrammer Stab mbhangui $";
#endif

unsigned long
print_control(char *filename, char *domain, int max_users_per_level, int silent)
{
	FILE           *fp;
	char           *ptr;
	char            buffer[MAX_BUFF];
	int             users_per_level = 0;
	unsigned long   total = 0;

	if ((fp = fopen(filename, "r")) != (FILE *) 0)
	{
		users_per_level = max_users_per_level ? max_users_per_level : MAX_USERS_PER_LEVEL;
		for (;;)
		{
			if (fscanf(fp, "%s", buffer) != 1)
			{
				if (ferror(fp))
					break;
			}
			if (feof(fp))
				break;
			vread_dir_control(buffer, &vdir, domain);
			for(ptr = buffer;*ptr;ptr++)
				if (*ptr == '_')
					*ptr = '/';
			if (!silent)
			{
				printf("Dir Control     = %s\n", buffer);
				printf("cur users       = %-8lu\n", vdir.cur_users);
				printf("dir prefix      = %s\n", vdir.the_dir);
				printf("Users per level = %d\n", users_per_level);
				printf("level_cur       = %-5d\n", vdir.level_cur);
				printf("level_max       = %-5d\n", vdir.level_max);
				printf("level_index 0   = %-5d\n", vdir.level_index[0]);
				printf("            1   = %-5d\n", vdir.level_index[1]);
				printf("            2   = %-5d\n", vdir.level_index[2]);
				printf("level_start 0   = %-5d\n", vdir.level_start[0]);
				printf("            1   = %-5d\n", vdir.level_start[1]);
				printf("            2   = %-5d\n", vdir.level_start[2]);
				printf("level_end   0   = %-5d\n", vdir.level_end[0]);
				printf("            1   = %-5d\n", vdir.level_end[1]);
				printf("            2   = %-5d\n", vdir.level_end[2]);
				printf("level_mod   0   = %-5d\n", vdir.level_mod[0]);
				printf("            1   = %-5d\n", vdir.level_mod[1]);
				printf("            2   = %-5d\n", vdir.level_mod[2]);
				putchar(10);
			}
			total += vdir.cur_users;
		}
		fclose(fp);
		return(total);
	}
	return(0);
}

void
getversion_print_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
