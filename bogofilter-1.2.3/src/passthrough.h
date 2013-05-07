/* $Id: passthrough.h 6654 2007-02-26 00:25:29Z relson $ */

/*****************************************************************************

NAME:
   passthrough.h -- prototypes and definitions for passthrough.c

******************************************************************************/

#ifndef	PASSTHROUGH_H
#define	PASSTHROUGH_H

/* in main.c */
extern FILE *fpo;

extern const char *spam_header_place;

extern void passthrough_setup(void);
extern void passthrough_cleanup(void);

extern int  passthrough_keepopen(void);

extern void write_message(rc_t status);
extern void write_log_message(rc_t status);

extern void output_setup(void);
extern void output_cleanup(void);

#endif	/* PASSTHROUGH_H */
