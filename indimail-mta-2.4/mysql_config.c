/*
 * $Log: mysql_config.c,v $
 * Revision 1.5  2016-05-15 22:40:33+05:30  Cprogrammer
 * use execvp() to locate mysql_config
 *
 * Revision 1.4  2011-05-07 16:00:33+05:30  Cprogrammer
 * fix compiler warnings
 *
 * Revision 1.3  2011-04-29 12:49:57+05:30  Cprogrammer
 * prevent recursive call to mysql_config
 *
 * Revision 1.2  2009-07-11 08:55:52+05:30  Cprogrammer
 * use /usr/local/mysql as the first mysql prefix
 *
 */
#ifdef MYSQL_CONFIG
#include <unistd.h>
int
main(int argc, char **argv)
{
	execv("/usr/bin/mysql_config", argv);
	execv("/usr/local/mysql/bin/mysql_config", argv);
	execvp("mysql_config", argv);
	return (1);
}
#else
int
main(int argc, char **argv)
{
	return (0);
}
#endif
