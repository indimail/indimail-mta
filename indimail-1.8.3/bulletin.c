/*
 * $Log: bulletin.c,v $
 * Revision 2.12  2010-06-07 18:31:28+05:30  Cprogrammer
 * additional connect_all argument to findmdahost()
 *
 * Revision 2.11  2009-02-06 11:36:37+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.10  2008-10-29 11:12:49+05:30  Cprogrammer
 * disable mysql_escape
 *
 * Revision 2.9  2008-09-08 09:31:21+05:30  Cprogrammer
 * do not allow absolute path for sql import file
 *
 * Revision 2.8  2008-05-28 16:33:40+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.7  2004-09-20 19:53:13+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.6  2004-05-17 14:00:35+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.5  2003-02-27 23:55:14+05:30  Cprogrammer
 * minor bug fix
 *
 * Revision 2.4  2002-10-27 21:13:53+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-08-03 04:25:45+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.2  2002-06-26 03:17:01+05:30  Cprogrammer
 * corrections for non-distributed code
 *
 * Revision 2.1  2002-04-15 19:34:01+05:30  Cprogrammer
 * function for mail bulletin
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: bulletin.c,v 2.12 2010-06-07 18:31:28+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>

struct mdahosts
{
	char            mdahost[DBINFO_BUFF];
	long            emailcount;
	char          **emailptr;
};

struct mdahosts **StoreEmail(char *, char *, char *);
static long     insert_bulletin(char *, char *, char *);

struct mdahosts **MdaHosts = (struct mdahosts **) 0;

long
bulletin(char *emailFile, char *subscriber_list)
{
	FILE           *fp;
	char           *domain, *ptr, *cptr;
	char          **emailptr;
	char            buffer[DBINFO_BUFF], TmpBuf[DBINFO_BUFF], SplitFile[MAX_BUFF];
	struct mdahosts **mdaptr;
	MYSQL         **mysqlptr;
	uid_t           uid;
	gid_t           gid;
	int             err, fd, count, ret;

	if (!(fp = fopen(subscriber_list, "r")))
	{
		perror(subscriber_list);
		return (-1);
	}
	for (count = err = 0;;)
	{
		if (!fgets(buffer, DBINFO_BUFF - 2, fp))
			break;
		if ((ptr = strchr(buffer, '\n')) || (ptr = strchr(buffer, '#')))
			*ptr = 0;
		for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!(ptr = strchr(buffer, '@')))
		{
			fprintf(stderr, "Invalid email address %s\n", buffer);
			err++;
			continue;
		} else
			domain = ptr + 1;
		for (ptr = findmdahost(buffer, 0); ptr && *ptr && *ptr != ':'; ptr++);
		if (ptr)
		{
			ptr++;
			for (cptr = ptr; cptr && *cptr && *cptr != ':'; cptr++);
			*cptr = 0;
			if (StoreEmail(ptr, domain, buffer))
			{
				err++;
				continue;
			}
		} else
			fprintf(stderr, "%s: No such user\n", buffer);
	} /*- for(;;) */
	fclose(fp);
	for (mdaptr = MdaHosts; mdaptr && *mdaptr; mdaptr++)
	{
		emailptr = (*mdaptr)->emailptr;
		if ((domain = strchr(*emailptr, '@')))
			domain++;
		else
		{
			fprintf(stderr, "Invalid Mda at %s\n", *emailptr);
			err++;
			continue;
		}
		if (!vget_assign(domain, 0, 0, &uid, &gid))
		{
			fprintf(stderr, "%s: No such domain\n", domain);
			continue;
		}
		snprintf(SplitFile, MAX_BUFF, "/tmp/indi.%s.XXXXXX", (*mdaptr)->mdahost);
		if ((fd = mkstemp(SplitFile)) == -1 || !(fp = fdopen(fd, "w")))
		{
			perror(SplitFile);
			unlink(SplitFile);
			return (-1);
		} else
		if (chown(SplitFile, uid, gid) || chmod(SplitFile, INDIMAIL_QMAIL_MODE))
		{
			perror(SplitFile);
			unlink(SplitFile);
			return (-1);
		}
		for (; *emailptr; emailptr++)
			fprintf(fp, "%s\n", *emailptr);
		fclose(fp);
		scopy(TmpBuf, (*mdaptr)->mdahost, DBINFO_BUFF);
		if ((ptr = strchr(TmpBuf, '@')))
			*ptr = 0;
		if (!(mysqlptr = mdaMysqlConnect(TmpBuf, domain)))
		{
			fprintf(stderr, "Unable to locate Mysql Host for %s\n", TmpBuf);
			err++;
			unlink(SplitFile);
			continue;
		}
		vauth_init(1, *mysqlptr);
		if ((ret = insert_bulletin(domain, emailFile, SplitFile)) == -1)
			err++;
		else
			count += ret;
		unlink(SplitFile);
		is_open = 0; /*- do not close main connection set by mysqlptr */
	} /*- for(mdaptr = MdaHosts;*mdaptr;mdaptr++) */
	return (err ? -1 : count);
}

