/*
 * $Log: log.h,v $
 * Revision 1.1  2013-05-15 00:14:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef LOG_H
#define LOG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "utils.h"

#define MAXLOGBSIZE 128

void            debug(char *msg, ...);
void            fatal(char *msg);

#endif
