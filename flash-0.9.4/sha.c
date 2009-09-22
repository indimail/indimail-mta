/*
 * $Log: sha.c,v $
 * Revision 1.2  2008-07-17 21:39:09+05:30  Cprogrammer
 * fixed compile warnings on mac
 *
 * Revision 1.1  2002-12-16 01:55:20+05:30  Manny
 * Initial revision
 *
 *
 * Implementation of NIST's Secure Hash Algorithm (FIPS 180)
 * Lightly bummed for execution efficiency.
 *
 * Jim Gillogly 3 May 1993
 *
 * 27 Aug 93: imported LITTLE_ENDIAN mods from Peter Gutmann's implementation
 * 5 Jul 94: Modified for NSA fix
 *
 * Compile: cc -O -o sha sha.c
 *
 * To remove the test wrapper and use just the nist_hash() routine,
 * compile with -DONT_WRAP
 *
 * To reverse byte order for little-endian machines, use -DLITTLE_ENDIAN
 *
 * To get the original SHA definition before the 1994 fix, use -DVERSION_0
 *
 * Usage: sha [-vt] [filename ...]
 *
 *       -v switch: output the filename as well
 *       -t switch: suppress spaces between 32-bit blocks
 *
 *       If no input files are specified, process standard input.
 *
 * Output: 40-hex-digit digest of each file specified (160 bits)
 *
 * Synopsis of the function calls:
 *
 *    sha_file(char *filename, unsigned long *buffer)
 *       Filename is a file to be opened and processed.
 *       buffer is a user-supplied array of 5 or more longs.
 *       The 5-word buffer is filled with 160 bits of non-terminated hash.
 *       Returns 0 if successful, non-zero if bad file.
 * 
 *    void sha_stream(FILE *stream, unsigned long *buffer)
 *       Input is from already-opened stream, not file.
 *
 *    void sha_memory(char *mem, long length, unsigned long *buffer)
 *       Input is a memory block "length" bytes long.
 *
 * Caveat:
 *       Not tested for case that requires the high word of the length,
 *       which would be files larger than 1/2 gig or so.
 *
 * Limitation:
 *       sha_memory (the memory block function) will deal with blocks no longer
 *       than 4 gigabytes; for longer samples, the stream version will
 *       probably be most convenient (e.g. perl moby_data.pl | sha).
 * 
 * Bugs:
 *       The standard is defined for bit strings; I assume bytes.
 * 
 * Copyright 1993, Dr. James J. Gillogly
 * This code may be freely used in any application.
 */

/*
 * #define LITTLE_ENDIAN
 */

/*
 * #define VERSION_0 
 * Define this to get the original SHA definition 
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define VERBOSE

#define TRUE  1
#define FALSE 0

#define SUCCESS 0
#define FAILURE -1

int             sha_file();		/*- External entries */
void            sha_stream(), sha_memory();
char           *shacrypt(char *key, char *salt);

static void     nist_guts();

char            crypted_buffer[31];

#define ascii_to_bin(c) ((c)>='a'?(c-59):(c)>='A'?((c)-53):(c)-'.')
#define bin_to_ascii(c) ((c)>=38?((c)-38+'a'):(c)>=12?((c)-12+'A'):(c)+'.')

#define HASH_SIZE 5				/*- Produces 160-bit digest of the message */


#ifdef O_WRAP					/*- Using just the hash routine itself */

main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *prog = *argv;

	argc--, argv++;

	if (argc > 1)
	{
		if (strlen(*(argv + 1)) != 3)
			printf("Salt must be 3 characters\n");
		else
			printf("%s\n", shacrypt(*argv, *(argv + 1)));
	} else
		printf("Usage: %s key salt\n", prog);
}

#endif	/* O_WRAP */

#ifdef LITTLE_ENDIAN			/*- Imported from Peter Gutmann's implementation */

