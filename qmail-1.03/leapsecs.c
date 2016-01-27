/*
 * $Log: leapsecs.c,v $
 * Revision 1.1  2016-01-28 01:42:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tai.h"
#include "leapsecs.h"
#include "caldate.h"
#include "auto_qmail.h"

/*
 * XXX: breaks tai encapsulation 
 */

/*
 * XXX: output here has to be binary; DOS redirection uses ASCII 
 */

char            line[100];

int
main()
{
	struct caldate  cd;
	struct tai      t;
	char            x[TAI_PACK];
	long            leaps = 0;

	if (chdir(auto_qmail) == -1)
		exit(1);
	while (fgets(line, sizeof line, stdin)) {
		if (line[0] == '+')
			if (caldate_scan(line + 1, &cd)) {
				t.x = (caldate_mjd(&cd) + 1) * 86400ULL + 4611686014920671114ULL + leaps++;
				tai_pack(x, &t);
				fwrite(x, TAI_PACK, 1, stdout);
			}
	}
	exit(0);
}

void
getversion_leapsecs_c()
{
	static char    *x = "$Id: leapsecs.c,v 1.1 2016-01-28 01:42:07+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
