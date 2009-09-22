/*
 * $Log: mysql_config.c,v $
 * Revision 1.2  2009-07-11 08:55:52+05:30  Cprogrammer
 * use /usr/local/mysql as the first mysql prefix
 *
 */
#include <unistd.h>

int
main(int argc, char **argv)
{
#ifdef MYSQL_CONFIG
	if (!access("/usr/local/mysql/bin/mysql_config", X_OK))
		execv("/usr/local/mysql/bin/mysql_config", argv);
	execvp("mysql_config", argv);
#endif
	return(1);
}
