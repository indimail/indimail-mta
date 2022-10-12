/*
 * $Log: srs.h,v $
 * Revision 1.2  2022-10-12 19:15:32+05:30  Cprogrammer
 * added srs_setup() function
 *
 * Revision 1.1  2022-03-26 10:16:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SRS_H
#define SRS_H
#include <stralloc.h>

extern stralloc srs_result;
extern stralloc srs_error;
extern int      srs_setup(int);
extern int      srsforward(char *);
extern int      srsreverse(char *);

#endif
