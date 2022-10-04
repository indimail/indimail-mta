/*
 * $Log: qmail.h,v $
 * Revision 1.7  2022-10-04 23:43:51+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.6  2020-05-12 12:14:13+05:30  Cprogrammer
 * fix integer signedness error in qmail_put() (CVE-2005-1515)
 *
 * Revision 1.5  2009-04-21 14:27:24+05:30  Cprogrammer
 * define CUSTOM_ERR_FD
 *
 * Revision 1.4  2005-05-31 15:45:23+05:30  Cprogrammer
 * added fdc for passing custom error message to qmail-queue
 *
 * Revision 1.3  2004-10-11 13:57:51+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:26+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef QMAIL_H
#define QMAIL_H

#include "substdio.h"

#ifndef CUSTOM_ERR_FD
#define CUSTOM_ERR_FD 2
#endif

struct qmail
{
	int             flagerr;
	unsigned long   pid;
	int             fdm; /*- fd message */
	int             fde; /*- fd envelope */
	int             fdc; /*- fd custom */
	substdio        ss;
	char            buf[1024];
};

int             qmail_open(struct qmail *);
void            qmail_put(struct qmail *, char *, unsigned int);
void            qmail_puts(struct qmail *, char *);
void            qmail_from(struct qmail *, char *);
void            qmail_to(struct qmail *, char *);
void            qmail_fail(struct qmail *);
char           *qmail_close(struct qmail *);
unsigned long   qmail_qp(struct qmail *);

#endif
