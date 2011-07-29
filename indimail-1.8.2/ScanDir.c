/*
 * $Log: ScanDir.c,v $
 * Revision 2.7  2011-07-29 09:26:21+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.6  2009-02-18 09:08:04+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.5  2008-07-13 19:47:07+05:30  Cprogrammer
 * 64 bit compilation
 *
 * Revision 2.4  2008-06-24 21:54:50+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.3  2002-12-31 00:12:31+05:30  Cprogrammer
 * added missing closedir()
 *
 * Revision 2.2  2002-12-01 18:51:01+05:30  Cprogrammer
 * added unseen, unread message count
 * verbose = 2 or 4 scans each message
 *
 * Revision 2.1  2002-10-16 23:44:01+05:30  Cprogrammer
 * replaced skip of system files with a function skip_system_files()
 *
 * Revision 1.6  2002-02-23 23:53:11+05:30  Cprogrammer
 * Increased space for printing folders
 * corrected bug where any folder or file ending with 'T' was being treated as trash
 *
 * Revision 1.5  2001-12-22 21:44:51+05:30  Cprogrammer
 * coded added to display no of mails, trash information
 *
 * Revision 1.4  2001-12-19 16:27:06+05:30  Cprogrammer
 * emails marked deleted not to be counted in quota calculation
 *
 * Revision 1.3  2001-11-24 12:17:26+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:22+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: ScanDir.c,v 2.7 2011-07-29 09:26:21+05:30 Cprogrammer Stab mbhangui $";
#endif

static int MailPrint(FILE *, char *, int, int *, int *, int *, int *, int *, int *);
static void print_header(FILE *, char *, mdir_t);

/*
 * verbose
 * 1 - Print Stat for all folders
 * 2 - Print mail headers
 * 3 - Print Stat only for Inbox
 * 4 - Print mail headers only for Inbox
 */
int
ScanDir(fp, homedir, verbose, total, mcount, trashtotal, trashcount, unread, unseen)
	FILE           *fp;
	char           *homedir;
	int             verbose;
	mdir_t         *total, *mcount, *trashtotal, *trashcount, *unread, *unseen;
{
	DIR            *dp;
	struct dirent  *dir;
	struct stat     statbuf;
	int             Ltotal, Mailcount, LTrashcount, LTrashtotal, unreadCount, unseenCount;
	char            DirPath[MAX_BUFF];
	char           *ptr;

	if (!(dp = opendir(homedir)))
	{
		perror(homedir);
		return (1);
	}
	if ((ptr = strrchr(homedir, '/')) != NULL)
		ptr++;
	else
		ptr = homedir;
	if((verbose == 3 || verbose == 4) && strncmp(ptr, "Maildir", 8))
	{
		closedir(dp);
		return(0);
	}
	if(verbose == 2 || verbose == 4)
	{
		if (!strncmp(ptr, "Maildir", 8))
			fprintf(fp, "\nFolder Inbox\n");
		else
			fprintf(fp, "\nFolder %s\n", ptr + 1);
		fprintf(fp, "-------------\n\n");
	} else
	if(verbose == 1 || verbose == 3)
	{
		if (!strncmp(ptr, "Maildir", 8))
			fprintf(fp, "%-25s ", "Inbox");
		else
			fprintf(fp, "%-25s ", ptr + 1);
	}
	Mailcount = Ltotal = LTrashcount = LTrashtotal = unreadCount = unseenCount = 0;
	snprintf(DirPath, MAX_BUFF, "%s/new", homedir);
	MailPrint(fp, DirPath, verbose, &Ltotal, &Mailcount, &LTrashtotal, &LTrashcount, &unreadCount, &unseenCount);
	snprintf(DirPath, MAX_BUFF, "%s/cur", homedir);
	MailPrint(fp, DirPath, verbose, &Ltotal, &Mailcount, &LTrashtotal, &LTrashcount, &unreadCount, &unseenCount);
	snprintf(DirPath, MAX_BUFF, "%s/tmp", homedir);
	MailPrint(fp, DirPath, verbose, &Ltotal, &Mailcount, &LTrashtotal, &LTrashcount, &unreadCount, &unseenCount);
	if(verbose == 1 || verbose == 3)
		fprintf(fp, "%10d  %8d  %10d  %8d %6d %6d\n", Ltotal, Mailcount, LTrashtotal, LTrashcount, unreadCount, unseenCount);
	else
	if((verbose == 2 || verbose == 4) && !Ltotal && !LTrashtotal)
		fprintf(fp, "Empty Folder. No action required for this folder\n");
	*mcount += Mailcount;
	*total += Ltotal;
	*trashcount += LTrashcount;
	*trashtotal += LTrashtotal;
	*unread += unreadCount;
	*unseen += unseenCount;
	for (;;)
	{
		if (!(dir = readdir(dp)))
			break;
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
			continue;
		else
		if (!strcmp(dir->d_name, "cur") || !strcmp(dir->d_name, "new") || !strcmp(dir->d_name, "tmp"))
			continue;
		else
		if(strncmp(dir->d_name, ".Trash", 6) && skip_system_files(dir->d_name))
			continue;
		slen(dir->d_name);
		snprintf(DirPath, MAX_BUFF, "%s/%s", homedir, dir->d_name);
		if (stat(DirPath, &statbuf))
			continue;
		if (S_ISDIR(statbuf.st_mode))
			ScanDir(fp, DirPath, verbose, total, mcount, trashtotal, trashcount, unread, unseen);
	}
	closedir(dp);
	return (0);
}

