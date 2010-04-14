/*
 * $Log: makeseekable.c,v $
 * Revision 2.5  2009-03-31 10:23:06+05:30  Cprogrammer
 * added orignal copyright (missed out by oversight)
 *
 * Revision 2.4  2003-10-23 13:19:24+05:30  Cprogrammer
 * added check for failure of tmpfile()
 *
 * Revision 2.3  2002-11-22 01:50:58+05:30  Cprogrammer
 * rewind stream as we are at EOF
 *
 * Revision 2.2  2002-11-21 14:35:30+05:30  Cprogrammer
 * included string.h to suppress compilation warnings
 *
 * Revision 2.1  2002-07-12 01:01:03+05:30  Cprogrammer
 * function to make stdin seekable
 *
 */

/*
 * Copyright (c) 1987 University of Maryland Computer Science Department.
 * All rights reserved.
 * Permission to copy for any purpose is hereby granted so long as this
 * copyright notice remains intact.
 *
 * MakeSeekable forces an input stdio file to be seekable, by copying to
 * a temporary file if necessary.
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: makeseekable.c,v 2.5 2009-03-31 10:23:06+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef MAKE_SEEKABLE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int
makeseekable(inpF)
	register FILE  *inpF;
{
	register int    outF, n;
	FILE           *tmpf;
	int             blksize;
#ifdef MAXBSIZE
	char            buf[MAXBSIZE];
	struct stat     st;
#else
	char            buf[BUFSIZ];
#endif

	if (lseek(fileno(inpF), 0L, SEEK_CUR) >= 0 && !isatty(fileno(inpF)))
		return (0);
	if(!(tmpf = tmpfile())) /*- tmpfile() is not safe on all systems */
	{
		fprintf(stderr, "tmpfile: %s\n", strerror(errno));
		return(-1);
	}
	outF = fileno(tmpf);
#ifdef MAXBSIZE
	if (fstat(outF, &st))			/*- how can this ever fail? */
		blksize = MAXBSIZE;
	else
		blksize = (MAXBSIZE < st.st_blksize ? MAXBSIZE : st.st_blksize);
#else
	blksize = BUFSIZ;
#endif
	/*- copy from input file to temp file */
	while ((n = fread(buf, 1, blksize, inpF)) > 0)
	{
		if (sockwrite(outF, buf, n) != n)
		{
			(void) close(outF);
			fprintf(stderr, "sockwrite: %s\n", strerror(errno));
			return (-1);
		}
	}
	/*
	 * ferror() is broken in Ultrix 1.2; hence the && 
	 */
	if (ferror(inpF) && !feof(inpF))
	{
		fprintf(stderr, "EOF: %s\n", strerror(errno));
		(void) close(outF);
		return (-1);
	}
	/*
	 * Now switch inpF to point at the temp file.  Since we hit EOF, there
	 * is nothing in inpF's stdio buffers, so we can play a dirty trick: 
	 */
	clearerr(inpF);
	if (dup2(outF, fileno(inpF)))
	{
		fprintf(stderr, "dup2: %s\n", strerror(errno));
		(void) close(outF);
		return (-1);
	}
	(void) close(outF);
	rewind(inpF);
	return (0);
}
#endif

void
getversion_makeseekable_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
