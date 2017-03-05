/*
** Copyright 1998 - 2006 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#include	"config.h"
#include	"maildircache.h"
#include	"numlib/numlib.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<signal.h>
#include	<pwd.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if	HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
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

#define exit(_a_) _exit(_a_)


static const char * const *authvars;
static char **authvals;
static time_t expinterval;
static time_t lastclean=0;
static const char *cachedir;
static const char *cacheowner;

int maildir_cache_init(time_t n, const char *d, const char *o,
			const char * const *a)
{
	unsigned x;

	expinterval=n;
	cachedir=d;
	cacheowner=o;
	authvars=a;

	for (x=0; a[x]; x++)
		;

	if ((authvals=malloc(sizeof(char *)*(x+1))) == NULL)
		return (-1);

	for (x=0; a[x]; x++)
		authvals[x]=0;
	return (0);
}

static char *create_cache_name(const char *userid, time_t login_time)
{
int	l;
char	buf[NUMBUFSIZE];
const char	*p;
char	*q;
char	*f, *g;

	login_time /= expinterval;
	l=1;
	for (p=userid; *p; p++)
	{
		++l;
		if (*p < ' ' || *p == ';' || *p == '\'' || *p == ';')
		{
			fprintf(stderr, "CRIT: maildircache: invalid chars in userid: %s\n", p);
			return (NULL);
		}
		if (*p == '/' || *p == '+' || (int)(unsigned char)*p >= 127)
			l += 2;
	}
	g=malloc(l);
	if (!g)
	{
		perror("CRIT: maildircache: malloc failed");
		return (NULL);
	}
	q=g;
	while (*userid)
	{
		if (*userid == '/' || *userid == '+'
			|| (int)(unsigned char)*userid >= 127)
		{
		static char xdigit[]="0123456789ABCDEF";

			*q++ = '+';
			*q++ = xdigit[ (*userid >> 4) & 15 ];
			*q++ = xdigit[ (*userid) & 15 ];
		}
		else
			*q++ = *userid;

		++userid;
	}
	*q=0;

	l=sizeof("//xx/xxxxxxx") + strlen(cachedir);
	l += strlen(libmail_str_time_t( login_time, buf)) + strlen(g);
	f=malloc(l);
	if (!f)
	{
		free(g);
		perror("CRIT: maildircache: malloc failed");
		return (NULL);
	}
	strcat(strcat(strcat(strcpy(f, cachedir), "/"), buf), "/");
	strncpy(buf, g, 2);
	buf[2]=0;
	while (strlen(buf) < 2)	strcat(buf, "+");
	strcat(strcat(strcat(f, buf), "/"), g);
	free(g);
	return (f);
}

static pid_t childproc= -1;
static int childpipe;

void maildir_cache_start()
{
int	pipefd[2];
char	buf[2048];
int	i, j;
char	*userid, *login_time, *data;
time_t	login_time_n;
char	*f;
FILE	*fp;

	if (pipe(pipefd) < 0)
	{
		perror("CRIT: maildircache: pipe() failed");
		return;
	}
	while ((childproc=fork()) < 0)
	{
		sleep(5);
	}

	if (childproc)
	{
		close(pipefd[0]);
		childpipe=pipefd[1];
		return;
	}
	close(pipefd[1]);
	i=0;

	for (;;)
	{
		if (i >= sizeof(buf)-1)
		{
			close(pipefd[0]);

			/* Problems */

			fprintf(stderr, "CRIT: maildircache: Max cache buffer overflow.\n");
			exit(1);
		}

		j=read(pipefd[0], buf+i, sizeof(buf)-1-i);
		if (j < 0)
		{
			perror("CRIT: maildircache: Cache create failure");
			exit(1);
		}
		if (j == 0)	break;
		i += j;
	}
	close(pipefd[0]);
	buf[i]=0;

	{
	struct passwd *pwd=getpwnam(cacheowner);

		if (!pwd || setgid(pwd->pw_gid) || setuid(pwd->pw_uid))
		{
			fprintf(stderr, "CRIT: maildircache: Cache create failure - cannot change to user %s\n",
			       cacheowner);
			exit(1);
		}
	}

	if (strncmp(buf, "CANCELLED\n", 10) == 0)
		exit (0);

	userid=buf;
	if ((login_time=strchr(userid, ' ')) == 0)
	{
		fprintf(stderr, "CRIT: maildircache: Cache create failure - authentication process crashed.\n");
		exit(1);
	}
	*login_time++=0;
	if ((data=strchr(login_time, ' ')) == 0)
	{
		fprintf(stderr, "CRIT: maildircache: Cache create failure - authentication process crashed.\n");
		exit(1);
	}
	*data++=0;

	login_time_n=0;
	while (*login_time >= '0' && *login_time <= '9')
		login_time_n = login_time_n * 10 + (*login_time++ -'0');

	f=create_cache_name(userid, login_time_n);

	if (!f)
		exit(0);

	if ((fp=fopen(f, "w")) == 0)	/* Try creating subdirs */
	{
		char	*p=f+strlen(cachedir);

		while (p && *p == '/')
		{
			*p=0;
			mkdir(f, 0700);
			*p='/';
			p=strchr(p+1, '/');
		}

		if ((fp=fopen(f, "w")) == 0)
		{
			fprintf(stderr, "CRIT: maildircache: Cache create failure - unable to create file %s.\n", f);
			exit(1);
		}
	}

	if ( fwrite(data, strlen(data), 1, fp) != 1 || fflush(fp)
	     || ferror(fp))
	{
		fclose(fp);
		unlink(f);	/* Problems */
		free(f);
		fprintf(stderr, "CRIT: maildircache: Cache create failure - write error.\n");
		exit(1);
	}
	else	fclose(fp);
	free(f);
	exit(0);
}

