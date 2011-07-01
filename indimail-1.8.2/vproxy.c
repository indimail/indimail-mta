/*
 * $Log: vproxy.c,v $
 * Revision 1.1  2002-03-29 00:21:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <string.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vproxy.c,v 1.1 2002-03-29 00:21:34+05:30 Cprogrammer Stab mbhangui $";
#endif

int
main(int argc, char **argv)
{
	char           *ptr;
	
	if((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	if(argc == 3)
		return(monkey(argv[1], argv[2], 0, 0));
	else
	if(argc == 4)
		return(monkey(argv[1], argv[2], argv[3], 0));
	fprintf(stderr, "USAGE: %s host port [login sequence]\n", ptr);
	return(1);
}

void
getversion_vproxy_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