static int
MailPrint(FILE *fp, char *HomeDir, int verbose, int *total, int *mailcount, int *trashtotal, int *trashcount,
	int *unread, int *unseen)
{
	DIR            *dp;
	char            MailFile[MAX_BUFF];
	char           *ptr;
	int             is_trash;
	struct dirent  *dir;
	struct stat     statbuf;

	if (!(dp = opendir(HomeDir)))
		return (1);
	if(strstr(HomeDir, "/Maildir/.Trash"))
		is_trash = 1;
	else
		is_trash = 0;
	for (;;)
	{
		if (!(dir = readdir(dp)))
			break;
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
			continue;
		snprintf(MailFile, MAX_BUFF, "%s/%s", HomeDir, dir->d_name);
		if (stat(MailFile, &statbuf))
		{
			perror(MailFile);
			continue;
		}
		if (S_ISREG(statbuf.st_mode))
		{
			if((verbose == 2 || verbose == 4) && !*total && !*trashtotal)
			{
				fprintf(fp, "From                                Subject                            Size\n");
				fprintf(fp, "---------------------------------------------------------------------------\n");
			}
			slen(dir->d_name);
			if((ptr = strrchr(dir->d_name, ':')))
				ptr++;
			if(is_trash || (ptr && strchr(ptr, 'T')))
			{
				*trashcount += 1;
				*trashtotal += statbuf.st_size;
			} else
			{
				*mailcount += 1;
				*total += statbuf.st_size;
			}
			if(!ptr)
				*unseen += 1;
			if(!ptr || !strchr(ptr, 'S'))
				*unread += 1;
			if(verbose == 2 || verbose == 4)
				print_header(fp, MailFile, statbuf.st_size);
		}
	}
	closedir(dp);
	return(0);
}

static void
print_header(fp, fname, size)
	FILE           *fp;
	char           *fname;
	mdir_t          size;
{
	FILE           *fpr;
	char           *ptr;
	char            buffer[MAX_BUFF + 2];
	int             fromflag, subjectflag;

	if (!(fpr = fopen(fname, "r")))
	{
		perror(fname);
		return;
	}
	for (fromflag = subjectflag = 0; !fromflag || !subjectflag;)
	{
		if (!fgets(buffer, MAX_BUFF, fpr))
		{
			if (feof(fpr))
				break;
			fprintf(stderr, "print_header: fgets: %s\n", strerror(errno));
			return;
		}
		if (!fromflag && !strncmp(buffer, "From:", 5))
		{
			fromflag = 1;
			if ((ptr = strchr(buffer, '\n')) != NULL)
				*ptr = 0;
			fprintf(fp, "%-35.35s ", buffer + 6);
		} else
		if (!subjectflag && !strncmp(buffer, "Subject:", 8))
		{
			subjectflag = 1;
			if ((ptr = strchr(buffer, '\n')) != NULL)
				*ptr = 0;
			fprintf(fp, "%-30.30s ", buffer + 9);
			break;
		}
	}
	if (!fromflag)
		fprintf(fp, "%-35.35s ", "???");
	if (!subjectflag)
		fprintf(fp, "%-30.30s ", "???");
	fprintf(fp, "%8"PRIu64"\n", size);
	fclose(fpr);
	return;
}

void
getversion_ScanDir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
