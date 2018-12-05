#ifndef	random128_h
#define	random128_h

/*
** Copyright 1998 - 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifdef	__cplusplus
extern "C" {
#endif

/*
	random128 returns 128 random bits from the entropy.  random128
	returns a pointer to 32 hexadecimal uppercase nibbles, all total
	being 128 bits.
*/

const char *random128();

/*
	random128_alpha does the same thing, except that the return
	string contains uppercase alphabetic letters only (letters 'A'
	through 'P').
*/

const char *random128_alpha();

/*
** random128_bin(), saves the 128 random bits in 16 bytes.
*/

typedef unsigned char random128binbuf[16];

void random128_binary(random128binbuf *);

#ifdef	__cplusplus
}
#endif

#endif
