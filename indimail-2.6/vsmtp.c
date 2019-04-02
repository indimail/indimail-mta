/*
 * $Log: vsmtp.c,v $
 * Revision 2.8  2019-04-02 11:00:17+05:30  Cprogrammer
 * removed unused variable tmpbuf
 *
 * Revision 2.7  2018-09-11 15:12:03+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.6  2011-11-09 19:46:40+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.5  2010-05-20 10:58:07+05:30  Cprogrammer
 * error messages on stderr instead of stdout
 *
 * Revision 2.4  2004-07-03 23:56:17+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.3  2003-03-05 00:21:20+05:30  Cprogrammer
 * return success if no routes are configured
 *
 * Revision 2.2  2003-01-28 23:29:41+05:30  Cprogrammer
 * changes for tcl/tk admin module
 *
 * Revision 2.1  2002-05-10 10:10:23+05:30  Cprogrammer
 * error messages made more meaningful
 *
 * Revision 1.6  2002-03-29 11:26:09+05:30  Cprogrammer
 * improved display formatting
 *
 * Revision 1.5  2002-02-24 22:47:16+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.4  2001-12-27 01:31:38+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.3  2001-12-13 01:54:06+05:30  Cprogrammer
 * added mta option
 *
 * Revision 1.2  2001-12-10 00:17:07+05:30  Cprogrammer
 * added update option
 *
 * Revision 1.1  2001-12-09 23:48:35+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp.c,v 2.8 2019-04-02 11:00:17+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

char            MdaHost[MAX_BUFF];
char            Host[MAX_BUFF];
char            Domain[MAX_BUFF];
char            mta[MAX_BUFF];
int             Port;

#define SMTP_SELECT 0
#define SMTP_INSERT 1
#define SMTP_DELETE 2
#define SMTP_UPDATE 3

int             SmtpAction;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *tmpsmtp, *ptr, *cptr;
	char            srchost[MAX_BUFF], dsthost[MAX_BUFF];
	int             OldPort, err;

	if(get_options(argc, argv))
		return(0);
	err = 0;
	switch (SmtpAction)
	{
	case SMTP_SELECT:
		for(err = 1, OldPort = 0;;OldPort++)
		{
			if(!(tmpsmtp = vsmtp_select(Domain, &Port)))
			{
				if(!OldPort)
					return(0);
				break;
			}
			for(cptr = srchost, ptr = tmpsmtp;*ptr && *ptr != ' ';*cptr++ = *ptr++);
			*cptr = 0;
			for(;*ptr && *ptr == ' ';ptr++);
			for(cptr = dsthost;*ptr;*cptr++ = *ptr++);
			*cptr = 0;
			if(*mta && strncmp(mta, srchost, MAX_BUFF))
				continue;
			if(*Host && strncmp(Host, dsthost, MAX_BUFF))
				continue;
			if(!OldPort)
				printf("Source Host          Destination Host -> Port\n");
			printf("%-20s %-16s -> %d\n", srchost, dsthost, Port);
			err = 0;
		}
		break;
	case SMTP_INSERT:
		if(SmtpAction == SMTP_INSERT)
			err = vsmtp_insert(Host, mta, Domain, Port);
		break;
	case SMTP_DELETE:
			err = vsmtp_delete(Host, mta, Domain, Port);
		break;
	case SMTP_UPDATE:
			if((OldPort = get_smtp_service_port(mta, Domain, Host)) == -1)
			{
				fprintf(stderr, "failed to get Old Port\n");
				err = 1;
			} else
				err = vsmtp_update(Host, mta, Domain, OldPort, Port);
		break;
	default:
		fprintf(stderr, "error, Smtp Action is invalid %d\n", SmtpAction);
		err = 1;
		break;
	}
	return(err);
}

void
usage()
{
	fprintf(stderr, "usage: vsmtp [options] [[-d|-i port|-u port] -m mta host@domain_name]\n");
	fprintf(stderr, "       vsmtp [options] [-s domain_name]\n");
	fprintf(stderr, "       vsmtp [options] [-s host@domain_name]\n");
	fprintf(stderr, "       vsmtp [options] [-s -m mta host@domain_name]\n");
	fprintf(stderr, "options: -V ( print version number )\n");
	fprintf(stderr, "         -v ( verbose )\n");
	fprintf(stderr, "         -s domain_name ( show smtp ports )\n");
	fprintf(stderr, "         -d ( delete smtp ports )\n");
	fprintf(stderr, "         -i port (insert smtp port)\n");
	fprintf(stderr, "         -u port (update smtp port)\n");
	fprintf(stderr, "         -m mta_ipaddress\n");
}

int
get_options(int argc, char **argv)
{
	int             c;

	memset(Host, 0, MAX_BUFF);
	memset(MdaHost, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	memset(mta, 0, MAX_BUFF);
	Port = 0;
	SmtpAction = SMTP_SELECT;
	while ((c = getopt(argc, argv, "vsdi:u:m:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			SmtpAction = SMTP_SELECT;
			break;
		case 'd':
			SmtpAction = SMTP_DELETE;
			break;
		case 'i':
			SmtpAction = SMTP_INSERT;
			Port = atoi(optarg);
			break;
		case 'u':
			SmtpAction = SMTP_UPDATE;
			Port = atoi(optarg);
			break;
		case 'm':
			scopy(mta, optarg, MAX_BUFF);
			break;
		default:
			usage();
			return(1);
		}
	}
	if (optind < argc)
	{
		scopy(MdaHost, argv[optind++], MAX_BUFF);
		if(strchr(MdaHost, '@'))
		{
			if (parse_email(MdaHost, Host, Domain, MAX_BUFF))
			{
				fprintf(stderr, "%s: Email too long\n", MdaHost);
				return (1);
			}
		} else
			scopy(Domain, MdaHost, MAX_BUFF);
	}
	if (!*MdaHost)
	{
		if(SmtpAction == SMTP_SELECT)
			fprintf(stderr, "vsmtp: must supply host@domain or domain_name\n");
		else
			fprintf(stderr, "vsmtp: must supply host@domain\n");
		usage();
		return(1);
	} else
	if(!*mta && SmtpAction != SMTP_SELECT)
	{
		fprintf(stderr, "vsmtp: must supply MTA IP Address\n");
		usage();
		return(1);
	}
	return(0);
}
#else
int
main()
{
	printf("IndiMail not configured with --enable-user-cluster=y\n");
	return(0);
}
#endif

void
getversion_vsmtp_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