static int savebuf(char *p, int l)
{
	while (l)
	{
	int	n=write(childpipe, p, l);

		if (n <= 0)	return (-1);
		p += n;
		l -= n;
	}
	return (0);
}

void maildir_cache_save(const char *a, time_t b, const char *homedir,
		      uid_t u, gid_t g)
{
char	buf[2048];
char	buf2[NUMBUFSIZE];
pid_t	p;
int	waitstat;

	strcat(strcpy(buf, a), " ");
	strcat(strcat(buf, libmail_str_time_t(b, buf2)), " ");
	strcat(strcat(buf, libmail_str_uid_t(u, buf2)), " ");
	strcat(strcat(buf, libmail_str_gid_t(g, buf2)), " ");
	strncat(buf, homedir, sizeof(buf)-2-strlen(homedir));
	strcat(buf, "\n");

	if (savebuf(buf, strlen(buf)) == 0)
	{
	int	i;

		for (i=0; authvars[i]; i++)
		{
		const char *p;

			strcat(strcpy(buf, authvars[i]), "=");
			p=getenv(authvars[i]);
			if (!p || strlen(p)+strlen(buf) >= sizeof(buf)-2 ||
				strchr(p, '\n'))
				continue;
			strcat(strcat(buf, p), "\n");
			if (savebuf(buf, strlen(buf)))	break;
		}
	}
	close(childpipe);
	while ((p=wait(&waitstat)) != -1 && p != childproc)
		;
	childproc= -1;
}

void maildir_cache_cancel()
{
	if (childproc > 0)
	{
		if (write(childpipe, "CANCELLED\n", 10) < 0)
			perror("write");

		close(childpipe);
	}
}

int maildir_cache_search(const char *a, time_t b,
			 int (*callback_func)(uid_t, gid_t, const char *,
					      void *), void *callback_arg)
{
	char *f=create_cache_name(a, b);
	FILE *fp;
	uid_t	u;
	gid_t	g;
	char dir[1024];
	int	n;
	int	c;

	if (!f)
		return (-1);
	fp=fopen(f, "r");
	free(f);
	if (!fp)
		return (-1);

	u=0;
	while ((c=getc(fp)) != ' ')
	{
		if (c < '0' || c > '9')
		{
			fclose(fp);
			return (-1);
		}
		u=u*10 + (c-'0');
	}

	g=0;
	while ((c=getc(fp)) != ' ')
	{
		if (c < '0' || c > '9')
		{
			fclose(fp);
			return (-1);
		}
		g=g*10 + (c-'0');
	}

	for (n=0; (c=getc(fp)) != EOF; n++)
	{
		if (c == '\n')	break;
		if (n >= sizeof(dir)-1)
		{
			fclose(fp);
			fprintf(stderr, "CRIT: maildircache: Cache record overflow.\n");
			return (-1);
		}
		dir[n]=(char)c;
	}
	dir[n]=0;

	if ((n=(*callback_func)(u, g, dir, callback_arg)) != 0)
	{
		fclose(fp);
		return (n);
	}

	if (c != EOF)
	{
		while (fgets(dir, sizeof(dir), fp))
		{
		char	*q;

			if ( (q=strchr(dir, '\n')) == 0)
			{
				fclose(fp);
				fprintf(stderr, "CRIT: maildircache: Cache record overflow.\n");
				return (-1);
			}
			*q=0;

			for (n=0; authvars[n]; n++)
			{
				int l=strlen(authvars[n]);

				if (strncmp(dir, authvars[n], l) == 0 &&
				    dir[l] == '=')
				{
					char *s=strdup(dir);

					if (!s)
					{
						fclose(fp);
						perror("CRIT: maildircache: malloc failed");
						return (-1);
					}

					putenv(s);

					if (authvals[n])
						free(authvals[n]);
					authvals[n]=s;
					break;
				}
			}
		}
		fclose(fp);
	}
	return (0);
}

