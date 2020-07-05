/*
 * $Log: qmail-smtpd.c,v $
 * Revision 1.4  2020-07-05 10:58:04+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.3  2004-10-22 20:29:36+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:21:51+05:30  Cprogrammer
 * added RCS log
 *
 */
void qmail_smtpd(int, char **, char **);

int
main(int argc, char **argv)
{
	qmail_smtpd(argc, argv, 0);
	return(0);
}

void
getversion_qmail_smtpd_c()
{
	static char    *x = "$Id: qmail-smtpd.c,v 1.4 2020-07-05 10:58:04+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
