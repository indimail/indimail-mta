/*
 * $Log: buffer_2.c,v $
 * Revision 1.2  2008-07-17 23:02:45+05:30  Cprogrammer
 * use unistd.h instead of readwrite.h
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "buffer.h"

char            buffer_2_space[256];
static buffer   it = BUFFER_INIT(write, 2, buffer_2_space, sizeof buffer_2_space);
buffer         *buffer_2 = &it;
