/*
 * $Log: socket.h,v $
 * Revision 1.1  2005-06-15 20:24:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SOCKET_H
#define SOCKET_H

#include "haveip6.h"

int             socket_tcp4(void);
int             socket_tcp6(void);

#endif
