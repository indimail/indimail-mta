/*
 * $Log: commands.h,v $
 * Revision 1.2  2005-05-13 23:44:28+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef COMMANDS_H
#define COMMANDS_H

struct commands
{
	char           *verb;
	void            (*action) (char *);
	void            (*flush) (void);
};

int             commands(substdio *, struct commands *);

#endif
