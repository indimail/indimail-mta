/*
 * $Log: crc.c,v $
 * Revision 2.6  2008-07-17 23:01:11+05:30  Cprogrammer
 * conditional compilation of ERESTART
 *
 * Revision 2.5  2005-12-21 09:45:54+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.4  2003-01-03 14:14:36+05:30  Cprogrammer
 * add getversion_crc_c()
 *
 * Revision 2.3  2002-12-27 16:39:59+05:30  Cprogrammer
 * removed compilation warnings
 *
 * Revision 2.2  2002-12-11 10:27:54+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.1  2002-09-27 13:17:52+05:30  Cprogrammer
 * Crc Application
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#ifndef WindowsNT
#include <pwd.h>
#include <grp.h>
#endif
#define MAXBUF 4096
#ifndef S_IRGRP
#define S_IRGRP	(S_IREAD >> 3)
#define S_IWGRP (S_IWRITE >> 3)
#define S_IXGRP (S_IEXEC >> 3)
#define S_IROTH (S_IREAD >> 6)
#define S_IWOTH (S_IWRITE >> 6)
#define S_IXOTH (S_IEXEC >> 6)
#endif/*- ifndef S_IRGRP -*/

#ifndef CNULL
#define CNULL 0
#endif

/*
 * Routines for Calculating CRC. Manvendra
 * The CRC polynomial. These 4 values define the crc-polynomial. If you
 * change them, you must change crctab[]'s initial value to what is printed
 * by initcrctab() [see 'compile with -DMAKETAB' above].
 */
/*- Value used by:                        CITT   XMODEM ARC     -*/
#define P        0xA001	/*- the poly    : 0x1021 0x1021 A001    -*/
#define INIT_CRC 0L		/*- init value  : -1     0      0       -*/
#define SWAPPED			/*- bit order   : undef defined defined -*/
#define W        16		/*- bits in CRC : 16     16     16      -*/

/*
 * data type that holds a W-bit unsigned integer 
 */
#if W <= 16
#define WTYPE unsigned short
#else
#define WTYPE   unsigned long
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

/*
 * the number of bits per char: don't change it. 
 */
#define B 8

/*- as calculated by initcrctab() -*/
static WTYPE    crctab[1 << B] =
{
	0x0, 0xc0c1, 0xc181, 0x140, 0xc301, 0x3c0, 0x280, 0xc241,
	0xc601, 0x6c0, 0x780, 0xc741, 0x500, 0xc5c1, 0xc481, 0x440,
	0xcc01, 0xcc0, 0xd80, 0xcd41, 0xf00, 0xcfc1, 0xce81, 0xe40,
	0xa00, 0xcac1, 0xcb81, 0xb40, 0xc901, 0x9c0, 0x880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
};

struct stat     stat_buf;
long            initial_crc = INIT_CRC;
extern char    *optarg;
extern int      optind;
extern int      opterr;

int printcrc    __P((char *, unsigned long *, int));
void stats      __P((char *));
int print_perm  __P((unsigned));
WTYPE updcrc    __P((WTYPE, char *, int));

#ifndef	lint
static char     sccsid[] = "$Id: crc.c,v 2.6 2008-07-17 23:01:11+05:30 Cprogrammer Stab mbhangui $";
#endif

int             main __P((int, char **));

#ifdef MAIN
int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             statflag = 0;
	unsigned long   linecount;
	int             c;
	int             retval;

	if (argc == 1)
		return 0;
	/*
	 * process all arguments 
	 */
	while ((c = getopt(argc, argv, "VvI:i:")) != EOF)
	{
		switch (c)
		{
		case 'V':
		case 'v':
			statflag = 1;
			break;
		case 'I':
		case 'i':
			initial_crc = atoi(optarg);
			break;
		default:
			(void) fprintf(stderr, "crc:  -v (verbose listing)\n");
			(void) fprintf(stderr, "      -i value (initial crc value)\n");
			exit(1);
		}
	}
	for (retval = 0; optind < argc; optind++)
		retval = (printcrc(argv[optind], &linecount, statflag) == -1 ? -1 : retval);
	return retval;
}
#endif

/* Function printcrc
 * name = File for which CRC is to be calculated
 * lcount = int where no of lines in the file is stored
 * statflag =
 * -1 Non Verbose. Function returns the crc value
 *  0 Print only the CRC Value on stdout
 *  1 Print the Long form on the screen as below
 *  9787 -rw-r--r-- manny	None	Dec  5 00:11:54 2000 crc.c
 */

