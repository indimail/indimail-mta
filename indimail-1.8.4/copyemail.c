/*
 * $Log: copyemail.c,v $
 * Revision 2.2  2008-11-21 16:02:56+05:30  Cprogrammer
 * initialize errflag
 *
 * Revision 2.1  2008-07-28 23:25:58+05:30  Cprogrammer
 * program to copy email directly to user's home directory
 *
 */
#include "indimail.h"
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: copyemail.c,v 2.2 2008-11-21 16:02:56+05:30 Cprogrammer Stab mbhangui $";
#endif

int             get_options(int , char **);
char            homedir[MAXPATHLEN];
char            fname[MAX_BUFF];
char            email[MAX_BUFF];
char            To[MAX_BUFF];
char            From[MAX_BUFF];
char            Subject[MAX_BUFF];
int             setDate = 0;

int
main(int argc, char **argv)
{
	struct stat     statbuf;

	if (get_options(argc, argv))
		return 1;
	if (stat(fname, &statbuf))
	{
		fprintf(stderr, "copyemail: stat on file %s failed:%s", fname, strerror(errno));
		return 1;
	}
	if (CopyEmailFile(homedir, fname, email, To, From, Subject, setDate, 2, statbuf.st_size))
	{
		fprintf(stderr,"copyemail: Failed to copy the file as email\n");
		return 1;
	}
	return 0;
}

void usage()
{
	fprintf(stderr, "usage: copyemail [options] filename\n"); 
	fprintf(stderr, "options: -h      homedir where the mail is to be copied\n");
	fprintf(stderr, "         -e      To email address\n");
	fprintf(stderr, "         -T      User name\n");
	fprintf(stderr, "         -F      From address\n");
	fprintf(stderr, "         -S      Subject\n");
	fprintf(stderr, "         -d      Set date\n");
	return;
}


int
get_options(int argc, char **argv)
{
	int             errflag = 0, c;
	memset(homedir, 0, MAXPATHLEN);
	memset(fname, 0, MAX_BUFF);
	memset(email, 0, MAX_BUFF);
	memset(To, 0, MAX_BUFF);
	memset(From, 0, MAX_BUFF);
	memset(Subject, 0, MAX_BUFF);

	while (!errflag && (c = getopt(argc, argv, "dh:f:e:T:F:S:")) != -1)
	{
		switch (c)
		{
		case 'h':
			scopy(homedir, optarg, MAXPATHLEN);
			break;
		case 'e':
			scopy(email, optarg, MAX_BUFF);
			break;
		case 'T':
			scopy(To, optarg, MAX_BUFF);
			break;
		case 'F':
			scopy(From, optarg, MAX_BUFF);
			break;
		case 'S':
			scopy(Subject, optarg, MAX_BUFF);
			break;
		case 'd':
			setDate = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag)
	{
		usage();
		return 1;
	}
	if (optind < argc)
		scopy(fname, argv[optind++], MAX_BUFF);
	if (!*fname)
	{
		fprintf(stderr, " Missing file name\n");
		usage();
		return 1;
	}
	if (!*homedir)
	{
		fprintf(stderr, " Invalid homedir -h\n");
		usage();
		return 1;
	}
	return 0;
}

void
copyemail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
