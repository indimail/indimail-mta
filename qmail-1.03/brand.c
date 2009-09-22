/*
 * $Log: brand.c,v $
 * Revision 1.2  2004-10-22 20:22:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-10-21 22:48:30+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Silly program to "trivially" (ha!) store uid and gid numbers in correct place 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

typedef struct
{
	char           *name;
} file_list_t;

typedef struct
{
	char           *varname;
	int             ugid;
} number_list_t;

file_list_t     file_list[20];
number_list_t   number_list[20];

int             filebrand(int file, number_list_t * list, int count);
void            usage();

int
main(int argc, char **argv)
{
	int             n;
	int             fd;
	struct passwd  *myp;
	struct group   *myg;
	int             filecount = -1, numbercount = -1;

	if (argc < 4)
		usage();

	for (n = 0; n < argc; n++)
	{
		if (strcmp(argv[n], "-f") == 0)
			if (n + 1 < argc)
			{
				n++;
				file_list[++filecount].name = argv[n];
				continue;
			} else
				usage();

		if (strcmp(argv[n], "-u") == 0)
			if (n + 2 < argc)
			{
				n++;
				number_list[++numbercount].varname = argv[n++];
				if ((myp = getpwnam(argv[n])) == NULL)
				{
					fprintf(stderr, "Can't find user %s\n", argv[n]);
					exit(1);
				}
				number_list[numbercount].ugid = myp->pw_uid;
				continue;
			} else
				usage();

		if (strcmp(argv[n], "-g") == 0)
			if (n + 2 < argc)
			{
				n++;
				number_list[++numbercount].varname = argv[n++];
				if ((myg = getgrnam(argv[n])) == NULL)
				{
					fprintf(stderr, "Can't find group %s\n", argv[n]);
					exit(1);
				}
				number_list[numbercount].ugid = myg->gr_gid;
			} else
				usage();
	}
	filecount++;
	numbercount++;
	for (n = 0; n < filecount; n++)
	{
		if ((fd = open(file_list[n].name, O_RDWR)) == -1)
		{
			fprintf(stderr, "Can't open file %s\n", file_list[n].name);
			exit(1);
		}
		/*
		 * fprintf( stderr, "Searching %s\n", file_list[n].name ); 
		 */
		filebrand(fd, &number_list[0], numbercount);
		close(fd);
	}
	return 0;
}

void
usage(void)
{
	fprintf(stderr, "Usage: brand { -f target_file } ... { [ -u | -g ] string user_or_group } ...\n");
	exit(1);
}

/*
 * Search for multiple strings in open file 
 */
int
filebrand(int file, number_list_t * list, int listcount)
{
	char            c;
	int             i;
	char            buf[BUFSIZ];
	int             pos = 0;

	/*
	 * Go through each character of file, filling in buffer until string terminator
	 * found or buffer is full 
	 */
	while (read(file, &c, sizeof(c)))
	{
		buf[pos++] = c;
		if (c != 0 && pos < sizeof(buf) - 1)
			continue;

		/*
		 * Something found, terminate string and reset pointer 
		 */
		buf[pos] = 0;
		pos = 0;
		/*
		 * Now a string can be compared with each item in list 
		 */
		for (i = 0; i < listcount; i++)
			if (strcmp(buf, list[i].varname) == 0)
			{
				/*
				 * fprintf( stderr, "Found %s writing %d\n", list[i].varname, list[i].ugid ); 
				 */
				if (write(file, &list[i].ugid, sizeof(list[i].ugid)) != sizeof(list[i].ugid))
				{
					fprintf(stderr, "Error writing to file.\n");
					exit(1);
				}
			}
	}
}

void
getversion_brand_c()
{
	static char    *x = "$Id: brand.c,v 1.2 2004-10-22 20:22:17+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
