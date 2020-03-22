/*
 * $Log: fn_handler.c,v $
 * Revision 1.1  2018-04-25 21:36:33+05:30  Cprogrammer
 * Initial revision
 *
 */

int 
fn_handler(errfn, timeoutfn, option, arg)
	void              (*errfn)();
	void              (*timeoutfn)();
	int               option;
	char             *arg;
{
	if (!option)
		(*errfn)(arg);
	else
		(*timeoutfn)();
	return (-1);
}

void
getversion_fn_handler_c()
{
	static char    *x = "$Id: fn_handler.c,v 1.1 2018-04-25 21:36:33+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