/*
 * When run on a little-endian CPU we need to perform byte reversal on an
 * array of longwords.  It is possible to make the code endianness-
 * independant by fiddling around with data at the byte level, but this
 * makes for very slow code, so we rely on the user to sort out endianness
 * at compile time 
 */

static void
byteReverse(unsigned long *buffer, int byteCount)
{
	unsigned long   value;
	int             count;

	byteCount /= sizeof(unsigned long);
	for (count = 0; count < byteCount; count++)
	{
		value = (buffer[count] << 16) | (buffer[count] >> 16);
		buffer[count] = ((value & 0xFF00FF00L) >> 8) | ((value & 0x00FF00FFL) << 8);
	}
}
#endif /*- LITTLE_ENDIAN */


char           *
shacrypt(char *key, char *salt)
{
	unsigned long   hbuf[HASH_SIZE];
	unsigned long   hhbuf[HASH_SIZE];
	char           *KEY, *f, *t;
	long            l;
	int             i;

	l = strlen(key) + 3;

	KEY = malloc(strlen(key) + 4);
	if (KEY == NULL)
	{
		perror("SHA");
		exit(EXIT_FAILURE);
	}

	memcpy(KEY, salt, 3);
	strcpy(KEY + 3, key);

	sha_memory(KEY, l, hbuf);
#ifdef LITTLE_ENDIAN
	byteReverse(hbuf, 4 * HASH_SIZE);
#endif /* LITTLE_ENDIAN */

	free(KEY);

	for (i = 0; i < 16; i++)
	{
		memcpy(hhbuf, hbuf, sizeof(hbuf));
		sha_memory((char *) hhbuf, sizeof(hbuf), hbuf);
#ifdef LITTLE_ENDIAN
		byteReverse(hbuf, 4 * HASH_SIZE);
#endif /*
	    * LITTLE_ENDIAN 
	    */
	}

	memcpy(crypted_buffer, salt, 3);
	t = crypted_buffer + 3;
	f = (char *) hbuf;

	for (i = 0; i < sizeof(hbuf); i++, t++, f++)
		*t = bin_to_ascii((*f) & 0x3f);
	*t = '\0';

	return crypted_buffer;
}

union longbyte
{
	unsigned long   W[80];		/*- Process 16 32-bit words at a time */
	char            B[320];		/*- But read them as bytes for counting */
};

int
sha_file(filename, buffer)		/*- Hash a file */
	char           *filename;
	unsigned long  *buffer;
{
	FILE           *infile;

	if ((infile = fopen(filename, "rb")) == NULL)
	{
		int             i;

		for (i = 0; i < 5; i++)
			buffer[i] = 0xdeadbeef;
		return FAILURE;
	}
	(void) sha_stream(infile, buffer);
	fclose(infile);
	return SUCCESS;
}

void
sha_memory(mem, length, buffer)	/*- Hash a memory block */
	char           *mem;
	unsigned long   length;
	unsigned long  *buffer;
{
	nist_guts(FALSE, (FILE *) NULL, mem, length, buffer);
}

void
sha_stream(stream, buffer)
	FILE           *stream;
	unsigned long  *buffer;
{
	nist_guts(TRUE, stream, (char *) NULL, 0l, buffer);
}

#define f0(x,y,z) (z ^ (x & (y ^ z)))	/*- Magic functions */
#define f1(x,y,z) (x ^ y ^ z)
#define f2(x,y,z) ((x & y) | (z & (x | y)))
#define f3(x,y,z) (x ^ y ^ z)

#define K0 0x5a827999			/*- Magic constants */
#define K1 0x6ed9eba1
#define K2 0x8f1bbcdc
#define K3 0xca62c1d6

#define S(n, X) ((X << n) | (X >> (32 - n)))	/*- Barrel roll */

#define r0(f, K) \
    temp = S(5, A) + f(B, C, D) + E + *p0++ + K; \
    E = D;  \
    D = C;  \
    C = S(30, B); \
    B = A;  \
    A = temp

