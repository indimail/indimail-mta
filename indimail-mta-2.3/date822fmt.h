/*
 * $Log: date822fmt.h,v $
 * Revision 1.3  2004-10-11 13:51:54+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:58:18+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef DATE822FMT_H
#define DATE822FMT_H
#include "datetime.h"
#define DATE822FMT 60

unsigned int    date822fmt(char *, struct datetime *);

#endif
