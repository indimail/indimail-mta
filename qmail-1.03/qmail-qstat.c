/*
 * $Log: qmail-qstat.c,v $
 * Revision 1.17  2008-06-06 20:47:13+05:30  Cprogrammer
 * handle options before calling qmHandle
 *
 * Revision 1.16  2004-12-27 22:18:35+05:30  Cprogrammer
 * skip spaces before calling scan_int()
 *
 * Revision 1.15  2004-10-22 20:28:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.14  2004-09-22 23:13:30+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.13  2004-07-17 21:21:15+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "scan.h"
#include "auto_qmail.h"

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	FILE           *fp;
	char            cmmd[128], buffer[128], Qbase[128];
	char           *ptr, *queue_count_ptr, *queue_start_ptr, *qbase;
	int             i, idx, qstart, qcount, count, status = -1, found;
	int             local_count, remote_count, todo_count, bounce_count, run_once;

	if (chdir(auto_qmail) == -1)
	{
		fprintf(stderr, "chdir: %s: %s\n", auto_qmail, strerror(errno));
		return(1);
	}
	run_once = 0;
	while ((i = getopt(argc, argv, "alLRsq:m:d:f:S:h:b:H:B:t:DVcN")) != -1)
	{
		switch(i)
		{
		case 'q':
			run_once = 1;
			break;
		case '?':
			return(1);
		}
	}
	if (!(ptr = getenv("CONTROLDIR")))
		ptr = "control";
	snprintf(buffer, sizeof(buffer), "%s/%s/queue_base", auto_qmail, ptr);
	if ((fp = fopen(buffer, "r")))
	{
		if (!fgets(Qbase, sizeof(Qbase) - 2, fp))
		{
			qbase = (char *) 0;
			fclose(fp);
		} else
		{
			if ((ptr = strchr(Qbase, '\n')))
				*ptr = 0;
			qbase = Qbase;
			fclose(fp);
		}
	} else
		qbase = (char *) 0;
	if (!qbase && !(qbase = getenv("QUEUE_BASE")))
		qbase = auto_qmail;
	if(!(queue_count_ptr = getenv("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	if(!(queue_start_ptr = getenv("QUEUE_START")))
		qstart = 1;
	else
		scan_int(queue_start_ptr, &qstart);
	local_count = remote_count = bounce_count = todo_count = 0;
	for (found = 0, idx = qstart, count = 1; count <= (qcount + 1); count++, idx++)
	{
		if (count == qcount + 1)
			snprintf(buffer, sizeof(buffer), "%s/nqueue", qbase);
		else
			snprintf(buffer, sizeof(buffer), "%s/queue%d", qbase, idx);
		if (access(buffer, F_OK))
			continue;
		if (run_once == 1)
		{
			snprintf(cmmd, sizeof(buffer), "%s/bin/qmail-qmHandle -s", auto_qmail);
			if (argc > 1)
				for (i = 1; i < argc; i++)
				{
					strcat(cmmd, " ");
					strcat(cmmd, argv[i]);
				}
		} else
		{
			if (argc == 1)
			{
				if (count == qcount + 1)
					snprintf(cmmd, sizeof(buffer), "%s/bin/qmail-qmHandle -q%s/nqueue -s", auto_qmail, qbase);
				else
					snprintf(cmmd, sizeof(buffer), "%s/bin/qmail-qmHandle -q%s/queue%d -s", auto_qmail, qbase, idx);
			} else
			{
				if (count == qcount + 1)
					snprintf(cmmd, sizeof(buffer), "%s/bin/qmail-qmHandle -q%s/nqueue", auto_qmail, qbase);
				else
					snprintf(cmmd, sizeof(buffer), "%s/bin/qmail-qmHandle -q%s/queue%d", auto_qmail, qbase, idx);
				for (i = 1; i < argc; i++)
				{
					strcat(cmmd, " ");
					strcat(cmmd, argv[i]);
				}
			}
		}
		if (run_once == 1)
			printf("Mail queue\n");
		else
		if (count == qcount + 1)
			printf("Notification Queue\n");
		else
			printf("Mail queue%d\n", idx);
		if (!(fp = popen(cmmd, "r")))
		{
			perror(cmmd);
			return (1);
		}
		for (status = -1;;)
		{
			fgets(buffer, sizeof(buffer) - 2, fp);
			if (feof(fp))
				break;
			if ((ptr = strstr(buffer, "Messages in local  queue")))
			{
				found++;
				for (ptr += 24;*ptr && *ptr != ':';ptr++);
				if(*ptr)
				{
					for (ptr++;isspace((int) *ptr);ptr++);
					scan_int(ptr, &i);
					local_count += i;
				}
			} else
			if ((ptr = strstr(buffer, "Messages in remote queue")))
			{
				found++;
				for (ptr += 24;*ptr && *ptr != ':';ptr++);
				if(*ptr)
				{
					for (ptr++;isspace((int) *ptr);ptr++);
					scan_int(ptr, &i);
					remote_count += i;
				}
			} else
			if ((ptr = strstr(buffer, "Messages in todo   queue")))
			{
				found++;
				for (ptr += 24;*ptr && *ptr != ':';ptr++);
				if(*ptr)
				{
					for (ptr++;isspace((int) *ptr);ptr++);
					scan_int(ptr, &i);
					todo_count += i;
				}
			} else
			if ((ptr = strstr(buffer, "Messages in bounce queue")))
			{
				found++;
				for (ptr += 24;*ptr && *ptr != ':';ptr++);
				if(*ptr)
				{
					for (ptr++;isspace((int) *ptr);ptr++);
					scan_int(ptr, &i);
					bounce_count += i;
				}
			}
			printf("%s", buffer);
		}
		status = pclose(fp);
		printf("\n");
		if (run_once == 1)
			break;
	}
	if(found)
	{
		printf("Total Messages in local  queue: %d\n", local_count);
		printf("Total Messages in remote queue: %d\n", remote_count);
		printf("Total Messages in bounce queue: %d\n", bounce_count);
		printf("Total Messages in todo   queue: %d\n", todo_count);
		printf("Total Messages in        queue: %d\n", local_count + remote_count + bounce_count + todo_count);
	}
	return(status);
}

void
getversion_qmail_qstat_c()
{
	static char    *x = "$Id: qmail-qstat.c,v 1.17 2008-06-06 20:47:13+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
