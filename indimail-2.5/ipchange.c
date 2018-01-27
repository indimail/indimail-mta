/*
 * $Log: ipchange.c,v $
 * Revision 2.4  2008-09-08 09:44:53+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-06-13 08:59:56+05:30  Cprogrammer
 * fixed compilation error if CLUSTERED_SITE was not defined
 *
 * Revision 2.2  2004-05-28 09:42:45+05:30  Cprogrammer
 * modified usage() descriptions
 *
 * Revision 2.1  2004-05-22 23:45:40+05:30  Cprogrammer
 * IP address maintanance utility
 *
 */
#include <indimail.h>

#ifndef	lint
static char     sccsid[] = "$Id: ipchange.c,v 2.4 2008-09-08 09:44:53+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void            usage();
int             get_options(int argc, char **argv, char **, char **, char **, char **, int *);

int
main(int argc, char **argv)
{
	char           *old_ip, *new_ip, *table_name, *column_name;
	char            SqlBuf[SQL_BUF_SIZE];
	int             which, err;

	if (get_options(argc, argv, &old_ip, &new_ip, &table_name, &column_name, &which))
		return (1);
	snprintf(SqlBuf, sizeof(SqlBuf), 
		"update low_priority %s set %s=\"%s\" where %s=\"%s\"",
		table_name, column_name, new_ip, column_name, old_ip);
	if ((which == ON_MASTER ? open_master() : vauth_open((char *) 0)))
	{
		fprintf(stderr, "ipchange: Failed to open %s db\n", which == ON_MASTER ? "master" : "local");
		return (-1);
	}
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf))
	{
		fprintf(stderr, "ipchange: %s: %s\nQuery: %s\n", table_name, mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]), SqlBuf);
		return (-1);
	}
	if ((err = mysql_affected_rows(which == ON_MASTER ? &mysql[0] : &mysql[1])) == -1)
	{
		fprintf(stderr, "ipchange: %s\n", mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]));
		return (-1);
	}
	if (verbose)
		printf("%d rows affected\n", err);
	return (err ? 0 : 1);
}

void
usage()
{
	fprintf(stderr, "usage: ipchange [options] table_name\n");
	fprintf(stderr, "         -v           ( verbose )\n");
	fprintf(stderr, "         -o old_ip    ( old IP Address )\n");
	fprintf(stderr, "         -n new_ip    ( new IP Address )\n");
	fprintf(stderr, "         -c col_name  ( Column Name )\n");
	fprintf(stderr, "         -m           (table is on hostcntrl)\n");
}

int
get_options(int argc, char **argv, char **old_ip, char **new_ip,
	char **table_name, char **column_name, int *which)
{
	int             c;

	*column_name = *old_ip = *new_ip = *table_name = 0;
	*which = ON_LOCAL;
	verbose = 0;
	while ((c = getopt(argc, argv, "vmc:o:n:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'm':
			*which = ON_MASTER;
			break;
		case 'c':
			*column_name = optarg;
			break;
		case 'o':
			*old_ip = optarg;
			break;
		case 'n':
			*new_ip = optarg;
			break;
		default:
			usage();
			return (1);
		}
	}
	if (optind < argc)
		*table_name = argv[optind++];
	if (!*old_ip || !*new_ip || !*table_name || !*column_name)
	{
		usage();
		return (1);
	}
	return (0);
}
#else
int
main()
{
	fprintf(stderr, "IndiMail not configured with --enable-user-cluster=y\n");
	return (0);
}
#endif

void
getversion_ipchange_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
