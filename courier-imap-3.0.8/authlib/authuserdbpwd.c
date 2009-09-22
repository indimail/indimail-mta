/*
** Copyright 2001 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<signal.h>
#include	<pwd.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"userdb/userdb.h"
#include	"auth.h"
#include	"authmod.h"
#include	"authstaticlist.h"
#include	"authwait.h"
#include	"sbindir.h"
#if HAVE_HMACLIB
#include        "../libhmac/hmac.h"
#endif

static const char rcsid[]="$Id: authuserdbpwd.c,v 1.5 2001/08/30 12:13:36 mrsam Exp $";


static int bad(const char *q)
{
	const char *p;

	for (p=q; *p; p++)
		if ((int)(unsigned char)*p < ' ' || *p == '|' || *p == '='
		    || *p == '\'' || *p == '"')
			return (1);
	return (0);
}


#if HAVE_HMACLIB

static char *hmacpw(const char *pw, const char *hash)
{
	int     i;

	for (i=0; hmac_list[i] &&
		     strcmp(hmac_list[i]->hh_name, hash); i++)
                                ;
	if (hmac_list[i])
	{
		struct hmac_hashinfo    *hmac=hmac_list[i];
		unsigned char *p=malloc(hmac->hh_L*2);
		char *q=malloc(hmac->hh_L*4+1);
		unsigned i;

                if (!p || !q)
                {
                        perror("malloc");
                        exit(1);
                }

                hmac_hashkey(hmac, pw, strlen(pw), p, p+hmac->hh_L);
                for (i=0; i<hmac->hh_L*2; i++)
                        sprintf(q+i*2, "%02x", (int)p[i]);
		free(p);
		return (q);
	}
	return (NULL);
}
#endif

static int dochangepwd1(const char *, const char *, const char *, const char *,
			int);

int auth_userdb_passwd(const char *service,
		      const char *uid,
		      const char *opwd_buf,
		      const char *npwd_buf)
{
	char *opwd;
	char *npwd;
	int rc;
	int hmac_flag=0;

	if (bad(uid) ||
	    bad(service) ||
	    bad(opwd_buf) ||
	    bad(npwd_buf) ||
	    strchr(uid, '/'))
	{
		errno=EPERM;
		return (1);
	}

#if HAVE_HMACLIB

	if (strncmp(service, "hmac-", 5) == 0)
	{
		opwd=hmacpw(opwd_buf, service+5);
		npwd=hmacpw(npwd_buf, service+5);

		if (!opwd)
		{
			if (npwd)
				free(npwd);
			errno=EPERM;
			return (1);
		}
		hmac_flag=1;
	}
	else
#endif
	{
		opwd=strdup(opwd_buf);
		if (!opwd)
		{
			errno=EPERM;
			return (1);
		}

		npwd=userdb_mkmd5pw(npwd_buf);
		if (!npwd || !(npwd=strdup(npwd)))
		{
			free(opwd);
			errno=EPERM;
			return (1);
		}
	}


	rc=dochangepwd1(service, uid, opwd, npwd, hmac_flag);

	free(opwd);
	free(npwd);
	return (rc);
}

static int dochangepwd2(const char *service, const char *uid,
			char    *u,
			const struct  userdbs *udb, const char *npwd);

static int dochangepwd1(const char *service, const char *uid,
			const char *opwd, const char *npwd, int hmac_flag)
{
	char *udbs;
	char *services;
	char *passwords;
	int rc;

	char    *u;
	struct  userdbs *udb;


	udbs=userdbshadow(USERDB "shadow.dat", uid);

	if (!udbs)
	{
		errno=EINVAL;
		return (1);
	}

	if ((services=malloc(strlen(service)+sizeof("pw"))) == 0)
	{
		perror("malloc");
		free(udbs);
		errno=EPERM;
		return (1);
	}

	strcat(strcpy(services, service), "pw");

	passwords=userdb_gets(udbs, services);
	free(services);

	if (passwords == 0)
	{
		passwords=userdb_gets(udbs, "systempw");
		service="system";
	}

	if (!passwords || (hmac_flag ? strcmp(opwd, passwords):
			   authcheckpassword(opwd, passwords)))
	{
		if (passwords)
			free(passwords);
		free(udbs);
		errno=EPERM;
		return (1);
	}
	free(passwords);
	free(udbs);

        userdb_init(USERDB ".dat");
        if ( (u=userdb(uid)) == 0 ||
	     (udb=userdb_creates(u)) == 0)
        {
		if (u)
			free(u);
		errno=EPERM;
		return (1);
        }

	rc=dochangepwd2(service, uid, u, udb, npwd);

	userdb_frees(udb);
	free(u);
	return (rc);
}

static int dochangepwd2(const char *services, const char *uid,
			char    *u,
			const struct  userdbs *udb, const char *npwd)
{
	char *envp[5];
	char *argv[10];
	pid_t p, p2;
	int waitstat;

	envp[0]="PATH=/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/usr/local/sbin";
	envp[1]="LC_ALL=C";
	envp[2]="SHELL=/bin/sh";
	envp[3]=0;

	argv[0]=SBINDIR "/userdb";
	argv[1]=malloc(strlen(udb->udb_source ? udb->udb_source:"")
		       +strlen(uid)+1);

	if (!argv[1])
	{
		errno=EPERM;
		return (1);
	}
	strcpy(argv[1],udb->udb_source ? udb->udb_source:"");
	strcat(argv[1],uid);
	argv[2]="set";

	argv[3]=malloc(strlen(services)+strlen(npwd)+10);
	if (!argv[3])
	{
		free(argv[1]);
		errno=EPERM;
		return (1);
	}

	sprintf(argv[3], "%spw=%s", services, npwd);
	signal(SIGCHLD, SIG_DFL);
	argv[4]=0;

	p=fork();

	if (p < 0)
	{
		free(argv[3]);
		free(argv[1]);
		errno=EPERM;
		return (1);
	}

	if (p == 0)
	{
		execve(argv[0], argv, envp);
		perror(argv[0]);
		exit(1);
	}

	free(argv[1]);
	free(argv[3]);

	while ((p2=wait(&waitstat)) != p)
	{
		if (p2 < 0 && errno == ECHILD)
		{
			perror("wait");
			errno=EPERM;
			return (1);
		}
	}

	if (!WIFEXITED(waitstat) || WEXITSTATUS(waitstat))
	{
		errno=EPERM;
		return (1);
	}

	p=fork();

	if (p < 0)
	{
		perror("fork");
		return (1);
	}

	if (p == 0)
	{
		argv[0]= SBINDIR "/makeuserdb";
		argv[1]=0;

		execve(argv[0], argv, envp);
		perror(argv[0]);
		exit(1);
	}

	while ((p2=wait(&waitstat)) != p)
	{
		if (p2 < 0 && errno == ECHILD)
		{
			errno=EPERM;
			return (1);
		}
	}

	if (!WIFEXITED(waitstat) || WEXITSTATUS(waitstat))
	{
		errno=EPERM;
		return (1);
	}
	return (0);
}
