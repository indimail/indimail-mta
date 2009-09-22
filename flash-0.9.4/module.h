/*
 * $Log: module.h,v $
 * Revision 1.1  2002-12-16 01:55:35+05:30  Manny
 * Initial revision
 *
 */
#if !defined (_MODULE_H)
#define _MODULE_H

void            InitBackground(int, char **, FILE *, int *);
void            InitBarClock(int, char **, FILE *, int *);
void            InitMailCheck(int, char **, FILE *, int *);
void            InitCountdown2000(int, char **, FILE *, int *);
void            module(int, char **, FILE *, int *);
void            RunModuleFunction(int, char **);

#endif
