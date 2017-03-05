/*
** Copyright 2001-2008 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"numlib/numlib.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<ctype.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#include	<errno.h>
#include	<sys/time.h>
#if	HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if	HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include	"tlsclient.h"



#define ERRMSG(s) (cinfo->errmsg[0]=0, \
		strncat(cinfo->errmsg, (s), sizeof(cinfo->errmsg)-3))

#define SYSERRMSG (strncat(strcpy(cinfo->errmsg, "Failed: "), \
		strerror(errno), sizeof(cinfo->errmsg)-15))

void couriertls_init(struct couriertls_info *cinfo)
{
	memset(cinfo, 0, sizeof(*cinfo));
	cinfo->cipher=cinfo->version="Unknown";
}

/*
** Convenient function to start couriertls, return any client certificate
** error message, and the x509 certificate info.
*/

static int do_couriertls_start(char **, struct couriertls_info *);

int couriertls_start(char **args, struct couriertls_info *cinfo)
{
	int rc=do_couriertls_start(args, cinfo);
	int l;
	char *p;

	if (rc && cinfo->errmsg[0] == 0)
		strcpy(cinfo->errmsg, "Failed to initialize TLS/SSL\n");

	l=strlen(cinfo->errmsg);

	while (l > 0 && cinfo->errmsg[l-1] == '\n')
		--l;
	cinfo->errmsg[l]=0;

	if (rc || cinfo->x509info == 0)
		return (rc);

	cinfo->x509info[cinfo->x509info_len]=0;
	p=strtok(cinfo->x509info, "\r\n");

	while (p)
	{
		int i;

		for (i=0; p[i]; i++)
			if (!isalpha(p[i]))
				break;

		if (p[i] != ':')
		{
			p=strtok(NULL, "\r\n");
			continue;
		}
		p[i++]=0;

		/*
		** IMPORTANT: UCase *MUST* match the output of couriertls.
		** I'd love to use strcasecmp, here, but certain glibc
		** locale break the standard case of lower ascii chset
		** range.
		*/

		if (strcmp(p, "Subject") == 0)
		{
			struct tls_subject *subj, *subj2;
			struct tls_subjitem **itemptr;

			p += i;

			for (subj=cinfo->first_subject; subj && subj->next;
			     subj=subj->next)
				;

			subj2=(struct tls_subject *)
				malloc(sizeof(struct tls_subject));
			if (!subj2)
			{
				SYSERRMSG;
				return (-1);
			}

			if (subj)
				subj->next=subj2;
			else
				cinfo->first_subject=subj2;

			subj2->next=0;
			subj2->firstitem=0;
			itemptr= &subj2->firstitem;

			while ( p && (*p == 0
				      || isspace((int)(unsigned char)*p)))
			{
				while (*p && isspace((int)(unsigned char)*p))
					++p;
				for (i=0; p[i]; i++)
					if (!isalpha((int)(unsigned char)p[i]))
						break;
				if (p[i] != '=')
				{
					p=strtok(NULL, "\r\n");
					continue;
				}
				p[i++]=0;

				*itemptr= (struct tls_subjitem *)
					malloc(sizeof (struct tls_subjitem));

				if (!*itemptr)
				{
					SYSERRMSG;
					return (-1);
				}

				(*itemptr)->name=p;
				(*itemptr)->value=p+i;
				(*itemptr)->nextitem=0;

				itemptr= &(*itemptr)->nextitem;
				p=strtok(NULL, "\r\n");
			}
			continue;
		}

		if (strcmp(p, "Cipher") == 0)
		{
			p += i;
			while (*p && isspace((int)(unsigned char)*p))
				++p;
			cinfo->cipher=p;
		}
		else if (strcmp(p, "Version") == 0)
		{
			p += i;
			while (*p && isspace((int)(unsigned char)*p))
				++p;
			cinfo->version=p;
		}
		else if (strcmp(p, "Bits") == 0)
		{
			p += i;
			while (*p && isspace((int)(unsigned char)*p))
				++p;
			cinfo->bits=atoi(p);
		}
		p=strtok(NULL, "\r\n");
	}

	return (0);
}

