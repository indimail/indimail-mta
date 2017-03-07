/*
 * $Id: dotqmail.h,v 1.1 2010-04-26 12:07:47+05:30 Cprogrammer Exp mbhangui $
 */

int             dotqmail_delete_files(char *user);
int             dotqmail_add_line(char *user, char *line);
int             dotqmail_del_line(char *user, char *line);
#ifndef VALIAS
int             dotqmail_open_files(char *user);
void            dotqmail_close_files(char *user, int keep);
int             dotqmail_cleanup(char *user, char *line);
int             dotqmail_count(char *user);
#endif