int
printcrc(name, lcount, statflag)
	char           *name;
	unsigned long  *lcount;
	int             statflag;
{
	int             fd;
	int             nr;
	char           *ptr;
	char            buf[MAXBUF + 1];
	WTYPE           crc;
#ifdef MAGICCHECK
	WTYPE           crc2;
#endif
	struct stat     statbuf;

	/*
	 * We silently ignore errors 
	 */
#ifdef WindowsNT
	if (stat(name, &stat_buf))
#else
	if (lstat(name, &stat_buf))
#endif /*- WindowsNT -*/
		return (-1);
	*lcount = 0l;
	if (!S_ISREG(stat_buf.st_mode))
	{
		memset((char *) &statbuf, 0, sizeof(struct stat));
		statbuf.st_mode = stat_buf.st_mode;
		statbuf.st_ino = stat_buf.st_ino;
		statbuf.st_dev = stat_buf.st_dev;
		statbuf.st_rdev = stat_buf.st_rdev;
		statbuf.st_size = stat_buf.st_size;
		/*
		 * statbuf.st_blksize = stat_buf.st_blksize;
		 * statbuf.st_blocks = stat_buf.st_blocks;
		 */
		crc = updcrc(initial_crc, (char *) &statbuf, sizeof(struct stat));
		if (statflag != -1)
		{
			(void) printf("%4.4x", (unsigned) crc);
			if (statflag)
				stats(name);
			else
				(void) printf("\n");
		}
		return (crc);
	}
	/*
	 * open the file and do a silent crc on it 
	 */
#if defined(CYGWIN) || defined(WindowsNT)
	if ((fd = open(name, O_RDONLY | O_BINARY, 0)) < 0)
#else
	if ((fd = open(name, O_RDONLY, 0)) < 0)
#endif
		return (-1);
	for (crc = initial_crc, *lcount = 0l;;)
	{
		if ((nr = read(fd, (char *) buf, MAXBUF)) == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			(void) close(fd);
			return (-1);
		}
		if (!nr)
			break;
		buf[nr] = CNULL;
		for(ptr = buf;*ptr;ptr++)
			if (*ptr ==  '\n')
				(*lcount)++;
		crc = updcrc(crc, buf, nr);
	}
	(void) close(fd);
	if (statflag != -1)
	{
		(void) printf("%4.4x", (unsigned) crc);
		if (statflag)
			stats(name);
		else
			(void) printf("\n");
	}
#ifdef MAGICCHECK
	/*
	 * take one's complement of crc onto data stream, and continue crc
	 * calculation.  Should get a constant (magic number) dependent only
	 * on P, not the data.
	 */
	crc2 = crc ^ -1L;
	for (nr = W - B; nr >= 0; nr -= B)
	{
		buf[0] = (crc2 >> nr);
		crc = updcrc(crc, buf, 1);
	}
	/*
	 * crc should now equal magic 
	 */
	buf[0] = buf[1] = buf[2] = buf[3] = 0;
	(void) printf("magic test: %lx =?= %lx\n", crc, updcrc((WTYPE) - 1, buf, W / B));
#endif
	return (crc);
}

void
stats(name)
	char           *name;
{

	struct passwd  *entry;
	struct group   *group_entry;
	static char     owner[56];
	static char     group[56];
	char            a_time[50];

	static int      prev_uid = -9999;
	static int      prev_gid = -9999;

#ifndef WindowsNT
	owner[55] = group[55] = CNULL;
	if (stat_buf.st_uid != prev_uid)
	{
		entry = getpwuid((int) stat_buf.st_uid);
		if (entry)
			(void) strncpy(owner, entry->pw_name, sizeof(owner) - 1);
		else
			(void) sprintf(owner, "%d", (int) stat_buf.st_uid);
		prev_uid = stat_buf.st_uid;
	}
	if (stat_buf.st_gid != prev_gid)
	{
		group_entry = getgrgid((int) stat_buf.st_gid);
		if (group_entry)
			(void) strncpy(group, group_entry->gr_name, sizeof(owner) - 1);
		else
			(void) sprintf(group, "%d", (int) stat_buf.st_gid);
		prev_gid = stat_buf.st_gid;
	}
#else
	strcpy(owner, "????");
	strcpy(group, "????");
#endif
	(void) strcpy(a_time, ctime(&stat_buf.st_mtime));
	a_time[24] = '\0';
	print_perm(stat_buf.st_mode);
	(void) printf(" %s\t%s\t%s %s\n", owner, group, a_time + 4, name);
	return;
}

