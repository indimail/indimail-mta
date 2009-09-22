/*
 * $Log: sig.h,v $
 * Revision 1.5  2009-06-04 12:09:02+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.4  2004-10-09 23:31:44+05:30  Cprogrammer
 * added new function definitions for daemontools inclusion
 *
 * Revision 1.3  2004-10-09 19:21:53+05:30  Cprogrammer
 * added sig_ignore() and sig_uncatch()
 *
 * Revision 1.2  2004-06-18 23:01:50+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef SIG_H
#define SIG_H

void            sig_catch(int, void(*)());
void            sig_block(int);
void            sig_unblock(int);
void            sig_blocknone(void);
void            sig_pause(void);

void            sig_dfl();

void            sig_miscignore(void);
void            sig_bugcatch(void (*)());

void            sig_pipeignore(void);
void            sig_pipedefault(void);

void            sig_contblock();
void            sig_contunblock();
void            sig_contcatch();
void            sig_contdefault();

void            sig_termblock(void);
void            sig_termunblock(void);
void            sig_termcatch(void (*)());
void            sig_termdefault(void);

void            sig_alarmblock(void);
void            sig_alarmunblock(void);
void            sig_alarmcatch(void (*)());
void            sig_alarmdefault(void);

void            sig_childblock(void);
void            sig_childunblock(void);
void            sig_childcatch(void (*)());
void            sig_childdefault(void);

void            sig_hangupblock(void);
void            sig_hangupunblock(void);
void            sig_hangupcatch(void (*)());
void            sig_hangupdefault(void);

int             sig_alarm;
int             sig_child;
int             sig_cont;
int             sig_hangup;
int             sig_int;
int             sig_pipe;
int             sig_term;

void            (*sig_defaulthandler) ();
void            (*sig_ignorehandler) ();
#define sig_ignore(s) (sig_catch((s),sig_ignorehandler))
#define sig_uncatch(s) (sig_catch((s),sig_defaulthandler))

#endif
