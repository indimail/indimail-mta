/*
 * $Id: mailinglist.h,v 1.2 2017-04-03 13:03:41+05:30 Cprogrammer Exp mbhangui $
 */

#include <time.h>

void            addlistgroup(char *template);
void            addlistgroupnow(int mod);
void            addmailinglist();
void            addmailinglistnow();
void            count_mailinglists();
void            dellistgroup(char *template);
void            dellistgroupnow(int mod);
void            delmailinglist();
void            delmailinglistnow();
int             ezmlm_sub(int mod, char *email);
void            modmailinglist();
void            modmailinglistnow();
void            show_list_group(char *template);
void            show_list_group_now(int mod);
void            show_mailing_lists(char *user, char *dom, time_t mytime);
void            show_mailing_list_line(char *user, char *dom, time_t mytime, char *dir);
void            show_mailing_list_line2(char *user, char *dom, time_t mytime, char *dir);
void            show_list_group_now(int mod);
void            show_current_list_values();
int             get_mailinglist_prefix(char *prefix);
