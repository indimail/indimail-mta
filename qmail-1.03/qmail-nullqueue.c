/*
 * $Log: qmail-nullqueue.c,v $
 * Revision 1.3  2004-10-22 20:28:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-15 23:32:39+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2003-10-11 00:04:43+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Send Mails to Trash
 */
#include <unistd.h>

int
main()
{
	int             n;
	char            buf[2048];

	for (;;)
	{
		if ((n = read(0, buf, sizeof(buf))) == -1)
			_exit(54);
		if (!n)
			break;
	}
	for (;;)
	{
		if ((n = read(1, buf, sizeof(buf))) == -1)
			_exit(54);
		if (!n)
			break;
	}
	_exit (0);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_nullqueue_c()
{
	static char    *x = "$Id: qmail-nullqueue.c,v 1.3 2004-10-22 20:28:35+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
