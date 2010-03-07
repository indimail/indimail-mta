/*
 * $Log: create_table.c,v $
 * Revision 2.7  2010-03-07 11:07:25+05:30  Cprogrammer
 * display database as "master" in error message
 *
 * Revision 2.6  2008-05-28 16:34:26+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2008-05-27 22:19:12+05:30  Cprogrammer
 * for non-cluster domain exit if table is meant for master
 *
 * Revision 2.4  2003-12-07 00:26:03+05:30  Cprogrammer
 * use layout() for getting the template for a table
 *
 * Revision 2.3  2003-10-16 00:00:30+05:30  Cprogrammer
 * added printing of sql query
 *
 * Revision 2.2  2002-11-28 00:44:09+05:30  Cprogrammer
 * correction for non clustered compilation
 *
 * Revision 2.1  2002-10-27 21:52:45+05:30  Cprogrammer
 * generic function to create tables
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: create_table.c,v 2.7 2010-03-07 11:07:25+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>

int
create_table(int which, char *table, char *_template)
{
	char           *SqlBuf, *template;
	int             len;

	if (!table || !*table)
		return(-1);
	if (!(template = (!_template || !*_template) ? layout(table) : _template))
	{
		fprintf(stderr, "Invalid template for table %s\n", table);
		return(-1);
	}
	if (which != ON_MASTER && which != ON_LOCAL)
		return(-1);
#ifdef CLUSTERED_SITE
	if ((which == ON_MASTER ? open_master() : vauth_open((char *) 0)))
	{
		fprintf(stderr, "create_table: Failed to open %s db\n",
			which == ON_MASTER ? "master" : "local");
		return(-1);
	}
#else
	if (which == ON_MASTER)
		return(0);
	if (vauth_open((char *) 0))
	{
		fprintf(stderr, "create_table: Failed to open %s\n", which == ON_MASTER ? "central" : "local");
		return(-1);
	}
#endif
	len = slen(table) + slen(template) + 33;
	if (!(SqlBuf = (char *) malloc(len)))
	{
		fprintf(stderr, "create_table: malloc: %d bytes: %s\n", len, strerror(errno));
		return(-1);
	}
	snprintf(SqlBuf, len, "create table IF NOT EXISTS %s ( %s )", table, template);
	if (mysql_query(which == ON_MASTER ? &mysql[0] : &mysql[1], SqlBuf))
	{
		fprintf(stderr, "create_table: %s: %s\nQuery: %s\n", table, mysql_error(which == ON_MASTER ? &mysql[0] : &mysql[1]), SqlBuf);
		free(SqlBuf);
		return(-1);
	}
	free(SqlBuf);
	return(0);
}

void
getversion_create_table_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
