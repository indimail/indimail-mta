/*
 * $Log: lowerit.c,v $
 * Revision 1.2  2001-11-20 10:55:22+05:30  Cprogrammer
 * Added getversion_lowerit_c()
 *
 * Revision 1.1  2001-10-24 18:15:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <ctype.h>

#ifndef	lint
static char     sccsid[] = "$Id: lowerit.c,v 1.2 2001-11-20 10:55:22+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * make all characters in a string be lower case
 */
void
lowerit(instr)
	char           *instr;
{
	if (!instr || !*instr)
		return;
	for (; *instr != 0; ++instr)
		if (isupper((int) *instr))
			*instr = tolower(*instr);
}

#include <stdio.h>
void
getversion_lowerit_c()
{
	printf("%s\n", sccsid);
}
