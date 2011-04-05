/*
 * $Log: vcalias.c,v $
 * Revision 2.10  2009-02-18 09:08:24+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.9  2009-02-06 11:40:43+05:30  Cprogrammer
 * ignore return value of fgets
 *
 * Revision 2.8  2008-09-08 09:56:41+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.7  2008-05-28 15:25:31+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 2.6  2004-09-20 19:55:09+05:30  Cprogrammer
 * fixed compilation warning for isspace()
 *
 * Revision 2.5  2004-09-20 19:35:49+05:30  Cprogrammer
 * skip comments
 *
 * Revision 2.4  2004-05-03 22:05:43+05:30  Cprogrammer
 * replace ':' with '.' in qmail files
 *
 * Revision 2.3  2002-10-27 21:35:03+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-05 01:12:55+05:30  Cprogrammer
 * added mysql_escape()
 *
 * Revision 2.1  2002-08-03 04:37:17+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.8  2002-02-23 20:25:03+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.7  2001-12-22 18:31:03+05:30  Cprogrammer
 * create table valias if query fails
 *
 * Revision 1.6  2001-12-03 01:57:49+05:30  Cprogrammer
 * changed insert to low priority
 *
 * Revision 1.5  2001-12-02 01:12:42+05:30  Cprogrammer
 * improved formatting
 *
 * Revision 1.4  2001-11-24 12:21:07+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:59:51+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:27:28+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:26+05:30  Cprogrammer
 * Initial revision
 *
 *
 * convert .qmail alias files into valias/mysql table
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vcalias.c,v 2.10 2009-02-18 09:08:24+05:30 Cprogrammer Stab mbhangui $";
#endif

#if defined(VALIAS)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <mysqld_error.h>
#include <sys/types.h>


int
main(int argc, char **argv)
{
	DIR            *entry;
	struct dirent  *dp;
	FILE           *fp;
	char           *ptr, *Domain, *AliasName;
	uid_t           Uid;
	gid_t           Gid;
	int             err, flag;
	char            buffer[MAX_ALIAS_LINE], Dir[MAX_BUFF], SqlBuf[SQL_BUF_SIZE];

	/*
	 * get the command line arguments 
	 */
	 if (argc != 2)
	 {
	 	if ((ptr = strrchr(argv[0], '/')) != NULL)
			ptr++;
		else
			ptr = argv[0];
		fprintf(stderr, "Usage: %s domain\n", ptr);
		return(1);
	 }
	 Domain = argv[1];
	/*
	 * find the directory 
	 */
	if (vget_assign(Domain, Dir, MAX_BUFF, &Uid, &Gid) == NULL)
	{
		printf("could not find domain in qmail assign file\n");
		return(-1);
	}
	printf("Looking in %s\n", Dir);
	if (chdir(Dir))
	{
		perror(Dir);
		return(-1);
	}
	/*
	 * open the directory 
	 */
	if (!(entry = opendir(Dir)))
	{
		fprintf(stderr, "opendir:%s: %s\n", Dir, strerror(errno));
		return (-1);
	}
	/*
	 * search for .qmail files 
	 */
	printf("Converting from Filesystem to MYSQL for domain %s\n", Domain);
	for (;;)
	{
		if (!(dp = readdir(entry)))
			break;
		else
		if (!strncmp(dp->d_name, ".qmail-default", 14))
			continue;
		else
		if (strncmp(dp->d_name, ".qmail-", 7))
			continue;
		/*- printf("Converting %-20s", dp->d_name + 7); -*/
		AliasName= dp->d_name + 7;
		if ((fp = fopen(dp->d_name, "r")) == NULL)
		{
			perror(dp->d_name);
			continue;
		}
		for (err = 0, flag = 0;;flag++)
		{
			if (!fgets(buffer, MAX_ALIAS_LINE - 1, fp))
			{
				if (feof(fp))
					break;
				perror("fgets");
				fclose(fp);
				vclose();
				return (-1);
			}
			if ((ptr = strrchr(buffer, '\n')) || (ptr = strchr(buffer, '#')))
				*ptr = 0;
			for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			if (!flag)
			{
				snprintf(Dir, MAX_BUFF, "%s@%s", dp->d_name + 7, Domain);
				printf("Converting %-30s -> %s\n", Dir, buffer);
			} else
				printf("           %-30s -> %s\n", " ", buffer);
			/*- Convert ':' to '.' */
			for (ptr = AliasName;*ptr;ptr++)
			{
				if (*ptr == ':')
					*ptr = '.';
			}
			snprintf(SqlBuf, SQL_BUF_SIZE, 
				"insert low_priority into valias  ( alias, domain, valias_line ) values ( \"%s\", \"%s\", \"%s\")", 
				AliasName, Domain, buffer);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
				{
					if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
						return(1);
					if (!mysql_query(&mysql[1], SqlBuf))
						continue;
				}
				fprintf(stderr, "vcalias: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
				err = 1;
				break;
			}
		} /*- for (err = 0, flag = 0;;flag++) */
		fclose(fp);
		if (!err)
			unlink(dp->d_name);
	}
	vclose();
	return(0);
}

#else
int
main()
{
	printf("IndiMail not configured with --enable-mysql=y and --enable-valias=y\n");
	return(0);
}
#endif

void
getversion_vcalias_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
