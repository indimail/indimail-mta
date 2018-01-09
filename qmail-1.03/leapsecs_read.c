/*
 * $Log: leapsecs_read.c,v $
 * Revision 1.4  2018-01-09 11:44:10+05:30  Cprogrammer
 * moved leapsecs.dat to /etc/indimail
 *
 * Revision 1.3  2016-01-28 09:00:17+05:30  Cprogrammer
 * open leapsecs.dat in qmail home/etc directory
 *
 * Revision 1.2  2004-10-22 20:26:04+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-05-14 00:44:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "tai.h"
#include "leapsecs.h"
#include <unistd.h>
#include <stdlib.h>

struct tai     *leapsecs = 0;
int             leapsecs_num = 0;

int
leapsecs_read()
{
	int             fd;
	struct stat     st;
	struct tai     *t;
	int             n;
	int             i;
	struct tai      u;

	fd = open("leapsecs.dat", O_RDONLY | O_NDELAY);
	if (fd == -1)
	{
		if (errno != ENOENT)
			return -1;
		if (leapsecs)
			free(leapsecs);
		leapsecs = 0;
		leapsecs_num = 0;
		return 0;
	}

	if (fstat(fd, &st) == -1)
	{
		close(fd);
		return -1;
	}

	t = (struct tai *) malloc(st.st_size);
	if (!t)
	{
		close(fd);
		return -1;
	}

	n = read(fd, (char *) t, st.st_size);
	close(fd);
	if (n != st.st_size)
	{
		free(t);
		return -1;
	}

	n /= sizeof(struct tai);

	for (i = 0; i < n; ++i)
	{
		tai_unpack((char *) &t[i], &u);
		t[i] = u;
	}

	if (leapsecs)
		free(leapsecs);

	leapsecs = t;
	leapsecs_num = n;
	return(0);
}

void
getversion_leapsecs_read_c()
{
	static char    *x = "$Id: leapsecs_read.c,v 1.4 2018-01-09 11:44:10+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
