/*
 * $Log: vfstab.c,v $
 * Revision 2.5  2008-05-28 17:41:10+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2003-03-05 00:19:53+05:30  Cprogrammer
 * added fix for updation of status
 *
 * Revision 2.3  2002-08-11 12:08:26+05:30  Cprogrammer
 * added display of load
 * added option for file system balancing
 *
 * Revision 2.2  2002-08-11 00:31:33+05:30  Cprogrammer
 * added option to add a local filesystem
 *
 * Revision 2.1  2002-08-07 18:42:19+05:30  Cprogrammer
 * program to administer fstab table
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab.c,v 2.5 2008-05-28 17:41:10+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char            MdaHost[MAX_BUFF];
char            FileSystem[MAX_BUFF];
long            size_quota = -1;
long            user_quota = -1;
int             FStabstatus = -1;

#define FSTAB_SELECT  0
#define FSTAB_INSERT  1
#define FSTAB_DELETE  2
#define FSTAB_UPDATE  3
#define FSTAB_STATUS  4
#define FSTAB_NEWFS   5
#define FSTAB_BALANCE 6

int             FstabAction;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *tmpfstab;
	int             status, flag, retval;
	float           load;
	long            max_users, cur_users, max_size, cur_size;

	if(get_options(argc, argv))
		return(0);
	switch (FstabAction)
	{
	case FSTAB_SELECT:
		for(flag = 0, retval = 1;;)
		{
			if(!(tmpfstab = vfstab_select(MdaHost, &status, &max_users, &cur_users, &max_size, &cur_size)))
				break;
			if(!flag++)
			{
				printf("File System          Host                 Status   Max Users  Cur Users       Max Size        Cur Size Load\n");
				retval = 0;
			}
			load = cur_size ? ((float) (cur_users * 1024 * 1024)/ (float) cur_size) : 0.1;
			printf("%-20s %-20s %s %10ld %10ld  %10ld Kb   %10ld Kb %6.4f\n", tmpfstab, MdaHost, status == FS_ONLINE ? "ONLINE " : "OFFLINE", max_users,
					cur_users, max_size/1024, cur_size/1024, load);
		}
		break;
	case FSTAB_INSERT:
		retval = vfstab_insert(FileSystem, MdaHost, FS_ONLINE, user_quota, size_quota);
		break;
	case FSTAB_DELETE:
		retval = vfstab_delete(FileSystem, MdaHost);
		break;
	case FSTAB_UPDATE:
		retval = vfstab_update(FileSystem, MdaHost, user_quota, size_quota, FStabstatus);
		break;
	case FSTAB_STATUS:
		retval = vfstab_status(FileSystem, MdaHost, FStabstatus);
		break;
	case FSTAB_NEWFS:
		retval = vfstabNew(FileSystem, user_quota, size_quota);
		break;
	case FSTAB_BALANCE:
		if(!(tmpfstab = getFreeFS()))
		{
			fprintf(stderr, "vfstab: balancing failed\n");
			return(1);
		} else
			printf("Filesystem Selected %s\n", tmpfstab);
		retval = 0;
		break;
	default:
		retval = 1;
		printf("error, FSTAB Action is invalid %d\n", FstabAction);
		break;
	}
	return(retval);
}

void
usage()
{
	printf("usage: vfstab [options] [ -d|-i|-l|-u|-o status -n user_quota -q size_quota -m mdaHost filesystem | -s [-m host] | -b ]\n");
	printf("options: -V (print version number)\n");
	printf("         -v (verbose )\n");
	printf("         -d (delete local/remote filesystem)\n");
	printf("         -i (insert local/remote filesystem)\n");
	printf("         -u (update local/remote filesystem)\n");
	printf("         -o (make   local/remote filesystem offline/online, 0 - Offline, 1 - Online)\n");
	printf("         -l (add    local filesystem)\n");
	printf("         -n max number of users\n");
	printf("         -q max size of filesystem\n");
	printf("         -m mdaHost\n");
	printf("         -s [-m mdaHost] (show filesystems)\n");
	printf("         -b (balance filesystems)\n");
}

int
get_options(int argc, char **argv)
{
	int             c;

	memset(MdaHost, 0, MAX_BUFF);
	FstabAction = FSTAB_SELECT;
	while ((c = getopt(argc, argv, "Vvdiusblo:m:q:n:")) != -1)
	{
		switch (c)
		{
		case 'V':
			getversion(sccsid);
			break;
		case 'v':
			verbose = 1;
			break;
		case 's':
			FstabAction = FSTAB_SELECT;
			break;
		case 'd':
			FstabAction = FSTAB_DELETE;
			break;
		case 'i':
			FstabAction = FSTAB_INSERT;
			break;
		case 'u':
			FstabAction = FSTAB_UPDATE;
			break;
		case 'l':
			FstabAction = FSTAB_NEWFS;
			break;
		case 'm':
			scopy(MdaHost, optarg, MAX_BUFF);
			break;
		case 'q':
			size_quota = atol(optarg);
			break;
		case 'n':
			user_quota = atol(optarg);
			break;
		case 'o':
			if(FstabAction == FSTAB_SELECT)
				FstabAction = FSTAB_STATUS;
			FStabstatus = atol(optarg);
			if(FStabstatus != 0 && FStabstatus != 1)
			{
				fprintf(stderr, "vfstab: status should be 0 or 1\n");
				usage();
				return(1);
			}
			break;
		case 'b':
			FstabAction = FSTAB_BALANCE;
			break;
		default:
			usage();
			return(1);
		}
	}
	if (optind < argc)
		scopy(FileSystem, argv[optind++], MAX_BUFF);
	if((FstabAction != FSTAB_SELECT && FstabAction != FSTAB_BALANCE) && !*FileSystem)
	{
		fprintf(stderr, "vfstab: Must specify FileSystem\n");
		usage();
		return(1);
	} else
	if(FstabAction != FSTAB_SELECT && FstabAction != FSTAB_NEWFS && FstabAction != FSTAB_BALANCE && !*MdaHost)
	{
		fprintf(stderr, "vfstab: must supply MDA IP Address\n");
		usage();
		return(1);
	} else
	if(FstabAction == FSTAB_INSERT && (size_quota == -1 && user_quota == -1))
	{
		fprintf(stderr, "vfstab: Must specify Max Size or Max Users\n");
		usage();
		return(1);
	} else
	if(FstabAction == FSTAB_UPDATE && (size_quota == -1 && user_quota == -1))
	{
		fprintf(stderr, "vfstab: Must specify Max Size or Max Users\n");
		usage();
		return(1);
	} 
	return(0);
}

void
getversion_vfstab_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
