/*-
 * $NetBSD: strfile.c,v 1.3 1995/03/23 08:28:47 cgd Exp $ 
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ken Arnold.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Changes, September 1995, to make the damn thing actually sort instead
 * of just pretending.  Amy A. Lewis
 * 
 * And lots more.
 * 
 * Fixed the special cases of %^J% (an empty fortune), no 'separator' at
 * the end of the file, and a trailing newline at the end of the file, all
 * of which produced total ballsup at one point or another.
 * 
 * This included adding a routine to go back and write over the last pointer
 * written or stored, for the case of an empty fortune.
 * 
 * unstr also had to be modified (well, for *lots* of reasons, but this was
 * one) to be certain to put the delimiters in the right places.
 */

#if 0
#ifndef lint
 static char copyright[] = "@(#) Copyright (c) 1989, 1993\n\
 The Regents of the University of California.  All rights reserved.\n";
#endif /*- not lint */
#ifndef lint
#if 0
static char sccsid[] = "@(#)strfile.c  8.1 (Berkeley) 5/31/93";
#else
static char rcsid[] = "$NetBSD: strfile.c,v 1.3 1995/03/23 08:28:47 cgd Exp $";
#endif
#endif /* not lint */
#endif /* if 0 */
/*
 * I haven't the faintest flipping idea what all that is, so kill the warnings 
 */

#include	<stdlib.h>
#include	<unistd.h>
#include	<netinet/in.h>
#include	<sys/param.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	"strfile.h"

#ifndef MAXPATHLEN
#define	MAXPATHLEN	1024
#endif /* MAXPATHLEN */

/*-
 * This program takes a file composed of strings seperated by
 * lines containing only the delimiting character (the default
 * character is '%') and creates another file which consists of a table
 * describing the file (structure from "strfile.h"), a table of seek
 * pointers to the start of the strings, and the strings, each terminated
 * by a null byte.  Usage:
 *
 *      % strfile [-iorsx] [ -cC ] sourcefile [ datafile ]
 *
 *      c - Change delimiting character from '%' to 'C'
 *      s - Silent.  Give no summary of data processed at the end of
 *          the run.
 *      o - order the strings in alphabetic order
 *      i - if ordering, ignore case 
 *      r - randomize the order of the strings
 *      x - set rotated bit
 *
 *              Ken Arnold      Sept. 7, 1978 --
 *
 *      Added ordering options.
 * 
 * Made ordering options do more than set the bloody flag, September 95 A. Lewis
 * 
 * Always make sure that your loop control variables aren't set to bloody
 * *zero* before distributing the bloody code, all right?
 * 
 */

#define	TRUE	1
#define	FALSE	0

#define	STORING_PTRS	(Oflag || Rflag)
#define	CHUNKSIZE	512

#define	ALWAYS	1
#define	ALLOC(ptr,sz)	if (ALWAYS) { \
			if (ptr == NULL) \
				ptr = malloc((unsigned int) (CHUNKSIZE * sizeof *ptr)); \
			else if (((sz) + 1) % CHUNKSIZE == 0) \
				ptr = realloc((void *) ptr, ((unsigned int) ((sz) + CHUNKSIZE) * sizeof *ptr)); \
			if (ptr == NULL) { \
				fprintf(stderr, "out of space\n"); \
				exit(1); \
			} \
		}

typedef struct
{
    char first;
    off_t pos;
}
STR;

char *Infile = NULL,		/* input file name */
  Outfile[MAXPATHLEN] = "",	/* output file name */
  Delimch = '%';		/* delimiting character */
int Sflag = FALSE;		/* silent run flag */
int Oflag = FALSE;		/* ordering flag */
int Iflag = FALSE;		/* ignore case flag */
int Rflag = FALSE;		/* randomize order flag */
int Xflag = FALSE;		/* set rotated bit */
long Num_pts = 0;		/* number of pointers/strings */
off_t *Seekpts;
FILE *Sort_1, *Sort_2;		/* pointers for sorting */
STRFILE Tbl;			/* statistics table */
STR *Firstch;			/* first chars of each string */

void usage(void)
{
    fprintf(stderr,
	    "strfile [-iorsx] [-c char] sourcefile [datafile]\n");
    exit(1);
}

/*
 *    This routine evaluates arguments from the command line
 */
