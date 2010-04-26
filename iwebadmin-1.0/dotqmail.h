/*
 * $Id: dotqmail.h,v 1.1.2.1 2004/11/20 01:10:41 tomcollins Exp $
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
