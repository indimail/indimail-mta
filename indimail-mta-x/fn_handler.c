/*
 * $Id: fn_handler.c,v 1.3 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */

int
fn_handler(void (*errfn)(const char *arg), void (*timeoutfn)(void), int option, const char *arg)
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
	const char     *x = "$Id: fn_handler.c,v 1.3 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: fn_handler.c,v $
 * Revision 1.3  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2018-04-25 21:36:33+05:30  Cprogrammer
 * Initial revision
 *
 */
