/*
 * $Log: crcdiff.c,v $
 * Revision 2.2  2009-02-18 09:06:34+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.1  2002-09-27 13:18:11+05:30  Cprogrammer
 * Crcdiff Application
 *
 */
/*-
 * Functions for Host Access. Manvendra
 * This progam will compare two crc lists and report the differences.
 * 
 * By Jon Zeeff (zeeff@b-tech.ann-arbor.mi.us)
 * 
 * Permission is granted to use this in any manner provided that    
 * 1) the copyright notice is left intact,
 * 2) you don't hold me responsible for any bugs and 
 * 3) you mail me any improvements that you make.
 * 
 * 
 * report:
 *     corrupt  - crc changed w/o date change 
 *     replaced - crc + date changed
 *     perm     - permissions changed
 *     own/grp  - owner or group changed
 *     removed  - 
 *     added    -
 *  Print the info for the new file except for deleted.
 * 
 * Use:
 * 
 * find / -print | sort | xargs crc -v > crc_file
 * 
 * to generate a crc list (crc.c should accompany this source).
 * 
 * Assume that no files have tabs or spaces in the name.
 * 
 -*/

/*
 * sequent stuff -- may or may not need it?  Worked fine without it on a
 * sequent I had, but others claim they need it.  Go figure.
 * 
 */
#ifdef sequent
#define strrchr(s, c)  rindex(s,c)
#endif
#ifndef CNULL
#define CNULL 0
#endif

/*
 * max size of line 
 */

#define BUF_SIZE 1124

#include <stdio.h>
#include <string.h>

char           *strrchr();
void            exit();

char            new_line[BUF_SIZE];
char            old_line[BUF_SIZE];


#ifndef	lint
static char     sccsid[] = "$Id: crcdiff.c,v 2.2 2009-02-18 09:06:34+05:30 Cprogrammer Stab mbhangui $";
#endif