struct mdahosts **
StoreEmail(char *host, char *domain, char *email)
{
	static struct mdahosts **mdaptr;
	static int      mdacount;
	int             len, emailcount;
	char            tmpbuf[DBINFO_BUFF];


	snprintf(tmpbuf, DBINFO_BUFF, "%s@%s", host, domain);
	for (mdaptr = MdaHosts; mdaptr && *mdaptr; mdaptr++)
	{
		if (!strncmp((*mdaptr)->mdahost, tmpbuf, DBINFO_BUFF))
			break;
	}
	if (!mdaptr || !(*mdaptr))
	{
		if (!(MdaHosts = (struct mdahosts **) realloc(MdaHosts, (mdacount + 1) * sizeof(struct mdahosts *))))
		{
			perror("malloc");
			return ((struct mdahosts **) 0);
		}
		if (!(MdaHosts[mdacount] = (struct mdahosts *) malloc(sizeof(struct mdahosts))))
		{
			perror("malloc");
			return ((struct mdahosts **) 0);
		}
		snprintf(MdaHosts[mdacount]->mdahost, DBINFO_BUFF, "%s@%s", host, domain);
		if (!(MdaHosts[mdacount]->emailptr = (char **) malloc(2 * sizeof(char *))))
		{
			perror("malloc");
			return ((struct mdahosts **) 0);
		} else
		if (!(MdaHosts[mdacount]->emailptr[0] = (char *) malloc(len = (slen(email) + 1))))
		{
			perror("malloc");
			return ((struct mdahosts **) 0);
		} else
			scopy(MdaHosts[mdacount]->emailptr[0], email, len);
		MdaHosts[mdacount]->emailcount = 1;
		MdaHosts[mdacount]->emailptr[1] = (char *) 0;
		MdaHosts[mdacount + 1] = (struct mdahosts *) 0;
		mdacount++;
	} else
	{
		emailcount = (*mdaptr)->emailcount + 1;
		if (!((*mdaptr)->emailptr = (char **) realloc((*mdaptr)->emailptr, (emailcount + 1) * sizeof(char *))))
		{
			perror("malloc");
			return ((struct mdahosts **) 0);
		} else
		if (!((*mdaptr)->emailptr[emailcount - 1] = (char *) malloc(len = (slen(email) + 1))))
		{
			perror("malloc");
			return ((struct mdahosts **) 0);
		} else
			scopy((*mdaptr)->emailptr[emailcount - 1], email, len);
		(*mdaptr)->emailcount = emailcount;
		(*mdaptr)->emailptr[emailcount] = (char *) 0;
	}
	return (0);
}
#else
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

static long     insert_bulletin(char *, char *, char *);

long
bulletin(char *emailFile, char *subscriber_list)
{
	FILE           *fp;
	char           *ptr, *domain;
	char            buffer[MAX_BUFF];

	if (!(fp = fopen(emailFile, "r")))
	{
		perror(emailFile);
		return (-1);
	}
	for (domain = (char *) 0;;)
	{
		if (!fgets(buffer, MAX_BUFF - 2, fp))
			break;
		if ((ptr = strchr(buffer, '\n')) || (ptr = strchr(buffer, '#')))
			*ptr = 0;
		if (!*buffer)
			continue;
		if ((domain = strrchr(buffer, '@')))
		{
			domain++;
			break;
		}
	}
	if(!domain)
	{
		fprintf(stderr, "No domain specified\n");
		return(-1);
	}
	fclose(fp);
	return (insert_bulletin(domain, emailFile, subscriber_list));
}
#endif /*- #ifdef CLUSTERED_SITE */

