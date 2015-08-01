/*
 * $Log: updatefile.c,v $
 * Revision 2.5  2011-11-09 19:45:38+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.4  2009-01-16 14:54:47+05:30  Cprogrammer
 * option to delete all matching lines
 *
 * Revision 2.3  2008-11-21 16:04:03+05:30  Cprogrammer
 * changed mode_t to unsigned int
 *
 * Revision 2.2  2003-06-22 14:21:26+05:30  Cprogrammer
 * bug fix - wrong order of arguments to remove_line()
 *
 * Revision 2.1  2003-06-22 10:47:32+05:30  Cprogrammer
 * program to update/delete lines from a file
 * useful for updating control files
 *
 */
#include "indimail.h"
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: updatefile.c,v 2.5 2011-11-09 19:45:38+05:30 Cprogrammer Stab mbhangui $";
#endif

static int      get_options(int argc, char **argv, char **, char **, char **, unsigned int *, int *, int *);

int
main(int argc, char **argv)
{
	unsigned int    mode;
	int             display, all;
	char          *filename, *updateline, *deleteline;
	char           buffer[MAX_BUFF];
	FILE          *fp;

	filename = updateline = deleteline = (char *) 0;
	mode = 0644;
	if(get_options(argc, argv, &filename, &updateline, &deleteline, &mode, &display, &all))
	{
		fprintf(stderr, "USAGE: updatefile [-s] [-a] [-u updateline] [-d deleteline] -m mode filename\n");
		fprintf(stderr, "-s Display File\n");
		fprintf(stderr, "-u updateline - Update File with updateline\n");
		fprintf(stderr, "-d deleteline - Delete line deleteline (first match)\n");
		fprintf(stderr, "-a Delete all matching Lines\n");
		fprintf(stderr, "-m mode       - Set permission on file\n");
		return(1);
	}
	if (deleteline && remove_line(deleteline, filename, all == 1 ? 0 : 1, mode) == -1)
		return (1);
	if (updateline && update_file(filename, updateline, mode))
		return (1);
	if(display)
	{
		if(!(fp = fopen(filename, "r")))
		{
			perror(filename);
			return(1);
		}
		for(;;)
		{
			if(!fgets(buffer, sizeof(buffer) - 2, fp))
				break;
			printf("%s", buffer);
		}
		fclose(fp);
	}
	return (0);
}

static int
get_options(int argc, char **argv, char **filename, char **updateLine, char **deleteLine, unsigned int *mode, int *display, int *all)
{
	int             c;
	int             errflag;

	errflag = 0;
	*all = 0;*filename = *updateLine = *deleteLine = 0;
	*mode = 0644;
	*display = 0;
	while (!errflag && (c = getopt(argc, argv, "vsau:d:m:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'a':
			*all = 1;
			break;
		case 's':
			*display = 1;
			break;
		case 'u':
			*updateLine = optarg;
			break;
		case 'd':
			*deleteLine = optarg;
			break;
		case 'm':
			sscanf(optarg, "%o", mode);
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag == 1)
		return(1);
	if (optind < argc)
		*filename = argv[optind];
	if(!*filename || !*mode)
		return(1);
	return(0);
}

void
getversion_updatefile_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