#ifdef VERSION_0
#define r1(f, K) \
    temp = S(5, A) + f(B, C, D) + E + \
	   (*p0++ = *p1++ ^ *p2++ ^ *p3++ ^ *p4++) + K; \
    E = D;  \
    D = C;  \
    C = S(30, B); \
    B = A;  \
    A = temp
#else /*
	   * Version 1: Summer '94 update 
	   */
#define r1(f, K) \
    temp = *p1++ ^ *p2++ ^ *p3++ ^ *p4++; \
    temp = S(5, A) + f(B, C, D) + E + (*p0++ = S(1,temp)) + K; \
    E = D;  \
    D = C;  \
    C = S(30, B); \
    B = A;  \
    A = temp
#endif

static void
nist_guts(file_flag, stream, mem, length, buf)
	int             file_flag;	/*- Input from memory, or from stream?  */
	FILE           *stream;
	char           *mem;
	unsigned long   length;
	unsigned long  *buf;
{
	int             i, nread, nbits;
	union longbyte  d;
	unsigned long   hi_length, lo_length;
	int             padded;
	char           *s;

	register unsigned long *p0, *p1, *p2, *p3, *p4;
	unsigned long   A, B, C, D, E, temp;

	unsigned long   h0, h1, h2, h3, h4;

	h0 = 0x67452301;			/*- Accumulators */
	h1 = 0xefcdab89;
	h2 = 0x98badcfe;
	h3 = 0x10325476;
	h4 = 0xc3d2e1f0;

	padded = FALSE;
	s = mem;
	for (hi_length = lo_length = 0;;)	/*- Process 16 longs at a time */
	{
		if (file_flag)
		{
			nread = fread(d.B, 1, 64, stream);	/*- Read as 64 bytes */
		} else
		{
			if (length < 64)
				nread = length;
			else
				nread = 64;
			length -= nread;
			memcpy(d.B, s, nread);
			s += nread;
		}
		if (nread < 64)			/*- Partial block?  */
		{
			nbits = nread << 3;	/*- Length: bits */
			if ((lo_length += nbits) < nbits)
				hi_length++;	/*
								 * 64-bit integer 
								 */

			if (nread < 64 && !padded)	/*- Append a single bit */
			{
				d.B[nread++] = 0x80;	/*- Using up next byte */
				padded = TRUE;	/*- Single bit once */
			}
			for (i = nread; i < 64; i++)	/*- Pad with nulls */
				d.B[i] = 0;
			if (nread <= 56)	/*- Room for length in this block */
			{
				d.W[14] = hi_length;
				d.W[15] = lo_length;
#ifdef LITTLE_ENDIAN
				byteReverse(d.W, 56);
#endif /* LITTLE_ENDIAN */
			}
#ifdef LITTLE_ENDIAN
			else
				byteReverse(d.W, 64);
#endif /*- LITTLE_ENDIAN */
		} else					/*- Full block -- get efficient */
		{
			if ((lo_length += 512) < 512)
				hi_length++;	/*- 64-bit integer */
#ifdef LITTLE_ENDIAN
			byteReverse(d.W, 64);
#endif /*- LITTLE_ENDIAN */
		}

		p0 = d.W;
		A = h0;
		B = h1;
		C = h2;
		D = h3;
		E = h4;

		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);
		r0(f0, K0);

		p1 = &d.W[13];
		p2 = &d.W[8];
		p3 = &d.W[2];
		p4 = &d.W[0];

		r1(f0, K0);
		r1(f0, K0);
		r1(f0, K0);
		r1(f0, K0);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f1, K1);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f2, K2);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);
		r1(f3, K3);

		h0 += A;
		h1 += B;
		h2 += C;
		h3 += D;
		h4 += E;

		if (nread <= 56)
			break;				/*- If it's greater, length in next block */
	}
	buf[0] = h0;
	buf[1] = h1;
	buf[2] = h2;
	buf[3] = h3;
	buf[4] = h4;
}