int
print_perm(perm)
	unsigned int    perm;
{

	char            string[20];

	(void) strcpy(string, "----------");
	/*-
     *    #define    S_IFMT         0170000  type of file
     *    #define    S_IFIFO        0010000  FIFO special
     *    #define    S_IFCHR        0020000  character special
     *    #define    S_IFDIR        0040000  directory
     *    #define    S_IFBLK        0060000  block special
     *    #define    S_IFREG        0100000  regular file
     *    #define    S_IFLNK        0120000  symbolic link
     *    #define    S_IFSOCK       0140000  socket
     *    #define    S_ISVTX        0001000  save swapped text even after use
     *    #define    S_IREAD        0000400  read permission, owner
     *    #define    S_IWRITE       0000200  write permission, owner
    -*/
	switch (perm & S_IFMT)
	{
	case S_IFDIR:
		string[0] = 'd';
		break;
	case S_IFBLK:
		string[0] = 'b';
		break;
	case S_IFCHR:
		string[0] = 'c';
		break;
	case S_IFIFO:
		string[0] = 'p';
		break;
#ifndef WindowsNT /*- Bill Gates Didn't think of these -*/
	case S_IFLNK:
		string[0] = 'l';
		break;
	case S_IFSOCK:
		string[0] = 's';
		break;
#endif
	}
	if (perm & S_IREAD)
		string[1] = 'r';
	if (perm & S_IWRITE)
		string[2] = 'w';
#ifndef WindowsNT
	if (perm & S_ISUID && perm & S_IEXEC)
		string[3] = 's';
	else
#endif
	if (perm & S_IEXEC)
		string[3] = 'x';
#ifndef WindowsNT
	else
	if (perm & S_ISUID)
		string[3] = 'S';
#endif
	if (perm & S_IRGRP)
		string[4] = 'r';
	if (perm & S_IWGRP)
		string[5] = 'w';
#ifndef WindowsNT
	if (perm & S_ISUID && perm & S_IXGRP)
		string[6] = 's';
	else
#endif
	if (perm & S_IXGRP)
		string[6] = 'x';
#ifndef WindowsNT
	else
	if (perm & S_ISUID)
		string[6] = 'l';
#endif
	if (perm & S_IROTH)
		string[7] = 'r';
	if (perm & S_IWOTH)
		string[8] = 'w';
#ifndef WindowsNT
	if (perm & S_ISVTX && perm & S_IXOTH)
		string[9] = 't';
	else
#endif
	if (perm & S_IXOTH)
		string[9] = 'x';
#ifndef WindowsNT
	else
	if (perm & S_ISVTX)
		string[9] = 'T';
#endif
	(void) printf(" %s", string);
	return(0);
}

WTYPE
updcrc(icrc, icp, icnt)
	WTYPE           icrc;
	char           *icp;
	int             icnt;
{
	register WTYPE  crc = icrc;
	register char *cp = icp;
	register int    cnt = icnt;

	while (cnt--)
	{
#ifndef SWAPPED
		crc = (crc << B) ^ crctab[(crc >> (W - B)) ^ *cp++];
#else
		crc = (crc >> B) ^ crctab[(crc & ((1 << B) - 1)) ^ *cp++];
#endif
	}
	return (crc);
}

#ifdef MAKETAB
main()
{
	initcrctab();
}

int
initcrctab __P((void))
{
	register int    b, i;
	WTYPE           v;

	for (b = 0; b <= (1 << B) - 1; ++b)
	{
#ifndef SWAPPED
		for (v = b << (W - B), i = B; --i >= 0;)
			v = v & ((WTYPE) 1 << (W - 1)) ? (v << 1) ^ P : v << 1;
#else
		for (v = b, i = B; --i >= 0;)
			v = v & 1 ? (v >> 1) ^ P : v >> 1;
#endif
		crctab[b] = v;
		(void) printf("0x%-4lx,", v & ((1L << W) - 1L));
		if ((b & 7) == 7)
			(void) printf("\n");
		else
			(void) printf("  ");
	}
}
#endif /*- #ifdef MAKETAB -*/

void
getversion_crc_c()
{
	printf("%s\n", sccsid);
	return;
}
