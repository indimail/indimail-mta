/*
 * $Log: set_mysql_options.c,v $
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
static char     sccsid[] = "$Id: set_mysql_options.c,v 2.5 2009-11-09 08:34:36+05:30 Cprogrammer Exp mbhangui $";
#endif

int
set_mysql_options(MYSQL *mysql, char *file, char *group)
{
	char           *default_file, *default_group, *connect_timeout, 
				   *read_timeout, *write_timeout, *init_cmd,
				   *set_client_ip, *opt_reconnect, *opt_protocol;
	char            temp[4];
	unsigned int    protocol;

	getEnvConfigStr(&init_cmd, "MYSQL_INIT_COMMAND", 0);
	getEnvConfigStr(&default_file, "MYSQL_READ_DEFAULT_FILE", file);
	getEnvConfigStr(&default_group, "MYSQL_READ_DEFAULT_GROUP", group);
	getEnvConfigStr(&connect_timeout, "MYSQL_OPT_CONNECT_TIMEOUT", "120");
	getEnvConfigStr(&read_timeout, "MYSQL_OPT_READ_TIMEOUT", "20");
	getEnvConfigStr(&write_timeout, "MYSQL_OPT_WRITE_TIMEOUT", "20");
	getEnvConfigStr(&set_client_ip, "MYSQL_SET_CLIENT_IP", 0);
	getEnvConfigStr(&opt_reconnect, "MYSQL_OPT_RECONNECT", "0");
	snprintf(temp, sizeof(temp) - 1, "%d", MYSQL_PROTOCOL_DEFAULT);
	getEnvConfigStr(&opt_protocol, "MYSQL_OPT_PROTOCOL", temp);
	protocol = atoi(opt_protocol);
	if (init_cmd && mysql_options(mysql, MYSQL_INIT_COMMAND, init_cmd))
		return (1);
	if (mysql_options(mysql, MYSQL_READ_DEFAULT_FILE, default_file))
		return (1);
	if (mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, default_group))
		return (1);
	if (mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, connect_timeout))
		return (1);
	if (mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, read_timeout))
		return (1);
	if (mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, write_timeout))
		return (1);
	if (getenv("MYSQL_SET_CLIENT_IP") && 
			mysql_options(mysql, MYSQL_SET_CLIENT_IP, set_client_ip))
		return (1);
	if (getenv("MYSQL_OPT_RECONNECT") &&
			mysql_options(mysql, MYSQL_OPT_RECONNECT, opt_reconnect))
		return (1);
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
