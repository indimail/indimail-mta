/*
 * $Log: srs.h,v $
 * Revision 1.1  2022-03-26 10:16:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SRS_H
#define SRS_H

extern stralloc srs_result;
extern stralloc srs_error;
extern int      srsforward(char *);
extern int      srsreverse(char *);

#endif