const char *couriertls_get_subject(struct couriertls_info *cinfo,
				   const char *subject)
{
	struct tls_subject *subj;
	struct tls_subjitem *item, *p;

	if ((subj=cinfo->first_subject) == 0)
		return NULL;

	p=NULL;

	for (item=subj->firstitem; item; item=item->nextitem)
	{
		const char *a=item->name;
		const char *b=subject;

		while (*a && *b)
		{
			int ca= *a++;
			int cb= *b++;

			/* Locale muddies things up, do this by hand */

			if (ca >= 'a' && ca <= 'z')
				ca -= 'a' - 'A';

			if (cb >= 'a' && cb <= 'z')
				cb -= 'a' - 'A';

			if (ca != cb)
				break;
		}

		if (!*a && !*b)
			p=item;
		/*
		** We want the last one, to match the behavior when couriertls
		** passes this stuff via the environment.
		*/
	}

	if (p)
		return p->value;
	return (0);
}

void couriertls_export_subject_environment(struct couriertls_info *cinfo)
{
	struct tls_subject *subj;
	struct tls_subjitem *item;

	if ((subj=cinfo->first_subject) == 0)
		return;

	for (item=subj->firstitem; item; item=item->nextitem)
	{
		char *a=malloc(strlen(item->name)+20);
		const char *b=item->value;
		char *p;

		if (!a) continue;

		strcat(strcpy(a, "TLS_SUBJECT_"), item->name);

		for (p=a; *p; p++)
			if (*p >= 'a' && *p <= 'z')
				*p -= 'a' - 'A';

		setenv(a, b, 1);
		free(a);
	}
}

