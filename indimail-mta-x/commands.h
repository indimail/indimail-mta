/*
 * $Log: commands.h,v $
 * Revision 1.3  2004-10-11 13:51:32+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:58:14+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef COMMANDS_H
#define COMMANDS_H
#include "substdio.h"

struct commands
{
	char           *text;
	void            (*fun) ();
	void            (*flush) ();
};

int             commands(substdio *, struct commands *);

#endif