void getargs(int argc, char **argv)
{
    extern char *optarg;
    extern int optind;
    int ch;

    while ((ch = getopt(argc, argv, "c:iorsx")) != EOF)
	switch (ch)
	  {
	  case 'c':		/* new delimiting char */
	      Delimch = *optarg;
	      if (!isascii(Delimch))
	      {
		  printf("bad delimiting character: '\\%o\n'",
			 Delimch);
	      }
	      break;
	  case 'i':		/* ignore case in ordering */
	      Iflag++;
	      break;
	  case 'o':		/* order strings */
	      Oflag++;
	      break;
	  case 'r':		/* randomize pointers */
	      Rflag++;
	      break;
	  case 's':		/* silent */
	      Sflag++;
	      break;
	  case 'x':		/* set the rotated bit */
	      Xflag++;
	      break;
	  case '?':
	  default:
	      usage();
	  }
    argv += optind;

    if (*argv)
    {
	Infile = *argv;
	if (*++argv)
	    (void) strcpy(Outfile, *argv);
    }
    if (!Infile)
    {
	puts("No input file name");
	usage();
    }
    if (*Outfile == '\0')
    {
	strcpy(Outfile, Infile);
	strcat(Outfile, ".dat");
    }
}

/*
 * add_offset:
 *      Add an offset to the list, or write it out, as appropriate.
 */
void add_offset(FILE * fp, off_t off)
{
    off_t net;

    if (!STORING_PTRS)
    {
	net = htonl(off);
	fwrite(&net, 1, sizeof net, fp);
    }
    else
    {
	ALLOC(Seekpts, Num_pts + 1);
	Seekpts[Num_pts] = off;
    }
    Num_pts++;
}

/*
 * fix_last_offset:
 *     Used when we have two separators in a row.
 */
void fix_last_offset(FILE * fp, off_t off)
{
    off_t net;

    if (!STORING_PTRS)
    {
	net = htonl(off);
	fseek(fp, -(sizeof net), SEEK_CUR);
	fwrite(&net, 1, sizeof net, fp);
    }
    else
	Seekpts[Num_pts - 1] = off;
}

/*
 * cmp_str:
 *      Compare two strings in the file
 */
int cmp_str(const void *v1, const void *v2)
{
    register int c1, c2;
    register int n1, n2;
    register STR *p1, *p2;

#define	SET_N(nf,ch)	(nf = (ch == '\n'))
#define	IS_END(ch,nf)	(ch == Delimch && nf)

    p1 = (STR *) v1;
    p2 = (STR *) v2;
    c1 = p1->first;
    c2 = p2->first;
    if (c1 != c2)
	return c1 - c2;

    fseek(Sort_1, p1->pos, 0);
    fseek(Sort_2, p2->pos, 0);

    n1 = FALSE;
    n2 = FALSE;
    while (!isalnum(c1 = getc(Sort_1)) && c1 != '\0')
	SET_N(n1, c1);
    while (!isalnum(c2 = getc(Sort_2)) && c2 != '\0')
	SET_N(n2, c2);

    while (!IS_END(c1, n1) && !IS_END(c2, n2))
    {
	if (Iflag)
	{
	    if (isupper(c1))
		c1 = tolower(c1);
	    if (isupper(c2))
		c2 = tolower(c2);
	}
	if (c1 != c2)
	    return c1 - c2;
	SET_N(n1, c1);
	SET_N(n2, c2);
	c1 = getc(Sort_1);
	c2 = getc(Sort_2);
    }
    if (IS_END(c1, n1))
	c1 = 0;
    if (IS_END(c2, n2))
	c2 = 0;
    return c1 - c2;
}

/*
 * do_order:
 *      Order the strings alphabetically (possibly ignoring case).
 */
void
  do_order(void)
{
    register long i;
    register off_t *lp;
    register STR *fp;

    Sort_1 = fopen(Infile, "r");
    Sort_2 = fopen(Infile, "r");
    qsort((char *) Firstch, (int) Num_pts - 1, sizeof *Firstch, cmp_str);
/*      i = Tbl.str_numstr;
 * Fucking brilliant.  Tbl.str_numstr was initialized to zero, and is still zero
 */
    i = Num_pts - 1;
    lp = Seekpts;
    fp = Firstch;
    while (i--)
	*lp++ = fp++->pos;
    fclose(Sort_1);
    fclose(Sort_2);
    Tbl.str_flags |= STR_ORDERED;
}

char *
  unctrl(char c)
{
    static char buf[3];

    if (isprint(c))
    {
	buf[0] = c;
	buf[1] = '\0';
    }
    else if (c == 0177)
    {
	buf[0] = '^';
	buf[1] = '?';
    }
    else
    {
	buf[0] = '^';
	buf[1] = c + 'A' - 1;
    }
    return buf;
}

/*
 * randomize:
 *      Randomize the order of the string table.  We must be careful
 *      not to randomize across delimiter boundaries.  All
 *      randomization is done within each block.
 */
