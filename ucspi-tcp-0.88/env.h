/*
 * $Log: env.h,v $
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef ENV_H
#define ENV_H

extern char **environ;

extern /*@null@*/char *env_get(char *);

#endif
