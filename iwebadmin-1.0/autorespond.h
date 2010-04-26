/*
 * $Id: autorespond.h,v 1.1.2.1 2004/11/20 01:10:41 tomcollins Exp $
 */

#include <time.h>

void addautorespond();
void addautorespondnow();
void count_autoresponders();
void delautorespond();
void delautorespondnow();
void modautorespond();
void modautorespondnow();
void show_autoresponders(char *user, char *dom, time_t mytime);
void show_autorespond_line(char *user, char *dom, time_t mytime, char *dir);
