/*
 * $Log: hexconversion.h,v $
 * Revision 1.1  2013-08-06 00:46:27+05:30  Cprogrammer
 * Initial revision
 *
 * File:   hexconversion.h
 * Author: tux
 *
 * Created on June 29, 2012, 4:24 AM
 */

#ifndef HEXCONVERSION_H
#define	HEXCONVERSION_H

void            bytetohex(unsigned char decimal, char hex[3]);
char            tohex(char num);
int             fromhex(unsigned char c);

#endif	/* HEXCONVERSION_H */
