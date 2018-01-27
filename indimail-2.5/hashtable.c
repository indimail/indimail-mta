/*
 * $Log: hashtable.c,v $
 * Revision 2.5  2008-06-13 08:54:31+05:30  Cprogrammer
 * changes for MAKE_SEEKABLE
 *
 * Revision 2.4  2004-10-08 18:42:25+05:30  Cprogrammer
 * unbuffered stdout
 * treat threshold=0 as a valid threshold
 *
 * Revision 2.3  2004-10-08 14:30:29+05:30  Cprogrammer
 * speed up by making stdin seekable and printing hash table at the end
 *
 * Revision 2.2  2002-12-27 11:58:36+05:30  Cprogrammer
 * code optimization by removing redundant block
 *
 * Revision 2.1  2002-12-25 22:54:00+05:30  Cprogrammer
 * utility to hash entries given on stdin and maintain a count against each entry
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <search.h>

#ifndef	lint
static char     sccsid[] = "$Id: hashtable.c,v 2.5 2008-06-13 08:54:31+05:30 Cprogrammer Stab mbhangui $";
#endif

static int      get_options(int, char **, int *, int *);
#ifdef MAKE_SEEKABLE
int             makeseekable(FILE *);
#endif

int
main(int argc, char **argv)
{
	ENTRY           e, *ep;
	int             i, len, threshold, hash_size, seekable = 0;
	char            buffer[1024];
	char          *ptr;

	if (get_options(argc, argv, &hash_size, &threshold))
		return(1);
#ifdef MAKE_SEEKABLE
	if (getenv("MAKESEEKABLE"))
	{
		if (makeseekable(stdin))
			return(1);
		seekable = 1;
	}
#else
	if (lseek(0, 0L, SEEK_CUR) >= 0 && !isatty(0))
		seekable = 1;
#endif
	/*
	 * starting with small table, and letting it grow does not work 
	 */
	hcreate(hash_size);
	setbuf(stdout, 0);
	for (;;)
	{
		if (!fgets(buffer, sizeof(buffer) - 2, stdin))
			break;
		if ((ptr = strchr(buffer, '\n')))
			*ptr = 0;
		if (!(ptr = malloc((len = strlen(buffer)) + 1)))
		{
			perror("malloc");
			return(1);
		}
		strncpy(ptr, buffer, len + 1);
		e.key = ptr;
		if (!(ep = hsearch(e, FIND)))
		{
			i = 1;
			e.data = (char *) i;
			ep = hsearch(e, ENTER);
		} else
			(ep->data)++;
		if (ep == NULL)
		{
			fprintf(stderr, "hashtable: hash search failed\n");
			exit(1);
		}
		if (!seekable)
		{
			e.key = buffer;
			ep = hsearch(e, FIND);
			if (ep && (int) ep->data > threshold)
				printf("%s  %d\n", e.key, (int) (ep->data));
		}
	}
	if (seekable)
	{
		rewind(stdin);
		for (;;)
		{
			if (!fgets(buffer, sizeof(buffer) - 2, stdin))
				break;
			if ((ptr = strchr(buffer, '\n')))
				*ptr = 0;
			e.key = buffer;
			ep = hsearch(e, FIND);
			if (ep && (int) ep->data > threshold)
			{
				printf("%s  %d\n", e.key, (int) (ep->data));
				*(ep->key) = 0;
			}
		}
	}
	return 0;
}

static int
get_options(int argc, char **argv, int *hashSize, int *threshold)
{
	int             c;

	*hashSize = 0;
	*threshold = -1;
	while ((c = getopt(argc, argv, "Vvl:c:")) != -1)
	{
		switch (c)
		{
		case 'l':
			*hashSize = atoi(optarg);
			break;
		case 'c':
			*threshold = atoi(optarg);
			break;
		default:
			fprintf(stderr, "USAGE: hashtable -l lineCount -c ReportCount\n");
			return (1);
		}
	}
	if (!*hashSize || *threshold == -1)
	{
		fprintf(stderr, "USAGE: hashtable -l lineCount -c ReportCount\n");
		return (1);
	}
	return (0);
}

void
getversion_hashtable_c()
{
	printf("%s\n", sccsid);
	return;
}