int
main(argc, argv)
	int             argc;
	char          **argv;
{

	char           *new_ptr;
	char           *old_ptr;
	FILE           *newfp;
	FILE           *oldfp;
	int             match, count;
	unsigned long   oldcrc, newcrc, diffcrc, modicount;
	char            tmpcrc[6];
	/*-
       	If line =, read new line from each file
       else
		If date/perm/crc change, report and read new line from each file
       else
        If old_line < new_line, report file removed, read old line
       else
        report new line as added
              read new_line
        loop
      -*/

	if (argc != 3)
	{
		(void) printf("crcdiff old_crc_file new_crc_file\n");
		exit(1);
	}
	if (!(newfp = fopen(argv[2], "r")))
	{
		perror(argv[2]);
		return (1);
	}
	if (!(oldfp = fopen(argv[1], "r")))
	{
		perror(argv[1]);
		return (1);
	}
	modicount = 0;
	for (diffcrc = oldcrc = 0l;;)
	{
		if (!fgets(old_line, BUF_SIZE, oldfp))
		{
			if (feof(oldfp))
				break;
			perror("fgets");
			return (1);
		}
		/*
		 * Compare just the file names 
		 */
		if (!(old_ptr = strrchr(old_line, ' ')))
		{
			(void) printf("Error in input data\n");
			exit(1);
		}
		strncpy(tmpcrc, old_line, 4);
		tmpcrc[4] = CNULL;
		sscanf(tmpcrc, "%x", &count);
		oldcrc += count;
		for (count = match = 0;; count++)
		{
			if (!fgets(new_line, BUF_SIZE, newfp))
			{
				if (feof(newfp))
				{
					rewind(newfp);
					break;
				}
				perror("fgets");
				return (1);
			}
			if (!strcmp(old_line, new_line))
			{
				match = 1;
				break;
			}
			if (!(new_ptr = strrchr(new_line, ' ')))
			{
				(void) printf("Error in input data\n");
				exit(1);
			}
			if (!strcmp(old_ptr, new_ptr))
			{
				match = 1;
				new_ptr = strrchr(new_line, '\t');
				old_ptr = strrchr(old_line, '\t');
				if (!new_ptr || !old_ptr)
				{
					(void) printf("Error in input data\n");
					return (1);
				}
				/*
				 * check crc change 
				 */
				if (strncmp(new_line, old_line, 4))
				{
					strncpy(tmpcrc, new_line, 4);
					tmpcrc[4] = CNULL;
					sscanf(tmpcrc, "%x", &count);
					diffcrc += count;
					strncpy(tmpcrc, old_line, 4);
					tmpcrc[4] = CNULL;
					sscanf(tmpcrc, "%x", &count);
					diffcrc -= count;
					if (!strcmp(new_ptr, old_ptr))
						(void) printf("corrupt    %s", new_line + 5);
					else
						(void) printf("replaced   %s", new_line + 5);
					modicount++;
				}
				/*
				 * check permission chenage 
				 */
				if (strncmp(new_line + 5, old_line + 5, 11))
				{
					(void) printf("permiss <  %s", old_line + 5);
					(void) printf("permiss >  %s", new_line + 5);
					modicount++;
					fflush(stdout);
				}
				/*
				 * check  owner/group 
				 */
				if (strncmp(new_line + 16, old_line + 16, new_ptr - new_line - 15))
				{
					(void) printf("own/grp <  %s", old_line + 5);
					(void) printf("own/grp >  %s", new_line + 5);
					modicount++;
					fflush(stdout);
				}
				break;
			} else
			if (!count)
				rewind(newfp);
		}/*- end of for(match = 0;;) -*/
		if (!match)
		{
			strncpy(tmpcrc, old_line, 4);
			tmpcrc[4] = CNULL;
			sscanf(tmpcrc, "%x", &count);
			diffcrc -= count;
			(void) printf("removed    %s", old_line + 5);
			modicount++;
		}
	}
	rewind(newfp);
	rewind(oldfp);
	for (newcrc = 0l;;)
	{
		if (!fgets(new_line, BUF_SIZE, newfp))
		{
			if (feof(newfp))
				break;
			perror("fgets");
			exit (1);
		}
		if (!(new_ptr = strrchr(new_line, ' ')))
		{
			(void) printf("Error in input data\n");
			exit(1);
		}
		strncpy(tmpcrc, new_line, 4);
		tmpcrc[4] = CNULL;
		sscanf(tmpcrc, "%x", &count);
		newcrc += count;
		for (count = match = 0;; count++)
		{
			if (!fgets(old_line, BUF_SIZE, oldfp))
			{
				if (feof(oldfp))
				{
					rewind(oldfp);
					break;
				}
				perror("fgets");
				exit (1);
			}
			if (!strcmp(old_line, new_line))
			{
				match = 1;
				break;
			}
			if (!(old_ptr = strrchr(old_line, ' ')))
			{
				(void) printf("Error in input data\n");
				exit(1);
			}
			if (!strcmp(old_ptr, new_ptr))
			{
				match = 1;
				break;
			} else
			if (!count)
				rewind(oldfp);
		}						/*
								 * end of for(match = 0;;) 
								 */
		if (!match)
		{
			strncpy(tmpcrc, new_line, 4);
			tmpcrc[4] = CNULL;
			sscanf(tmpcrc, "%x", &count);
			diffcrc += count;
			(void) printf("added      %s", new_line + 5);
			modicount++;
		}
	}
	if (!modicount)
		return (0);
	printf("----------------------------------------------------\n");
	printf("1. Current      Checksum : %ld\n", newcrc);
	printf("2. Old          Checksum : %ld\n", oldcrc);
	printf("3. Net Difference  (Cal) : %ld\n", diffcrc);
	printf("5. Current CRC - Old CRC : %ld\n", newcrc - oldcrc);
	printf("6. Total         Changes : %ld\n", modicount);
	printf("----------------------------------------------------\n");
	if (diffcrc == (newcrc - oldcrc))
		printf("Checksum Reconcialation Done\n");
	else
		printf("WARNING: Checksum Reconcialation failed\n");
	return (0);
}

void
getversion_crcdiff_c()
{
	printf("%s\n", sccsid);
}
