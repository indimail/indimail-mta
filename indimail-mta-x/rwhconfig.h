/*
 * $Log: rwhconfig.h,v $
 * Revision 1.3  2020-04-30 18:08:52+05:30  Cprogrammer
 * removed rwhconfig_err variable.
 *
 * Revision 1.2  2004-10-11 14:01:35+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-16 01:20:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef RWHCONFIG_H
#define RWHCONFIG_H

#include "strerr.h"
#include "sconfig.h"

int             rwhconfig(config_str *, stralloc *);

#endif