static int do_couriertls_start(char **args, struct couriertls_info *cinfo)
{
	pid_t p, p2;
	int waitstat;
	char **argvec;
	int nargs;
	char readbuf[BUFSIZ];
	fd_set fdr;
	int statuspipe_fd[2];
	int x509_fd[2];


	/* Create the pipes, and run couriertls */

	for (nargs=0; args[nargs]; nargs++)
		;

	argvec=malloc(sizeof(char *)*(nargs+10));
	if (!argvec)
	{
		SYSERRMSG;
		return (-1);
	}

	if (pipe(statuspipe_fd) < 0)
	{
		free(argvec);
		SYSERRMSG;
		return (-1);
	}

	if (pipe(x509_fd) < 0)
	{
		close(statuspipe_fd[0]);
		close(statuspipe_fd[1]);
		free(argvec);
		SYSERRMSG;
		return (-1);
	}

	if ((p=fork()) < 0)
	{
		close(x509_fd[0]);
		close(x509_fd[1]);
		close(statuspipe_fd[0]);
		close(statuspipe_fd[1]);
		free(argvec);
		SYSERRMSG;
		return (-1);
	}

	/* Child process starts another child process, which runs couriertls */

	if (p == 0)
	{
		static const char msg[]="500 Unable to start couriertls - insufficient resources.\n";

		FILE *fp;
		char miscbuf[NUMBUFSIZE];
		char statusfd_buf[NUMBUFSIZE+40];
		char x509fd_buf[NUMBUFSIZE+40];
		const char *s;

		close(statuspipe_fd[0]);
		close(x509_fd[0]);

		fp=fdopen(statuspipe_fd[1], "w");

		if (!fp)
		{
			if (write(statuspipe_fd[1], msg, sizeof(msg)-1) < 0)
				; /* Ignore */
			exit(0);
		}

		if ((p=fork()) != 0)
		{
			if (p < 0)
			{
				fprintf(fp,
					"500 Unable to start couriertls: %s\n",
					strerror(errno));
				fflush(fp);
			}
			exit(0);
		}

		argvec[0]="couriertls";
		argvec[1]=strcat(strcpy(statusfd_buf, "-statusfd="),
				 libmail_str_size_t(statuspipe_fd[1], miscbuf));
		argvec[2]=strcat(strcpy(x509fd_buf, "-printx509="),
				 libmail_str_size_t(x509_fd[1], miscbuf));
		for (nargs=0; (argvec[nargs+3]=args[nargs]) != 0; nargs++)
			;

		s=getenv("COURIERTLS");
		if (!s || !*s)
			s="couriertls";

		execv(s, argvec);
		fprintf(fp, "500 Unable to start couriertls: %s\n",
			strerror(errno));
		fflush(fp);
		exit(0);
	}

	/* The parent wait for the first child to exit */

	close(statuspipe_fd[1]);
	close(x509_fd[1]);

	while ((p2=wait(&waitstat)) != p)
		if (p2 < 0 && errno == ECHILD)
			break;

	if (p2 != p || !WIFEXITED(waitstat) || WEXITSTATUS(waitstat))
	{
		close(statuspipe_fd[0]);
		close(x509_fd[0]);
		ERRMSG("500 Error starting couriertls.");
		return (-1);
	}

	/* Now, we need to read from two pipes simultaneously, and save the
	** results.
	*/

	while (statuspipe_fd[0] >= 0 || x509_fd[0] >= 0)
	{
		FD_ZERO(&fdr);
		if (statuspipe_fd[0] >= 0)
			FD_SET(statuspipe_fd[0], &fdr);
		if (x509_fd[0] >= 0)
			FD_SET(x509_fd[0], &fdr);
		if (select( (statuspipe_fd[0] > x509_fd[0] ?
			     statuspipe_fd[0]:x509_fd[0])+1,
			    &fdr, NULL, NULL, NULL) < 0)
		{
			close(statuspipe_fd[0]);
			close(x509_fd[0]);
			SYSERRMSG;
			return (-1);
		}

		if (statuspipe_fd[0] >= 0 && FD_ISSET(statuspipe_fd[0], &fdr))
		{
			int n=read(statuspipe_fd[0], readbuf,
				   sizeof(readbuf)-1);

			if (n <= 0)
			{
				close(statuspipe_fd[0]);
				statuspipe_fd[0]= -1;
			}
			else
			{
				int l=strlen(cinfo->errmsg);

				readbuf[n]=0;
				if (l < sizeof(cinfo->errmsg)-2)
					strncat(cinfo->errmsg, readbuf,
						sizeof(cinfo->errmsg)-2-l);
			}
		}

		if (x509_fd[0] >= 0 && FD_ISSET(x509_fd[0], &fdr))
		{
			int n=read(x509_fd[0], readbuf, sizeof(readbuf));

			if (n <= 0)
			{
				close(x509_fd[0]);
				x509_fd[0]= -1;
			}
			else
			{
				if (n + cinfo->x509info_len >=
				    cinfo->x509info_size)
				{
					size_t news=n+cinfo->x509info_len
						+ 1024;
					char *newp= cinfo->x509info ?
						realloc(cinfo->x509info, news)
						: malloc(news);

					if (!newp)
					{
						SYSERRMSG;
						close(x509_fd[0]);
						x509_fd[0]= -1;
						continue;
					}
					cinfo->x509info=newp;
					cinfo->x509info_size=news;
				}

				memcpy(cinfo->x509info + cinfo->x509info_len,
				       readbuf, n);
				cinfo->x509info_len += n;
			}
		}

	}
	return (cinfo->errmsg[0] ? -1:0);
}

void couriertls_destroy(struct couriertls_info *info)
{
	struct tls_subject *subj;
	struct tls_subjitem *subjitem;

	if (info->x509info)
		free(info->x509info);

	while ((subj=info->first_subject) != 0)
	{
		info->first_subject=subj->next;
		while ((subjitem=subj->firstitem) != 0)
		{
			subj->firstitem=subjitem->nextitem;
			free(subjitem);
		}
		free(subj);
	}
}

#if 0
int main(int argc, char **argv)
{
	struct couriertls_info cinfo;
	struct tls_subject *subj;
	struct tls_subjitem *subjitem;

	couriertls_init(&cinfo);

	if (couriertls_start(argv+1, &cinfo))
	{
		printf("ERROR: %s\n",
		       cinfo.errmsg[0] ? cinfo.errmsg:"unknown error");
		exit(0);
	}

	printf("version=%s, cipher=%s, bits=%d\n", cinfo.cipher,
	       cinfo.version, cinfo.bits);

	for (subj=cinfo.first_subject; subj; subj=subj->next)
	{
		printf("Subject: ");

		for (subjitem=subj->firstitem; subjitem;
		     subjitem=subjitem->nextitem)
		{
			printf("/%s=%s", subjitem->name,
			       subjitem->value);
		}
		printf("\n");
	}
	couriertls_destroy(&cinfo);
	sleep(300);
	exit(0);
}
#endif
