/*
 * $Log: set_mysql_options.c,v $
 * Revision 2.9  2014-01-02 23:54:03+05:30  Cprogrammer
 * set delayed_insert variable for MYSQL delayed insert
 *
 * Revision 2.8  2010-04-15 14:14:25+05:30  Cprogrammer
 * set client flags for mysql_real_connect()
 *
 * Revision 2.7  2010-04-15 12:47:36+05:30  Cprogrammer
 * corrected data type of 3rd argument of mysql_options()
 *
 * Revision 2.6  2009-11-09 10:43:01+05:30  Cprogrammer
 * added comment for MYSQL_OPT_PROTOCOL enum values
 *
 * Revision 2.5  2009-11-09 08:34:36+05:30  Cprogrammer
 * added option to set MYSQL_OPT_PROTOCOL
 *
 * Revision 2.4  2009-03-16 10:36:42+05:30  Cprogrammer
 * added MYSQL_SET_CLIENT_IP, MYSQL_OPT_RECONNECT
 *
 * Revision 2.3  2009-03-16 09:38:40+05:30  Cprogrammer
 * added MYSQL_OPT_READ_TIMEOUT, MYSQL_OPT_WRITE_TIMEOUT
 *
 * Revision 2.2  2008-05-28 17:46:24+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2006-03-17 14:43:49+05:30  Cprogrammer
 * Initial Version
 *
 */
#include <stdlib.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: set_mysql_options.c,v 2.9 2014-01-02 23:54:03+05:30 Cprogrammer Exp mbhangui $";
#endif

int
set_mysql_options(MYSQL *mysql, char *file, char *group, unsigned int *flags)
{
	char           *default_file, *default_group, *c_timeout, 
				   *r_timeout, *w_timeout, *init_cmd,
				   *set_client_ip, *opt_reconnect, *opt_protocol;
	char            temp[4];
	char            o_reconnect;
	unsigned int    protocol, connect_timeout, read_timeout, write_timeout;

	*flags = 0;
	if (getenv("CLIENT_COMPRESS"))
		*flags += CLIENT_COMPRESS;
	if (getenv("CLIENT_INTERACTIVE"))
		*flags += CLIENT_INTERACTIVE;
#if 0
	if (getenv("CLIENT_FOUND_ROWS"))
		*flags += CLIENT_FOUND_ROWS;
	if (getenv("CLIENT_IGNORE_SPACE"))
		*flags += CLIENT_IGNORE_SPACE;
	if (getenv("CLIENT_NO_SCHEMA"))
		*flags += CLIENT_NO_SCHEMA;
	if (getenv("CLIENT_ODBC"))
		*flags += CLIENT_ODBC;
	if (getenv("CLIENT_TRANSACTIONS"))
		*flags += CLIENT_TRANSACTIONS;
#endif
	getEnvConfigStr(&init_cmd, "MYSQL_INIT_COMMAND", 0);
	getEnvConfigStr(&default_file, "MYSQL_READ_DEFAULT_FILE", file);
	getEnvConfigStr(&default_group, "MYSQL_READ_DEFAULT_GROUP", group);
	getEnvConfigStr(&c_timeout, "MYSQL_OPT_CONNECT_TIMEOUT", "120");
	getEnvConfigStr(&r_timeout, "MYSQL_OPT_READ_TIMEOUT", "20");
	getEnvConfigStr(&w_timeout, "MYSQL_OPT_WRITE_TIMEOUT", "20");
	getEnvConfigStr(&set_client_ip, "MYSQL_SET_CLIENT_IP", 0);
	getEnvConfigStr(&opt_reconnect, "MYSQL_OPT_RECONNECT", "0");
	snprintf(temp, sizeof(temp) - 1, "%d", MYSQL_PROTOCOL_DEFAULT);
	getEnvConfigStr(&opt_protocol, "MYSQL_OPT_PROTOCOL", temp);
	delayed_insert = ((char *) getenv("DELAYED_INSERT") ? 1 : 0);
	protocol = atoi(opt_protocol);
	o_reconnect = atoi(opt_reconnect);
	connect_timeout = atoi(c_timeout);
	read_timeout = atoi(r_timeout);
	write_timeout = atoi(w_timeout);
	if (init_cmd && mysql_options(mysql, MYSQL_INIT_COMMAND, init_cmd))
		return (1);
	if (mysql_options(mysql, MYSQL_READ_DEFAULT_FILE, default_file))
		return (1);
	if (mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, default_group))
		return (1);
	if (mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &connect_timeout))
		return (1);
	if (mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, (char *) &read_timeout))
		return (1);
	if (mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, (char *) &write_timeout))
		return (1);
	if (getenv("MYSQL_SET_CLIENT_IP") && 
			mysql_options(mysql, MYSQL_SET_CLIENT_IP, set_client_ip))
		return (1);
	if (getenv("MYSQL_OPT_RECONNECT") &&
			mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *) &o_reconnect))
		return (1);
	/*-
	 * enum mysql_protocol_type 
	 * MYSQL_PROTOCOL_DEFAULT, MYSQL_PROTOCOL_TCP, MYSQL_PROTOCOL_SOCKET,
	 * MYSQL_PROTOCOL_PIPE, MYSQL_PROTOCOL_MEMORY
	 */
	if (mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char *) &protocol))
		return (1);
	return (0);
}

void
getversion_set_mysql_options_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
