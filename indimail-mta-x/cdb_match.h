/*
 * $Log: cdb_match.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2022-10-30 20:17:42+05:30  Cprogrammer
 * Initial revision
 *
 *
 * $Id: cdb_match.h,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#ifndef _CDB_MATCHADDR_H
#define _CDB_MATCHADDR_H

#define CDB_MEM_ERR   -1
#define CDB_LSEEK_ERR -2
#define CDB_READ_ERR  -3
#define CDB_FILE_ERR  -4
#define CDB_NOMATCH    0
#define CDB_FOUND      1

int             cdb_match(const char *, const char *, int, char **);
int             cdb_matchaddr(const char *, const char *, int);

#endif
