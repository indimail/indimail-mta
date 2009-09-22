/*
** Copyright 2000-2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"auth.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"authsasl.h"
#include	"authwait.h"
#include	"debug.h"
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<unistd.h>
#include	<errno.h>
#include	<sys/time.h>
#include	<sys/select.h>
#include	"numlib/numlib.h"
#include	"authchangepwdir.h"

static const char rcsid[]="$Id: authdaemon.c,v 1.12 2004/04/18 15:54:38 mrsam Exp $";

extern int authdaemondo(const char *authreq,
	int (*func)(struct authinfo *, void *), void *arg);

struct authdaemon_info {
	char *username;
	void (*callback_func)(struct authinfo *, void *);
	void *callback_arg;
} ;

extern void auth_daemon_enumerate( void(*cb_func)(const char *name,
						  uid_t uid,
						  gid_t gid,
						  const char *homedir,
						  const char *maildir,
						  void *void_arg),
				   void *void_arg);

static int callback(struct authinfo *a, void *p)
{
	struct authdaemon_info *ai=(struct authdaemon_info *)p;

        if ((ai->username=strdup(a->address)) == 0)
        {
                perror("authdaemon: malloc");
                return (1);
        }

        {
	static  char *prevp=0;
	const char *cp=a->maildir;
	char    *p;

		if (!cp)        cp="";
		p=malloc(sizeof("MAILDIR=")+strlen(cp));
		if (!p)
		{
			perror("authdaemon: malloc");
			free(ai->username);
			return (1);
		}
		strcat(strcpy(p, "MAILDIR="), cp);
		putenv(p);
		if (prevp)      free(prevp);
		prevp=p;
	}

        {
		static  char *prevp=0;
		const char *cp=a->options;
		char    *p;

		if (!cp)        cp="";
		p=malloc(sizeof("OPTIONS=")+strlen(cp));
		if (!p)
		{
			perror("authdaemon: malloc");
			free(ai->username);
			return (1);
		}
		strcat(strcpy(p, "OPTIONS="), cp);
		putenv(p);
		if (prevp)      free(prevp);
		prevp=p;
	}

	if (a->sysusername && a->sysuserid)
		a->sysusername=0;

	if (ai->callback_func == 0)
		authsuccess(a->homedir, a->sysusername, a->sysuserid,
			    &a->sysgroupid, a->address, a->fullname);
	else
	{
		if (!a->address)
			a->address=a->sysusername;
		(*ai->callback_func)(a, ai->callback_arg);
	}
	return (0);
}

char *auth_daemon(const char *service, const char *authtype, char *authdata,
        int issession,
	void (*callback_func)(struct authinfo *, void *), void *callback_arg)

{
	char	tbuf[NUMBUFSIZE];
	size_t	l=strlen(service)+strlen(authtype)+strlen(authdata)+1;
	char	*n=libmail_str_size_t(l, tbuf);
	char	*buf=malloc(strlen(n)+l+20);
	int	rc;

	struct authdaemon_info ai;

	if (!buf)
	{
		perror("authdaemon: malloc");
		errno=EACCES;
		return (0);
	}
	strcat(strcat(strcpy(buf, "AUTH "), n), "\n");
	strcat(strcat(buf, service), "\n");
	strcat(strcat(buf, authtype), "\n");
	strcat(buf, authdata);

	ai.username=NULL;
	ai.callback_func=callback_func;
	ai.callback_arg=callback_arg;

	rc=authdaemondo(buf, callback, &ai);
	free(buf);

	if (auth_debug_login_level)
	{
	struct timeval t;

		/* short delay to try and allow authdaemond's courierlogger
		   to finish writing; otherwise items can appear out of order */
		t.tv_sec = 0;
		t.tv_usec = 100000;
		select(0, 0, 0, 0, &t);
	}

        if (rc < 0)
        {
                errno=EPERM;
                return (0);
        }
        if (rc > 0)
        {
                errno=EACCES;
                return (0);
        }

	return (ai.username);
}

extern int auth_daemon_pre(const char *uid, const char *service,
        int (*callback)(struct authinfo *, void *),
		    void *arg);

extern void auth_daemon_cleanup();

#define PROG AUTHCHANGEPWDIR "/authdaemon.passwd"

static int auth_daemon_passwd(const char *service,
			      const char *uid,
			      const char *opwd,
			      const char *npwd)
{
	pid_t p, p2;
	int waitstat;
	int pipefd[2];
	FILE *fp;

	signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE, SIG_IGN);

        if (pipe(pipefd) < 0)
        {
                perror("CRIT: authdaemon: pipe() failed");
                errno=EINVAL;
                return (-1);
        }

        p=fork();

        if (p == 0)
        {
                char *argv[2];

		argv[0]=PROG;
		argv[1]=0;
		close(0);
		dup(pipefd[0]);
		close(pipefd[0]);
		close(pipefd[1]);
		execv(argv[0], argv);
                perror("CRIT: authdaemon: failed to execute " PROG);
		exit(1);
	}

	close(pipefd[0]);

	if ((fp=fdopen(pipefd[1], "w")) != 0)
	{
		fprintf(fp, "%s\t%s\t%s\t%s\n",
			service, uid, opwd, npwd);
		fclose(fp);
	}
	close(pipefd[1]);

        while ((p2=wait(&waitstat)) != p)
        {
                if (p2 < 0 && errno == ECHILD)
                {
                        perror("CRIT: authdaemon: wait() failed");
                        errno=EPERM;
                        return (-1);
                }
        }

        if (WIFEXITED(waitstat) && WEXITSTATUS(waitstat) == 0)
                return (0);

	errno=EPERM;
	return (-1);
}

struct authstaticinfo authdaemon_info={
	"authdaemon",
	auth_daemon,
	auth_daemon_pre,
	auth_daemon_cleanup,
	auth_daemon_passwd,
	NULL,
	auth_daemon_enumerate};
