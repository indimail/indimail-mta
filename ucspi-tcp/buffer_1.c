/*
 * $Log: buffer_1.c,v $
 * Revision 1.2  2008-07-17 23:02:34+05:30  Cprogrammer
 * use unistd.h instead of readwrite.h
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "buffer.h"

char            buffer_1_space[BUFFER_OUTSIZE];
static buffer   it = BUFFER_INIT(write, 1, buffer_1_space, sizeof buffer_1_space);
buffer         *buffer_1 = &it;
