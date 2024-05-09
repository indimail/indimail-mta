/*
 * $Log: fn_handler.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2018-04-25 21:36:33+05:30  Cprogrammer
 * Initial revision
 *
 */

int
fn_handler(void (*errfn)(), void (*timeoutfn)(), int option, const char *arg)
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
	const char     *x = "$Id: fn_handler.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