void randomize(void)
{
    register int cnt, i;
    register off_t tmp;
    register off_t *sp;
    extern time_t time(time_t *);

    srandom((int) (time((time_t *) NULL) + getpid()));

    Tbl.str_flags |= STR_RANDOM;
/*      cnt = Tbl.str_numstr;
 * See comment above.  Isn't this stuff distributed worldwide?  How embarrassing!
 */
    cnt = Num_pts;

    /*
     * move things around randomly
     */

    for (sp = Seekpts; cnt > 0; cnt--, sp++)
    {
	i = random() % cnt;
	tmp = sp[0];
	sp[0] = sp[i];
	sp[i] = tmp;
    }
}

/*
 * main:
 *      Drive the sucker.  There are two main modes -- either we store
 *      the seek pointers, if the table is to be sorted or randomized,
 *      or we write the pointer directly to the file, if we are to stay
 *      in file order.  If the former, we allocate and re-allocate in
 *      CHUNKSIZE blocks; if the latter, we just write each pointer,
 *      and then seek back to the beginning to write in the table.
 */
int main(int ac, char **av)
{
    register char *sp;
    register FILE *inf, *outf;
    register off_t last_off, length, pos, *p;
    register int first, cnt;
    register char *nsp;
    register STR *fp;
    static char string[257];

    getargs(ac, av);		/* evalute arguments */
    if ((inf = fopen(Infile, "r")) == NULL)
    {
	perror(Infile);
	exit(1);
    }

    if ((outf = fopen(Outfile, "w")) == NULL)
    {
	perror(Outfile);
	exit(1);
    }
    if (!STORING_PTRS)
	(void) fseek(outf, sizeof Tbl, 0);

    /*
     * Write the strings onto the file
     */

    Tbl.str_longlen = 0;
    Tbl.str_shortlen = (unsigned int) 0xffffffff;
    Tbl.str_delim = Delimch;
    Tbl.str_version = STRVERSION;
    first = Oflag;
    add_offset(outf, ftell(inf));
    last_off = 0;
    do
    {
	sp = (char *) fgets(string, 256, inf);
	if (sp == NULL || STR_ENDSTRING(sp, Tbl))
	{
	    pos = ftell(inf);
	    length = pos - last_off - (sp ? strlen((char *) sp) : 0);
	    if (!length)
		/* Here's where we go back and fix things, if the
		 * 'fortune' just read was the null string.
		 * We had to make the assignment of last_off slightly
		 * redundant to achieve this.
		 */
	    {
		if (pos - last_off == 2)
		    fix_last_offset(outf, pos);
		last_off = pos;
		continue;
	    }
	    last_off = pos;
	    add_offset(outf, pos);
	    if (Tbl.str_longlen < length)
		Tbl.str_longlen = length;
	    if (Tbl.str_shortlen > length)
		Tbl.str_shortlen = length;
	    first = Oflag;
	}
	else if (first)
	{
	    for (nsp = sp; !isalnum(*nsp); nsp++)
		continue;
	    ALLOC(Firstch, Num_pts);
	    fp = &Firstch[Num_pts - 1];
	    if (Iflag && isupper(*nsp))
		fp->first = tolower(*nsp);
	    else
		fp->first = *nsp;
	    fp->pos = Seekpts[Num_pts - 1];
	    first = FALSE;
	}
    }
    while (sp != NULL);

    /*
     * write the tables in
     */

    fclose(inf);

    if (Oflag)
	do_order();
    else if (Rflag)
	randomize();

    if (Xflag)
	Tbl.str_flags |= STR_ROTATED;

    if (!Sflag)
    {
	printf("\"%s\" created\n", Outfile);
	if (Num_pts == 2)
	    puts("There was 1 string");
	else
	    printf("There were %ld strings\n", Num_pts - 1);
	printf("Longest string: %lu byte%s\n", Tbl.str_longlen,
	       Tbl.str_longlen == 1 ? "" : "s");
	printf("Shortest string: %lu byte%s\n", Tbl.str_shortlen,
	       Tbl.str_shortlen == 1 ? "" : "s");
    }

    fseek(outf, (off_t) 0, 0);
    Tbl.str_version = htonl(Tbl.str_version);
    Tbl.str_numstr = htonl(Num_pts - 1);
    /* Look, Ma!  After using the variable three times, let's store
     * something in it!
     */
    Tbl.str_longlen = htonl(Tbl.str_longlen);
    Tbl.str_shortlen = htonl(Tbl.str_shortlen);
    Tbl.str_flags = htonl(Tbl.str_flags);
    fwrite((char *) &Tbl, sizeof Tbl, 1, outf);
    if (STORING_PTRS)
    {
	for (p = Seekpts, cnt = Num_pts; cnt--; ++p)
	    *p = htonl(*p);
	fwrite((char *) Seekpts, sizeof *Seekpts, (int) Num_pts, outf);
    }
    fclose(outf);
    exit(0);
}
