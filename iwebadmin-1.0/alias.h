/*
 * $Id: alias.h,v 1.1.2.1 2004/11/20 01:10:41 tomcollins Exp $
 */

void            adddotqmail();
void            adddotqmailnow();
int             adddotqmail_shared(char *forwardname, char *dest, int create);
void            deldotqmail();
void            deldotqmailnow();
char           *dotqmail_alias_command(char *command);
void            moddotqmail();
void            moddotqmailnow();
int             show_aliases(void);
void            show_dotqmail_lines(char *user, char *dom, time_t mytime);
void            show_dotqmail_file(char *user);