#include <errno.h>
#include <mysqld_error.h>
static long
insert_bulletin(char *domain, char *emailFile, char *list_file)
{
	char            EmailFile[MAX_BUFF], TmpBuf[MAX_BUFF], buffer[MAX_BUFF], SqlBuf[SQL_BUF_SIZE];
	char           *ptr;
	int             fd, es_opt;
	uid_t           uid;
	gid_t           gid;
	long            row_count;
	FILE           *rfp, *wfp;

	if ((strchr(emailFile, '/')))
	{
		fprintf(stderr, "%s contains '/'. Filename cannot have any path\n", emailFile);
		return (-1);
	}
	snprintf(EmailFile, MAX_BUFF, "%s/control/%s/%s/%s", INDIMAILDIR, domain,
			 (((ptr = getenv("BULK_MAILDIR"))) ? ptr : BULK_MAILDIR), emailFile);
	if (!vget_assign(domain, 0, 0, &uid, &gid))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return (-1);
	} else
	if (!(rfp = fopen(list_file, "r")))
	{
		perror(list_file);
		return (-1);
	} else
	if (access(emailFile, F_OK))
	{
		perror(emailFile);
		return (-1);
	} else
	if (!access(EmailFile, F_OK))
	{
		errno = EEXIST;
		perror(EmailFile);
		return (-1);
	}
	scopy(TmpBuf, "/tmp/indiXXXXXX", MAX_BUFF);
	if ((fd = mkstemp(TmpBuf)) == -1 || !(wfp = fdopen(fd, "w")))
	{
		perror(TmpBuf);
		unlink(TmpBuf);
		return (-1);
	} else
	if (chown(TmpBuf, uid, gid) || chmod(TmpBuf, INDIMAIL_QMAIL_MODE))
	{
		perror(TmpBuf);
		unlink(TmpBuf);
		return (-1);
	}
	for (;;)
	{
		if (!fgets(buffer, MAX_BUFF - 2, rfp))
			break;
		if ((ptr = strchr(buffer, '\n')) || (ptr = strchr(buffer, '#')))
			*ptr = 0;
		if (!*buffer)
			continue;
		if ((ptr = strchr(buffer, '@')))
		{
			ptr++;
			if (strncmp(ptr, domain, MAX_BUFF))
			{
				fprintf(stderr, "Skipping email %s - does not belong to %s\n", buffer, domain);
				continue;
			}
			fprintf(wfp, "%s %s\n", buffer, emailFile);
		} else
			fprintf(wfp, "%s@%s %s\n", buffer, domain, emailFile);
	}
	fclose(rfp);
	fclose(wfp);
	if (vauth_open((char *) 0))
	{
		unlink(TmpBuf);
		return (-1);
	}
	if (fappend(emailFile, EmailFile, "w", INDIMAIL_QMAIL_MODE, uid, gid))
	{
		fprintf(stderr, "fappend: %s->%s: %s\n", emailFile, EmailFile, strerror(errno));
		unlink(TmpBuf);
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "load data infile \"%s\" into table bulkmail fields terminated by ' ' \
		lines terminated by '\\n'", TmpBuf);
	es_opt = disable_mysql_escape(1);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "bulkmail", BULKMAIL_TABLE_LAYOUT))
			{
				unlink(TmpBuf);
				unlink(EmailFile);
				disable_mysql_escape(es_opt);
				return (-1);
			}
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("insert_bulletin: mysql_query: %s: %s", SqlBuf, mysql_error(&mysql[1]));
				unlink(TmpBuf);
				unlink(EmailFile);
				disable_mysql_escape(es_opt);
				return (-1);
			}
		} else
		{
			unlink(TmpBuf);
			mysql_perror("insert_bulletin: mysql_query: %s: %s", SqlBuf, mysql_error(&mysql[1]));
			unlink(EmailFile);
			disable_mysql_escape(es_opt);
			return (-1);
		}
	}
	disable_mysql_escape(es_opt);
	unlink(TmpBuf);
	if ((row_count = mysql_affected_rows(&mysql[1])) == -1)
	{
		fprintf(stderr, "insert_bulletin: mysql_affected_rows: %s\n", mysql_error(&mysql[1]));
		unlink(EmailFile);
		return (-1);
	}
	return (row_count);
}

void
getversion_bulletin_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
