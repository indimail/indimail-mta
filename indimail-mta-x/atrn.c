/*
 * $Log: atrn.c,v $
 * Revision 1.7  2022-01-30 08:26:55+05:30  Cprogrammer
 * replaced execvp with execv
 *
 * Revision 1.6  2008-05-26 22:19:31+05:30  Cprogrammer
 * removed auto_qmail.h
 *
 * Revision 1.5  2004-10-22 20:21:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-09-25 23:59:18+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.3  2003-10-23 01:15:51+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2003-09-27 21:10:21+05:30  Cprogrammer
 * added code to read morercpthosts
 *
 * Revision 1.1  2003-07-05 17:31:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <case.h>
#include <sig.h>
#include <stralloc.h>
#include <constmap.h>
#include <str.h>
#include <wait.h>
#include <ctype.h>
#include <strerr.h>
#include <sys/types.h>
#include <unistd.h>
#include "etrn.h"
#include "rcpthosts.h"
#include "control.h"
#include "auto_prefix.h"

int             err_child();
void            die_nomem();
void            die_control();

static char    *binatrnargs[4] = { 0, 0, 0, 0 };
static stralloc etrn = { 0 };

int
atrn_queue(char *arg, char *remoteip)
{
	int             child, flagetrn, len, exitcode, wstat, end_flag;
	char           *cptr, *domain_ptr;
	static int      flagrcpt = 1;
	struct constmap mapetrn;
	stralloc        bin = {0};

	if(flagrcpt)
		flagrcpt = rcpthosts_init();
	if((flagetrn = control_readfile(&etrn, "etrnhosts", 0)) == -1)
		die_control();
	if(flagrcpt || !flagetrn)
		return(-2);
	if (!constmap_init(&mapetrn, etrn.s, etrn.len, 0))
		die_nomem();
	for(cptr = domain_ptr = arg;;cptr++) {
		if(*cptr == ' ' || *cptr == ',' || !*cptr) {
			if(*cptr) {
				end_flag = 0;
				*cptr = 0;
			} else
				end_flag = 1;
			case_lowerb(domain_ptr, len = str_len(domain_ptr)); /*- convert into lower case */
			if (!constmap(&mapetrn, domain_ptr, len))
				return(-2);
			if (rcpthosts(domain_ptr, len, 1) != 1)
				return(-2);
			if(end_flag)
				break;
			else
				*cptr = ' ';
			domain_ptr = cptr + 1;
		}
	}
	switch (child = fork())
	{
	case -1:
		return(-1);
	case 0:
		sig_pipedefault();
		dup2(1, 7);
		dup2(0, 6);
		if (!stralloc_copys(&bin, auto_prefix) ||
				!stralloc_catb(&bin, "/bin/atrn", 9) ||
				!stralloc_0(&bin))
			strerr_die1x(111, "atrn: fatal: out of memory");
		binatrnargs[0] = bin.s;
		binatrnargs[1] = arg;
		binatrnargs[2] = remoteip;
		execv(*binatrnargs, binatrnargs);
		_exit(1);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	if (wait_crashed(wstat))
		return err_child();
	if ((exitcode = wait_exitcode(wstat))) {
		exitcode = 0 - exitcode;
		return(exitcode); /*- no */
	}
	return (0);
}

void
getversion_atrn_c()
{
	static char    *x = "$Id: atrn.c,v 1.7 2022-01-30 08:26:55+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
