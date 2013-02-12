#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <stdlib.h>
#include <stdarg.h>

#include "utils.h"

#define MAXLOGBSIZE 128

void debug (char *msg, ...);
void fatal ( char *msg );

#endif
