/*
 * $Log: ismaildup.h,v $
 * Revision 2.1  2011-06-30 20:37:48+05:30  Cprogrammer
 * prototype for ismaildup()
 *
 */
#ifndef ISMAILDUP_H
#define ISMAILDUP_H
#include "indimail.h"

#ifndef	lint
static char     sccsidisduph[] = "$Id: ismaildup.h,v 2.1 2011-06-30 20:37:48+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_SSL
int             ismaildup(char *);
#endif

#endif
