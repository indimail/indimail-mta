/*
 * $Id: alias.h,v 1.1 2010-04-26 12:08:19+05:30 Cprogrammer Exp mbhangui $
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
