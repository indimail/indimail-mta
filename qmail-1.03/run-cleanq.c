/*
 * $Log: run-cleanq.c,v $
 * Revision 1.4  2004-10-22 20:30:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-09-19 22:48:47+05:30  Cprogrammer
 * changed service name to qscanq
 *
 * Revision 1.2  2004-07-17 21:01:14+05:30  Cprogrammer
 * run-cleanq.c
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "auto_qmail.h"

char           *svco_cmd[] = {
	"bin/svc",
	"-o",
	"/service/qscanq",
	0
};

int
main()
{
	if (chdir(auto_qmail) == -1)
		_exit(1);
	execv(*svco_cmd, svco_cmd);
	_exit(1);	/*- hopefully never reached */ ;
}

void
getversion_run_cleanq_c()
{
	static char    *x = "$Id: run-cleanq.c,v 1.4 2004-10-22 20:30:03+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
