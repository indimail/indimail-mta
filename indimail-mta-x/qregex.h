/*
 * $Log: qregex.h,v $
 * Revision 1.11  2023-09-05 20:59:42+05:30  Cprogrammer
 * moved matchregex error codes to libqmail/base/matchregex.h
 *
 * Revision 1.10  2010-04-18 17:00:54+05:30  Cprogrammer
 * moved common sql functions to sqlmatch.h
 *
 * Revision 1.9  2009-05-01 10:42:21+05:30  Cprogrammer
 * added error definitions
 * new prototypes for address_match(), cdbmatch(), sqlmatch() for errstr argument
 *
 * Revision 1.8  2009-04-30 21:13:46+05:30  Cprogrammer
 * added sqlmatch_close_db()
 *
 * Revision 1.7  2009-04-30 15:41:22+05:30  Cprogrammer
 * added sqlmatch() function
 *
 * Revision 1.6  2009-04-30 08:31:53+05:30  Cprogrammer
 * added cdbmatch()
 *
 * Revision 1.5  2009-04-29 09:02:08+05:30  Cprogrammer
 * added cdb filename argument to address_match()
 *
 * Revision 1.4  2009-04-29 08:24:26+05:30  Cprogrammer
 * change for address_match() function
 *
 * Revision 1.3  2004-09-21 23:49:02+05:30  Cprogrammer
 * added matchregex() and setdotChar()
 *
 * Revision 1.2  2003-12-22 18:35:26+05:30  Cprogrammer
 * added address_match() function
 *
 * Revision 1.1  2003-12-20 13:17:45+05:30  Cprogrammer
 * Initial revision
 *
 */
/*
 * simple header file for the matchregex prototype
 */
#ifndef _QREGEX_H_
#define _QREGEX_H_
#include "constmap.h"
#include "stralloc.h"
#include "matchregex.h"
#ifdef USE_SQL
#include "sqlmatch.h"
#endif

#ifndef AM_MYSQL_ERR
#define  AM_MYSQL_ERR  (MAX_AM_ERR -1)
#endif

int             address_match(char *, stralloc *, stralloc *, struct constmap *, stralloc *, char **);
void            setdotChar(char);
int             cdbmatch(char *, char *, int, struct constmap *, char **);
#endif
