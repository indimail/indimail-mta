/*
 * $Id: autorespond.h,v 1.1 2010-04-26 12:07:40+05:30 Cprogrammer Exp mbhangui $
 */

#include <time.h>

void            addautorespond();
void            addautorespondnow();
void            count_autoresponders();
void            delautorespond();
void            delautorespondnow();
void            modautorespond();
void            modautorespondnow();
void            show_autoresponders(char *user, char *dom, time_t mytime);
void            show_autorespond_line(char *user, char *dom, time_t mytime, char *dir);
