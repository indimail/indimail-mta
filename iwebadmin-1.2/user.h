/*
 * $Id: user.h,v 1.1 2010-04-26 12:08:11+05:30 Cprogrammer Exp mbhangui $
 */

void            adduser();
void            addusernow();
void            bounceall();
int             call_hooks(char *hook_type, char *p1, char *p2, char *p3, char *p4);
void            count_users();
void            deleteall();
void            deluser();
void            delusergo();
void            delusernow();
int             get_catchall();
void            moduser();
void            modusergo();
void            modusernow();
void            parse_users_dotqmail(char newchar);
void            setremotecatchall();
void            setremotecatchallnow();
void            show_users(char *Username, char *Domain, time_t Mytime);
int             show_user_lines(char *user, char *dom, time_t mytime, char *dir);
