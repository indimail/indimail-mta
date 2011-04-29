/*
 * $Log: mysql_config.c,v $
 * Revision 1.3  2011-04-29 12:49:57+05:30  Cprogrammer
 * prevent recursive call to mysql_config
 *
 * Revision 1.2  2009-07-11 08:55:52+05:30  Cprogrammer
 * use /usr/local/mysql as the first mysql prefix
 *
 */

int
main(int argc, char **argv)
{
#ifdef MYSQL_CONFIG
	execvp("/usr/bin/mysql_config", argv);
	execv("/usr/local/mysql/bin/mysql_config", argv);
#endif
	return(1);
}
