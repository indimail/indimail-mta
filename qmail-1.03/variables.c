/*
 * $Log: variables.c,v $
 * Revision 1.4  2017-03-21 15:40:38+05:30  Cprogrammer
 * added certdir variable
 *
 * Revision 1.3  2004-10-22 20:32:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:25:02+05:30  Cprogrammer
 * added RCS log
 *
 */
char           *queuedir;
char           *controldir;
char           *certdir;

void
getversion_variables_c()
{
	static char    *x = "$Id: variables.c,v 1.4 2017-03-21 15:40:38+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
