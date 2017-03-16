/*
 * $Log: mail_report.c,v $
 * Revision 2.2  2008-07-28 23:43:15+05:30  Cprogrammer
 * 64 bit compiant
 *
 * Revision 2.1  2008-07-28 23:28:17+05:30  Cprogrammer
 * provide detailed listing of mails in user's Maildir
 *
 */
#include "indimail.h"
#include <stdio.h>

#ifndef	lint
static char     sccsid[] = "$Id: mail_report.c,v 2.2 2008-07-28 23:43:15+05:30 Cprogrammer Stab mbhangui $";
#endif

static void     usage();

int
main(int argc, char **argv)
{
	char           *maildir;
	mdir_t          mailcount, trashtotal, trashcount, unread, unseen;
	mdir_t          total;

	if (argc != 2)
	{
		usage();
		return(1);
	}
	maildir=argv[1];
	trashcount = mailcount = trashtotal = total = unread = unseen = 0;
	if (ScanDir(stdout, maildir, 2, &total, &mailcount, &trashtotal, &trashcount, &unread, &unseen))
	{
		fprintf(stderr, "Failed to scan %s dir", argv[1]);
		return (1);
	}
	fprintf(stdout, "---------------------------------------------------------------------------\n");
	fprintf(stdout, "No of Mails %"PRIu64", Size of Mails %"PRIu64", Trashed Mails %"PRIu64", Size of Trashed Mails %"PRIu64"\n",
		mailcount, total, trashcount, trashtotal);
	fflush(stdout);
	return 0;
}

void usage()
{
	fprintf(stderr, "usage: mail_report homedir\n");
	return;
}

void
getversion_mail_report_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
