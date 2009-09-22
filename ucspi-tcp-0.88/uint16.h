/*
 * $Log: uint16.h,v $
 * Revision 1.2  2005-05-13 23:54:03+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef UINT16_H
#define UINT16_H

typedef unsigned short uint16;

void            uint16_pack(char *, uint16);
void            uint16_pack_big(char *, uint16);
void            uint16_unpack(char *, uint16 *);
void            uint16_unpack_big(char *, uint16 *);

#endif