struct purge_list {
	struct purge_list *next;
	char *n;
} ;

static void add_purge_list(struct purge_list **p, const char *a)
{
	char *c=malloc(strlen(a) + 1);
	struct purge_list *pp;

	if (!c)
		return;

	pp=malloc(sizeof(struct purge_list));
	if (!pp)
	{
		free(c);
		return;
	}

	pp->next=*p;

	*p=pp;
	pp->n=c;
	strcpy(c, a);
}

static void rmrf(const char *);

void maildir_cache_purge()
{
	time_t now;
	pid_t p;
	int waitstat;
	struct passwd *pw;
	struct purge_list *pl;
	DIR *dirp;
	struct dirent *de;
	struct sigaction sa, oldsa;

	time(&now);

	if (lastclean && lastclean >= now - expinterval)
		return;

	lastclean=now;

	memset(&sa, 0, sizeof(sa));

	sa.sa_handler=SIG_DFL;
	if (sigaction(SIGCHLD, &sa, &oldsa) < 0)
	{
		perror("sigaction");
		return;
	}

	p=fork();

	if (p < 0)
		return;

	if (p)
	{
		pid_t p2;

		while ((p2=wait(&waitstat)) >= 0 && p2 != p)
			;

		sigaction(SIGCHLD, &oldsa, NULL);
		return;
	}

	p=fork();

	if (p)
		exit(0);

	pw=getpwnam(cacheowner);

	if (!pw)
	{
		fprintf(stderr, "CRIT: maildircache: no such user %s - cannot purge login cache dir\n",
		       cacheowner);
		exit(0);
	}

	if (setgid(pw->pw_gid) < 0 || setuid(pw->pw_uid) < 0)
	{
		fprintf(stderr, "CRIT: maildircache: cannot change to uid/gid for %s - cannot purge login cache dir\n",
		       cacheowner);
		exit(0);
	}

	if (chdir(cachedir))
	{
		fprintf(stderr, "CRIT: maildircache: cannot change dir to %s\n", cachedir);
		exit(0);
	}

	pl=NULL;

	dirp=opendir(".");

	now /= expinterval;
	--now;

	while (dirp && (de=readdir(dirp)) != NULL)
	{
		if (!isdigit((int)(unsigned char)de->d_name[0]))
			continue;

		if (atol(de->d_name) >= now)
			continue;

		add_purge_list(&pl, de->d_name);
	}
	if (dirp)
		closedir(dirp);

	while (pl)
	{
		struct purge_list *p=pl;

		pl=pl->next;

		rmrf(p->n);
		free(p->n);
		free(p);
	}
	exit(0);
}

static void rmrf(const char *d)
{
	DIR *dirp;
	struct dirent *de;
	struct purge_list *pl=NULL, *p;

	if (chdir(d))
		return;

	dirp=opendir(".");

	while (dirp && (de=readdir(dirp)) != NULL)
	{
		if (strcmp(de->d_name, ".") == 0 ||
		    strcmp(de->d_name, "..") == 0)
			continue;

		add_purge_list(&pl, de->d_name);
	}
	if (dirp)
		closedir(dirp);

	while (pl)
	{
		p=pl;

		pl=pl->next;

		if (unlink(p->n))
			rmrf(p->n);
		free(p->n);
		free(p);
	}

	if (chdir("..") < 0 || rmdir(d) < 0)
	{
		fprintf(stderr, "CRIT: maildircache: cannot chdir to .. while purging login cache\n");
		exit(1);
	}
}
