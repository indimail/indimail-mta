/*
 * $Log: env.h,v $
 * Revision 1.5  2009-05-03 22:46:20+05:30  Cprogrammer
 * restore_env() now returns void
 *
 * Revision 1.4  2004-10-11 13:53:08+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-09-26 00:03:19+05:30  Cprogrammer
 * added restore_env()
 *
 * Revision 1.2  2004-06-18 22:58:29+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef ENV_H
#define ENV_H

int             env_isinit;
extern char   **environ;

int             env_init(void);
int             env_put(char *);
int             env_put2(char *, char *);
int             env_unset(char *);
void            restore_env(void);
char           *env_get(char *);
char           *env_pick(void);
void            env_clear(void);
char           *env_findeq(char *);

#endif
