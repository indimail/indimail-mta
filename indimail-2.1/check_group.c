/*
 * $Log: check_group.c,v $
 * Revision 2.1  2016-06-09 15:27:25+05:30  Cprogrammer
 * function to check if a gid belongs to current process supplementary groups
 *
 */
#include <stdio.h>
#include "error_stack.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#endif
#include <unistd.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: check_group.c,v 2.1 2016-06-09 15:27:25+05:30 Cprogrammer Exp mbhangui $";
#endif

int
check_group(gid_t gid)
{
	int             size, i;
	gid_t          *list;

	if ((size = getgroups(0, (gid_t *) 0)) == -1) {
		error_stack(stderr, "check_group: getgroups: size\n");
		return (-1);
	}
	if (!(list = (gid_t *) malloc(size * sizeof (gid_t)))) {
		error_stack(stderr, "check_group: malloc\n");
		return (-1);
	}
	if ((size = getgroups(size, list)) == -1) {
		error_stack(stderr, "check_group: getgroups\n");
		return (-1);
	}
	for (i = 0; i < size; i++) {
		if (list[i] == gid)
			return (1);
	}
	return (0);
}

void
getversion_check_group_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsid_error_stackh);
}
