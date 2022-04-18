/*
 * $Log: trigger.h,v $
 * Revision 1.4  2022-01-30 09:45:30+05:30  Cprogrammer
 * added trigger_clear() function
 *
 * Revision 1.3  2004-10-11 14:16:06+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:02:22+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef TRIGGER_H
#define TRIGGER_H

void            trigger_set();
void            trigger_selprep(int *, fd_set *);
void            trigger_clear(int *, fd_set *);
int             trigger_pulled(fd_set *);

#endif
