/*
 * authldap.c - 
 *
 * courier-imap - 
 * 
 * Copyright 1999 Luc Saillard <luc.saillard@alcove.fr>.
 *
 * This module use a server LDAP to authenticate user.
 * See the README.ldap
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA  02111-1307, USA.
 */

/*
 * Modified 28/11/2001 Iustin Pop <iusty@intensit.de>
 * There was a bug regarding the LDAP_TLS option: if both LDAP_TLS
 * and was LDAP_AUTHBIND were enabled, the ldap_start_tls function
 * was called only for the first connection, resulting in the fact
 * that the bind for checking the password was done without TLS,
 * sending the password in clear text over the network. Detected 
 * when using OpenLDAP with "security ssf=128" (which disalows any 
 * clear-text communication).
*/

/*
   Modified 01/21/2000 James Golovich <james@wwnet.net>

1. If LDAP_AUTHBIND is set in the config file, then the ldap server will
handle passwords via authenticated binds, instead of checking these
internally.
2. Changed paramaters for authldap_get to include pass.

*/

/*
   Modified 12/31/99 Sam Varshavchik:

1. read_env reads from a configuration file, instead of the environment
2. read_config appropriately modified.
3. If 'user' contains the @ character, domain from config is NOT appended.
4. added 'homeDir' attribute.  Use 'homeDir' instead of mailDir, and put
   mailDir into MAILDIR=
5. read_config renamed to authldap_read_config
6. get_user_info renamed to authldap_get
7. Added authldap_free_config, to clean up all the allocated memory
   (required for preauthldap).
8. Output LDAP attributes are defined in the configuration file as well.
9. Allow both plaintext and crypted passwords to be read from LDAP.
10. Added GLOB_UID GLOB_GID, as well as UID and GID params.

2/19/2000 Sam.

Rewrite to allow this code to be used in a long-running authentication daemon
(for Courier).  authldap_get renamed to authldapcommon, will initialize and
maintain a persistent connection.  Password checking moved entirely to
authldap.c.  authldapclose() will unbind and close the connection.

connection gets closed and reopened automatically after a protocol error.

error return from authldapcommon will indicate whether this is a transient,
or a permanent failure.

authldap_free_config removed - no longer required.

*/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_LBER_H
#include <lber.h>
#endif
#if HAVE_LDAP_H
#include <ldap.h>
#if LDAP_VENDOR_VERSION > 20000
#define OPENLDAPV2
#endif
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "authldap.h"
#include "auth.h"
#include "authldaprc.h"
#include "debug.h"

#ifndef DEBUG_LDAP
#define DEBUG_LDAP 0
#endif

#ifndef	LDAP_OPT_SUCCESS
#define LDAP_OPT_SUCCESS LDAP_SUCCESS
#endif

struct ldap_info
{
	const char *hostname;
	int   port;
	const char *binddn;
	const char *bindpw;
	const char *basedn;
	const char *mail;
        const char *filter;
	const char *domain;

	uid_t uid;
	gid_t gid;
	int   timeout;
	int   authbind;
	int   deref;
	int   protocol_version;
	int   tls;

	const char *mailroot;

	char **auxoptions;
	char **auxnames;
	const char **attrlist;

	/* Optional emailmap to handle */

	const char *emailmap;
	const char *emailmap_basedn;
	const char *emailmap_handle;
	const char *emailmap_handle_lookup;
};

/*
** There's a memory leak in OpenLDAP 1.2.11, presumably in earlier versions
** too.  See http://www.OpenLDAP.org/its/index.cgi?findid=864 for more
** information.  To work around the bug, the first time a connection fails
** we stop trying for 60 seconds.  After 60 seconds we kill the process,
** and let the parent process restart it.
**
** We'll control this behavior via LDAP_MEMORY_LEAK.  Set it to ZERO to turn
** off this behavior (whenever OpenLDAP gets fixed).
*/

static time_t ldapfailflag=0;

static void ldapconnfailure()
{
	const char *p=getenv("LDAP_MEMORY_LEAK");

	if (!p)
	{
#ifdef LDAP_VENDOR_NAME
#ifdef LDAP_VENDOR_VERSION
#define DO_OPENLDAP_CHECK
#endif
#endif

#ifdef DO_OPENLDAP_CHECK

		/* It's supposed to be fixed in 20019 */

		if (strcmp(LDAP_VENDOR_NAME, "OpenLDAP") == 0 &&
		    LDAP_VENDOR_VERSION < 20019)
			p="1";
		else
			p="0";
#else
		p="0";
#endif
	}

	if (atoi(p) && !ldapfailflag)
	{
		time(&ldapfailflag);
		ldapfailflag += 60;
	}
}

static int ldapconncheck()
{
	time_t t;

	if (!ldapfailflag)
		return (0);

	time(&t);

	if (t >= ldapfailflag)
		exit(0);
	return (1);
}

static int read_env(const char *env, const char **copy,
	const char *errstr, int needit, const char *value_default);
static void copy_value(LDAP *ld, LDAPMessage *entry, const char *attribut,
	char **copy, const char *username);

/*
 * Function: read_env
 * Copy the environnement $env and copy to $copy if not null
 * if needit is false, and env doesn't exist, copy $value_default to $copy
 * INPUT:
 *   $env: pointer to the environnement name
 *   $copy: where the value go
 *   $err: print a nice message when $env not_found and $needit is true
 *   $needit: if $needit is true and $value not found, return a error
 *   $value_default: the default value when $needit is false and $env doesn't exists
 * OUTPUT:
 *   boolean
 */
static int read_env(const char *env, const char **copy,
	const char *errstr, int needit, const char *value_default)
{
static char *ldapauth=0;
static size_t ldapauth_size=0;
size_t	i;
char	*p=0;
int	l=strlen(env);

	if (!ldapauth)
	{
	FILE	*f=fopen(AUTHLDAPRC, "r");
	struct	stat	buf;

		if (!f)	return (0);
		if (fstat(fileno(f), &buf) ||
			(ldapauth=malloc(buf.st_size+2)) == 0)
		{
			fclose(f);
			return (0);
		}
		if (fread(ldapauth, buf.st_size, 1, f) != 1)
		{
			free(ldapauth);
			ldapauth=0;
			fclose(f);
			return (0);
		}
		ldapauth[ldapauth_size=buf.st_size]=0;

		for (i=0; i<ldapauth_size; i++)
			if (ldapauth[i] == '\n')
				ldapauth[i]=0;
	}

	for (i=0; i<ldapauth_size; )
	{
		p=ldapauth+i;
		if (memcmp(p, env, l) == 0 &&
			isspace((int)(unsigned char)p[l]))
		{
			p += l;
			while (*p && *p != '\n' &&
				isspace((int)(unsigned char)*p))
				++p;
			break;
		}

		while (i < ldapauth_size)
			if (ldapauth[i++] == 0)	break;
	}

	if (i < ldapauth_size)
	{
		*copy= p;
		return (1);
	}

	if (needit)
	{
		err("%s", errstr);
		return 0;
	}

	*copy=0;
	if (value_default)
		*copy=value_default;

	return 1;
}

/*
 * Function: authldap_read_config
 *   Read Configuration from the environnement table
 * INPUT:
 *   $ldap: a structure where we place information
 * OUTPUT:
 *   boolean
 */
static int authldap_read_config(struct ldap_info *ldap)
{
	struct passwd *pwent;
	struct group  *grent;
	const char *p;
	size_t i, pass;

	for (i=0; ldap->auxoptions && ldap->auxoptions[i]; i++)
		free(ldap->auxoptions[i]);
	for (i=0; ldap->auxnames && ldap->auxnames[i]; i++)
		free(ldap->auxnames[i]);

	if (ldap->attrlist)
		free(ldap->attrlist);
	if (ldap->auxnames)
		free(ldap->auxnames);
	if (ldap->auxoptions)
		free(ldap->auxoptions);

	memset(ldap,0,sizeof(struct ldap_info));
	
	if (!read_env("LDAP_SERVER",&ldap->hostname,
		      "You need to specify a ldap server in config file",1,NULL))
		return 0;

	if (!read_env("LDAP_AUTHBIND", &p, "", 0, ""))
		return (0);

	if (p)
		sscanf(p,"%d",&ldap->authbind);

	ldap->port=LDAP_PORT;

	if (!read_env("LDAP_PORT", &p, "", 0, ""))
		return (0);

	if (p)
		sscanf(p,"%d",&ldap->port);

	if (!read_env("LDAP_BASEDN",&ldap->basedn,
		      "You need to specify a basedn in config file",1,NULL))
		return 0;
	if (!read_env("LDAP_BINDDN",&ldap->binddn,
		      "You need to specify a BINDDN in config file",0,NULL))
		return 0;
	if (!read_env("LDAP_BINDPW",&ldap->bindpw,
		      "You need to specify a password for the BINDDN in config file",0,NULL))
		return 0;
	if (!read_env("LDAP_MAIL",&ldap->mail,
		      "You need to specify a attribute for mail in config file",0,"mail"))
		return 0;
	if (!read_env("LDAP_DOMAIN",&ldap->domain,
		      "You need to specify a domain for mail in config file",0,""))
		return 0;

	p=0;
	ldap->uid=0;
	if (!read_env("LDAP_GLOB_UID", &p, "", 0, ""))
		return (0);

	if (p && *p)
	{
		unsigned long n;

		if (sscanf(p, "%lu", &n) == 1)
			ldap->uid=(uid_t)n;
		else
		{
			pwent=getpwnam(p);
			if (!pwent)
			{
				err("authldap: INVALID LDAP_GLOB_UID");
				return (0);
			}
			ldap->uid=pwent->pw_uid;
		}
	}

	ldap->gid=0;
	p=0;
	if (!read_env("LDAP_GLOB_GID", &p, "", 0, ""))
		return (0);

	if (p && *p)
	{
		unsigned long n;

		if (sscanf(p, "%lu", &n) == 1)
			ldap->gid=(gid_t)n;
		else
		{
			grent=getgrnam(p);
			if (!grent)
			{
				err("authldap: INVALID LDAP_GLOB_GID");
				return (0);
			}
			ldap->gid=grent->gr_gid;
		}
	}

	ldap->timeout=5;
	p=0;
	if (read_env("LDAP_TIMEOUT", &p, "", 0, "") && p)
	{
		sscanf(p,"%d",&ldap->timeout);
	}

	ldap->tls=0;
	p=0;
	if (read_env("LDAP_TLS", &p, "", 0, "") && p)
	{
		ldap->tls=atoi(p);
	}

	ldap->filter=0;
	p=0;
	if (read_env("LDAP_FILTER", &p, "", 0, "") && p && strlen (p))
	{
		ldap->filter=p;
	}

	ldap->deref = LDAP_DEREF_NEVER; 
	ldap->protocol_version = 0;	/* use API default */
	p=0;
	if (!read_env("LDAP_DEREF", &p, "", 0, ""))
		return (0);
	if (p)
	{
#ifndef LDAP_OPT_DEREF
		err("authldaplib: LDAP_OPT_DEREF not available, ignored");
#endif
		if (!strcasecmp (p, "never"))
			ldap->deref = LDAP_DEREF_NEVER;
		else if (!strcasecmp (p, "searching"))
			ldap->deref = LDAP_DEREF_SEARCHING;
		else if (!strcasecmp (p, "finding"))
			ldap->deref = LDAP_DEREF_FINDING;
		else if (!strcasecmp (p, "always"))
			ldap->deref = LDAP_DEREF_ALWAYS; 
	}

	if (!read_env("LDAP_PROTOCOL_VERSION", &p, 0, 0, 0))
		return (0);
	if (p)
	{
	int lpv = atoi(p);
#ifndef LDAP_OPT_PROTOCOL_VERSION
		err("authldaplib: LDAP_OPT_PROTOCOL_VERSION not available, ignored");
#endif
		if (lpv == 0
#ifdef LDAP_VERSION_MIN
			|| lpv < LDAP_VERSION_MIN
#endif
#ifdef LDAP_VERSION_MAX
			|| lpv > LDAP_VERSION_MAX
#endif
		)
			err("authldaplib: illegal protocol version ignored");
		else
			ldap->protocol_version = lpv;
	}

	if (!read_env("LDAP_MAILROOT",&ldap->mailroot,"",0,NULL)
	    || ldap->mailroot == NULL || ldap->mailroot[0] == 0)
		ldap->mailroot=NULL;

	if (!read_env("LDAP_EMAILMAP", &ldap->emailmap, "", 0, "") ||
	    !read_env("LDAP_EMAILMAP_BASEDN", &ldap->emailmap_basedn, "", 0, "") ||
	    !read_env("LDAP_EMAILMAP_ATTRIBUTE", &ldap->emailmap_handle, "", 0, "")||
	    !read_env("LDAP_EMAILMAP_MAIL",
		      &ldap->emailmap_handle_lookup, "", 0, ""))
		return (0);


	for (pass=0; pass<2; pass++)
	{
		if (pass)
		{
			if ((ldap->auxnames=malloc((i+1)*sizeof(char *)))
			    == NULL ||
			    (ldap->auxoptions=malloc((i+1)*sizeof(char *)))
			    == NULL)
			{
				perror("CRIT: authldap: malloc failed");
				if (ldap->auxnames)
					ldap->auxnames[0]=0;
				return 0;
			}
		}
		i=0;

		if (pass)
		{
			ldap->auxnames[0]=NULL;
			ldap->auxoptions[0]=NULL;
		}

		if (!read_env("LDAP_AUXOPTIONS", &p, "", 0, NULL)
		    || p == NULL || *p == 0)
			p=NULL;

		while (p && *p)
		{
			size_t n;

			if (*p == ',')
			{
				++p;
				continue;
			}

			for (n=0; p[n] && p[n] != ',' && p[n] != '='; n++)
				;

			if (pass)
			{
				if ((ldap->auxoptions[i]=malloc(n+1)) == NULL)
				{
					perror("CRIT: authldap: malloc failed");
					return 0;
				}

				memcpy(ldap->auxoptions[i], p, n);
				ldap->auxoptions[i][n]=0;
				ldap->auxoptions[i+1]=NULL;
			}

			p += n;

			if (*p == '=') ++p;

			for (n=0; p[n] && p[n] != ','; n++)
				;

			if (pass)
			{
				if (n == 0)
				{
					if ((ldap->auxnames[i]=
					     strdup(ldap->auxoptions[i]))
					    == NULL)
					{
						perror("CRIT: authldap: malloc failed");
						return 0;
					}
				}
				else if ((ldap->auxnames[i]=malloc(n+1)) == NULL)
				{
					perror("CRIT: authldap: malloc failed");
					return 0;
				}
				else
				{
					memcpy(ldap->auxnames[i], p, n);
					ldap->auxnames[i][n]=0;
					ldap->auxnames[i+1]=NULL;
				}
			}
			p += n;
			++i;
		}
	}

	if ((ldap->attrlist=malloc((i+20)*sizeof(const char *))) == NULL)
	{
		perror("CRIT: authldap: malloc failed");
		return 0;
	}

	return 1;
}

static char **get_values(LDAP *ld, LDAPMessage *entry, const char *attribut)
{
	char ** values;
	values=ldap_get_values(ld,entry, (char *)attribut);

	if (values==NULL)
	{
#ifdef HAVE_LDAP_RESULT2ERROR
		int ld_errno = ldap_result2error(ld,entry,0);
		if (ld_errno && ld_errno != LDAP_DECODING_ERROR)
			/* We didn't ask for this attribute */
			ldap_perror(ld,"ldap_get_values");
#else
		if (ld->ld_errno != LDAP_DECODING_ERROR)
			/* We didn't ask for this attribute */
			ldap_perror(ld,"ldap_get_values");
#endif
	}

	return values;
}



/*
 * Function: copy_value
 *   Copy value from a LDAP attribute to $copy
 * INPUT:
 *   $ld:       the connection with the LDAP server
 *   $entry:    the entry who contains attributes
 *   $attribut: this attribut
 *   $copy:     where data can go
 * OUTPUT:
 *   void
 */
static void copy_value(LDAP *ld, LDAPMessage *entry, const char *attribut,
	char **copy, const char *username)
{
  char ** values;
  values=ldap_get_values(ld,entry, (char *)attribut);

	if (values==NULL)
	{
#ifdef HAVE_LDAP_RESULT2ERROR
	  int ld_errno = ldap_result2error(ld,entry,0);
	  if (ld_errno && ld_errno != LDAP_DECODING_ERROR)
	    /* We didn't ask for this attribute */
	    ldap_perror(ld,"ldap_get_values");
#else
		if (ld->ld_errno != LDAP_DECODING_ERROR)
			/* We didn't ask for this attribute */
			ldap_perror(ld,"ldap_get_values");
#endif
		*copy=NULL;
		return;
	}
  /* We accept only attribute with one value */
	else if (ldap_count_values(values)>1)
	 {
		 *copy=strdup(values[0]);
		 fprintf(stderr, "WARN: authldaplib: duplicate attribute %s for %s\n",
			attribut,
			username);
		 *copy=NULL;
	 }
  /* We accept only attribute with one value */
	else if (ldap_count_values(values)!=1)
	 {
		 *copy=NULL;
	 }
  else
	 {
		 *copy=strdup(values[0]);
	 }
#if DEBUG_LDAP
  dprintf("copy_value %s: %s",attribut,values[0]);
#endif
  ldap_value_free(values);
}

static struct ldap_info my_ldap;
static LDAP *my_ldap_fp=0;

void authldapclose()
{
	if (my_ldap_fp)
	{
		ldap_unbind(my_ldap_fp);
		my_ldap_fp=0;
	}
}

static int ldaperror(int rc)
{
#ifdef OPENLDAPV2
	if (rc && !LDAP_NAME_ERROR(rc))
#else
	if (rc && !NAME_ERROR(rc))
#endif
	{
		/* If there was a protocol error, close the connection */
		authldapclose();
		ldapconnfailure();
	}
	return (rc);
}

/* This function takes a ldap connection and 
 * tries to enable TLS on it.
*/
static int enable_tls_on(LDAP *conn) {
#if HAVE_LDAP_TLS
	int version;
	int ldrc;

	if (ldaperror(ldrc=ldap_get_option (conn,
				    LDAP_OPT_PROTOCOL_VERSION,
				    &version))
	    != LDAP_SUCCESS)
	{
		const char *s=ldap_err2string(ldrc);

		err("ldap_get_option failed: %s", s);
		return (-1);
	}

	if (version < LDAP_VERSION3)
	{
		version = LDAP_VERSION3;
		(void)ldap_set_option (conn,
				       LDAP_OPT_PROTOCOL_VERSION,
				       &version);
	}

	if (ldaperror(ldrc=ldap_start_tls_s(conn, NULL, NULL))
	    != LDAP_SUCCESS)
	{
		const char *s=ldap_err2string(ldrc);

		err("ldap_start_tls_s failed: %s", s);
		return (-1);
	}
	return 0;
#else
	err("authldaplib: TLS not available");
	return (-1);
#endif
}

static LDAP *ldapconnect()
{
LDAP	*p;

#if DEBUG_LDAP
	dprintf("Hostname: %s:%d",my_ldap.hostname,my_ldap.port);
	dprintf("UID:      %d",my_ldap.uid);
	dprintf("GID:      %d",my_ldap.gid);
#endif

	if (ldapconncheck())
	{
		dprintf("authldaplib: timing out after failed connection");
		return (NULL);
	}

	p=ldap_init((char *)my_ldap.hostname,my_ldap.port);

	if (p==NULL)
	{
		err("cannot connect to LDAP server (%s:%d): %s",
			my_ldap.hostname, my_ldap.port, strerror(errno));
		ldapconnfailure();
	}
#if DEBUG_LDAP
	dprintf("ldapconnect end");
#endif
	return (p);
}

static int ldapopen()
{
int     ldrc;

	if (my_ldap_fp)	return (0);

	if (authldap_read_config(&my_ldap) == 0)
	{
		err("authldaplib: error in LDAP configuration file, aborting");
		return (1);
	}

	my_ldap_fp=ldapconnect();

	if (!my_ldap_fp)
	{
		return (1);
	}

#ifdef LDAP_OPT_PROTOCOL_VERSION

	/* Set protocol version if selected */
	if (my_ldap.protocol_version &&
		ldaperror(ldrc = ldap_set_option(my_ldap_fp, LDAP_OPT_PROTOCOL_VERSION,
		(void *) & my_ldap.protocol_version)) != LDAP_SUCCESS)
	  {
		const char *s=ldap_err2string(ldrc);

		err("ldap_set_option(PROTOCOL_VERSION %d) failed: %s",
			my_ldap.protocol_version, s);
		authldapclose();
		ldapconnfailure();
		return (-1);
	  }
	if (my_ldap.protocol_version)
		dprintf("selected ldap protocol version %d", my_ldap.protocol_version);
#endif

	if (my_ldap.tls && enable_tls_on(my_ldap_fp))
	{
		authldapclose();
		ldapconnfailure();
		return (-1);
	}

#ifdef LDAP_OPT_DEREF

	/* Set dereferencing mode */
	if (ldaperror(ldrc = ldap_set_option(my_ldap_fp, LDAP_OPT_DEREF,
					 (void *) & my_ldap.deref)) != LDAP_SUCCESS)
	  {
		const char *s=ldap_err2string(ldrc);

		err("ldap_set_option(DEREF) failed: %s", s);
		authldapclose();
		ldapconnfailure();
		return (-1);
	  }
#endif

  /* Bind to server */
  if (auth_debug_login_level >= 2)
	  dprintf("binding to LDAP server as DN '%s', password '%s'",
	  	my_ldap.binddn ? my_ldap.binddn : "<null>",
	  	my_ldap.bindpw ? my_ldap.bindpw : "<null>");
  else
	  dprintf("binding to LDAP server as DN '%s'",
	  	my_ldap.binddn ? my_ldap.binddn : "<null>");

  if (ldaperror(ldrc = ldap_simple_bind_s(my_ldap_fp,
		(char *)my_ldap.binddn,
		(char *)my_ldap.bindpw)) != LDAP_SUCCESS)
    {
    const char *s=ldap_err2string(ldrc);

	err("ldap_simple_bind_s failed: %s", s);
	authldapclose();
	ldapconnfailure();
	return (-1);
    }
	return (0);
}

static int auth_ldap_do(const char *, const char *, const char *,
			int (*)(struct authinfo *, void *),
                        void *arg, const char *);

int auth_ldap_changepw(const char *dummy, const char *user,
		       const char *pass,
		       const char *newpass)
{
	return auth_ldap_do("authlib", user, pass, NULL, NULL, newpass);
}

/*
 * Function: authldapcommon
 *   Get information from the LDAP server ($ldap) for this $user
 * INPUT:
 *   $user: the login name
 *   $pass: the login password (NULL if we don't want to check the pw)
 *   callback - callback function with filled in authentication info
 *   arg - extra argument for the callback function.
 * OUTPUT:
 *   < 0 - authentication failure
 *   > 0 - temporary failure
 *   else return code from the callback function.
 */

int authldapcommon(const char *service,
		   const char *user, const char *pass,
		   int (*callback)(struct authinfo *, void *),
		   void *arg)
{
	return (auth_ldap_do(service, user, pass, callback, arg, NULL));
}

static int auth_ldap_do2(const char *service,
			 const char *user, const char *pass,
			int (*callback)(struct authinfo *, void *),
			 void *arg, const char *newpass);

static char *escape_str(const char *);

static int auth_ldap_do(const char *service,
			const char *user, const char *pass,
			int (*callback)(struct authinfo *, void *),
                        void *arg, const char *newpass)
{
	char *q;
	int i;

	q=escape_str(user);

	if (!q)
		return (auth_ldap_do2(service,
				      user, pass, callback, arg, newpass));

	i=auth_ldap_do2(service, q, pass, callback, arg, newpass);
	free(q);
	return (i);
}


static int auth_ldap_do3(const char *service,
			 const char *attrname,
			 const char *user, const char *pass,
			 int (*callback)(struct authinfo *, void *),
			 void *arg, const char *newpass, const char *authaddr);

static char *emailmap_get_search_string(const char *str, const char *email);

static int auth_ldap_do2(const char *service,
			 const char *user, const char *pass,
			 int (*callback)(struct authinfo *, void *),
			 void *arg, const char *newpass)
{
	char *srch;
	struct timeval tv;
	const char *attributes[2];
	LDAPMessage *result, *entry;
	int cnt;
	char *v;
	const char *aname;

	if (ldapopen())	return (1);

	if (my_ldap.emailmap[0] == 0 || strchr(user, '@') == NULL)
		return (auth_ldap_do3(service, my_ldap.mail,
				      user, pass, callback, arg, newpass,
				      user));
	/* Mapping is not enabled */

	srch=emailmap_get_search_string(my_ldap.emailmap, user);

	if (!srch)
	{
		perror("CRIT: authldaplib: malloc");
		exit(1);
	}
	dprintf("using emailmap search: %s", srch);

	tv.tv_sec=my_ldap.timeout;
	tv.tv_usec=0;

	attributes[0]=my_ldap.emailmap_handle;

	if (!attributes[0][0])
		attributes[0]="handle";
	attributes[1]=NULL;

	if (ldaperror(ldap_search_st(my_ldap_fp,
				     (char *)(my_ldap.emailmap_basedn[0] ?
					      my_ldap.emailmap_basedn
					      :my_ldap.basedn),
				     LDAP_SCOPE_SUBTREE,
				     srch, (char **)attributes, 0,
				     &tv, &result)
		      != LDAP_SUCCESS))
	{
		free(srch);

		dprintf("ldap_search_st failed");
		if (my_ldap_fp)	return (-1);
		return (1);
	}

	if ((cnt=ldap_count_entries(my_ldap_fp, result)) != 1)
	{
		free(srch);

		if (cnt)
			err("emailmap: %d entries returned from search %s (but we need exactly 1)",
			       cnt, srch);
		ldap_msgfree(result);
		return -1;
	}
	free(srch);

	entry=ldap_first_entry(my_ldap_fp, result);

	if (!entry)
	{
		ldap_msgfree(result);

		err("authldap: unexpected NULL from ldap_first_entry");
		return -1;
	}

	copy_value(my_ldap_fp, entry, attributes[0], &v, user);

	if (!v)
	{
		dprintf("emailmap: empty attribute");
		ldap_msgfree(result);
		return (-1);
	}

	aname=my_ldap.emailmap_handle_lookup;
	if (aname[0] == 0)
		aname=my_ldap.mail;

	dprintf("emailmap results: aname=%s, handle=%s", aname, v);

	cnt=auth_ldap_do3(service,
			  aname, v, pass, callback, arg, newpass, user);

	ldap_msgfree(result);
	free(v);
	return (cnt);
}

static int auth_ldap_do3(const char *service,
			 const char *attrname,
			 const char *user, const char *pass,
			 int (*callback)(struct authinfo *, void *),
			 void *arg, const char *newpass,
			 const char *authaddr)
{
	char *newpass_crypt=0;
	const char *attributes[10];
	struct timeval timeout;

	LDAPMessage *result;
	LDAPMessage *entry;
	char *filter, *dn;
	int i, j;

	struct authinfo auth;
	char *homeDir=0;
	char *mailDir=0;
	char *userPassword=0;
	char *cryptPassword=0;
	char *options=0;
	char *cn=0;
	uid_t au;
	gid_t ag;
	int rc;
	char *quota=0;
        int additionalFilter = 0;
        int hasAdditionalFilter = 0;

        hasAdditionalFilter = my_ldap.filter != 0;

	memset(&auth, 0, sizeof(auth));

        if (hasAdditionalFilter)
        {
            /* To add the additional filter, we need to add on the
             * additional size for "(&)" and the other filter.  So
             * filter+3
             */
            additionalFilter = strlen(my_ldap.filter) + 3;
        }

	if ((filter=malloc(additionalFilter+strlen(attrname)+strlen(user)+
			   (my_ldap.domain ? strlen(my_ldap.domain):0)+
			   sizeof ("(=@)"))) == 0)
	{
		perror("malloc");
		return 1;
	}
        strcpy(filter, "\0");

        if (hasAdditionalFilter)
        {
            strcat(filter, "(&");
            strcat(filter, my_ldap.filter);
        }

        strcat(strcat(strcat(strcat(filter, "("), attrname), "="), user);
	if ( my_ldap.domain && my_ldap.domain[0] && strchr(user, '@') == 0 )
		strcat(strcat(filter, "@"), my_ldap.domain);
	strcat(filter, ")");
        
        if (hasAdditionalFilter)
        {
            strcat(filter, ")");
        }

	dprintf("using search filter: %s", filter);

	timeout.tv_sec=my_ldap.timeout;
	timeout.tv_usec=0;

	read_env("LDAP_HOMEDIR", &attributes[0], "", 0, "homeDir");
	read_env(service && strcmp(service, "courier") == 0
		 ? "LDAP_DEFAULTDELIVERY":"LDAP_MAILDIR",
		 &attributes[1], "", 0, 0);
	read_env("LDAP_FULLNAME", &attributes[2], "", 0, "cn");
	read_env("LDAP_CLEARPW", &attributes[3], "", 0, 0);
	read_env("LDAP_CRYPTPW", &attributes[4], "", 0, 0);
	read_env("LDAP_UID", &attributes[5], "", 0, 0);
	read_env("LDAP_GID", &attributes[6], "", 0, 0);
	attributes[7]=my_ldap.mail;
	read_env("LDAP_MAILDIRQUOTA", &attributes[8], "", 0, 0);

	j=0;
	for (i=0; i<9; i++)
	{
		if (attributes[i])
			my_ldap.attrlist[j++]=attributes[i];
	}

	for (i=0; my_ldap.auxoptions[i]; i++)
		my_ldap.attrlist[j++]=my_ldap.auxoptions[i];

	my_ldap.attrlist[j]=0;

	if (ldaperror(ldap_search_st(my_ldap_fp,
				     (char *)my_ldap.basedn,LDAP_SCOPE_SUBTREE,
				     filter, (char **)my_ldap.attrlist, 0,
				     &timeout, &result)
		      != LDAP_SUCCESS))
	{
		dprintf("ldap_search_st() failed");
		free(filter);

		if (my_ldap_fp)	return (-1);
		return (1);
	}

	free(filter);

	/* If we are more than one result, reject */
	if (ldap_count_entries(my_ldap_fp,result)!=1)
	{
		dprintf("number of entries returned: %d (but we need exactly 1)",
			ldap_count_entries(my_ldap_fp,result));
		ldap_msgfree(result);
		return -1;
	}

	dn = ldap_get_dn(my_ldap_fp, result);

	dprintf("one entry returned, DN: %s", dn ? dn : "<null>");

	if (dn == NULL) 
	{
		ldap_perror(my_ldap_fp, "ldap_get_dn");
		return -1;
	}

	/* Get the pointer on this result */
	entry=ldap_first_entry(my_ldap_fp,result);
	if (entry==NULL)
	{
		ldap_perror(my_ldap_fp,"ldap_first_entry");
		free(dn);
		return -1;
	}

#if DEBUG_LDAP
	dprintf("after ldap_first_entry");
#endif

	/* print all the raw attributes */
	if (auth_debug_login_level >= 2)
	{
	char *attr;
	BerElement *berptr = 0;

		attr = ldap_first_attribute(my_ldap_fp, entry, &berptr);
		if (attr) dprintf("raw ldap entry returned:");
		while (attr)
		{
		char **av, **ap;
		
			av = ldap_get_values(my_ldap_fp, entry, attr);
			ap = av;
			if (av)
			{
				while(*ap)
				{
					dprintf("| %s: %s", attr, *ap);
					ap++;
				}
				ldap_value_free(av);
			}
			ldap_memfree(attr);
			attr = ldap_next_attribute(my_ldap_fp, entry, berptr);
		}
		
		ber_free(berptr, 0);
	}

	/* Copy the directory and the password into struct */
	copy_value(my_ldap_fp,entry,attributes[0],&homeDir, user);
	if (attributes[1])
		copy_value(my_ldap_fp,entry,attributes[1],&mailDir, user);
	copy_value(my_ldap_fp,entry,attributes[2],&cn, user);
	if (attributes[3])
		copy_value(my_ldap_fp,entry,attributes[3],&userPassword, user);
	if (attributes[4])
		copy_value(my_ldap_fp,entry,attributes[4],&cryptPassword, user);

	au=my_ldap.uid;
	ag=my_ldap.gid;
	if (attributes[5])
	{
		char *p=0;
		unsigned long n;

		copy_value(my_ldap_fp, entry, attributes[5], &p, user);
		if (p) {
			if (sscanf(p, "%lu", &n) > 0)
				au= (uid_t)n;
			free(p);
		}
#if DEBUG_LDAP
		dprintf("au= %d",au);
#endif
	}

	if (attributes[6])
	{
		char *p=0;
		unsigned long n;

		copy_value(my_ldap_fp, entry, attributes[6], &p, user);
		if (p) {
			if (sscanf(p, "%lu", &n) > 0)
				ag= (gid_t)n;
			free(p);
		}
#if DEBUG_LDAP
		dprintf("ag= %d",ag);
#endif
	}

	if (attributes[8])
		copy_value(my_ldap_fp,entry,attributes[8],&quota, user);

	if (homeDir != 0 && my_ldap.mailroot != 0 && *my_ldap.mailroot)
	{
		char *new_mailroot=malloc(strlen(homeDir)+
					  strlen(my_ldap.mailroot)+2);

		if (!new_mailroot)
		{
			perror("CRIT: authldap: malloc failed");
			rc= -1;
		}
		else
		{
			strcat(strcat(strcpy(new_mailroot, my_ldap.mailroot),
				      "/"), homeDir);
			free(homeDir);
			homeDir=new_mailroot;
		}
	}

	j=1;

	for (i=0; my_ldap.auxoptions[i]; i++)
	{
		char *val;

		copy_value(my_ldap_fp, entry, my_ldap.auxoptions[i], &val,
			   user);

		if (!val)
			continue;

		j += 2 + strlen(my_ldap.auxnames[i]) +
			strlen(val);
		free(val);
	}

	options=malloc(j);

	if (!options)
	{
		perror("CRIT: authldap: malloc failed");
		rc= -1;
	}
	else
	{
		*options=0;

		for (i=0; my_ldap.auxoptions[i]; i++)
		{
			char *val;

			copy_value(my_ldap_fp, entry, my_ldap.auxoptions[i],
				   &val,
				   user);

			if (!val)
				continue;

			if (*options)
				strcat(options, ",");
			strcat(options, my_ldap.auxnames[i]);
			strcat(options, "=");
			strcat(options, val);
			free(val);
		}
	}


	auth.sysusername=user;
	auth.sysuserid= &au;
	auth.sysgroupid= ag;
	auth.homedir=homeDir;
	auth.address=authaddr;
	auth.fullname=cn;
	auth.maildir=mailDir;
	auth.clearpasswd=userPassword;
	auth.passwd=cryptPassword;
	auth.quota=quota;
	auth.options=options && *options ? options:NULL;

	if (auth.sysusername == 0)
		auth.sysusername=auth.address="";

	if (homeDir == 0)
		auth.homedir="";

	rc=0;

	if (au == 0 || ag == 0)
	{
		err("authldaplib: refuse to authenticate %s: uid=%d, gid=%d (zero uid or gid not permitted)",
		       user, au, ag);
		rc= 1;
	}

	auth_debug_authinfo("DEBUG: authldaplib: ", &auth,
		userPassword, cryptPassword);

	if (pass)
	{
		if (my_ldap.authbind) 
		{
			/* FIXME: if LDAP_V3 is available we should use a persistent connection for this */
			LDAP *bindp=ldapconnect();

			if (!bindp)
			{
				dprintf("ldapconnect failed");
				rc=1;
			}
			else
			{
#ifdef LDAP_OPT_PROTOCOL_VERSION
				/* Set protocol version */
				if (my_ldap.protocol_version &&
					ldap_set_option(bindp, LDAP_OPT_PROTOCOL_VERSION,
					(void *) & my_ldap.protocol_version) != LDAP_OPT_SUCCESS)
				  {
					err("ldap_set_option(PROTOCOL_VERSION %d) failed",
						my_ldap.protocol_version);
					rc = 1;
				  }
				else
#endif

				if(my_ldap.tls && enable_tls_on(bindp)) {
					err("authldaplib: LDAP_TLS enabled but I'm unable to start tls, check your config");
					rc = 1;
				} else {
				int ldrc;
					dprintf("rebinding with DN '%s' to validate password", dn);
					ldrc = ldap_simple_bind_s(bindp, dn, pass);
					switch (ldrc)
					{
					case LDAP_SUCCESS:
						dprintf("authentication bind successful");
						break;
					case LDAP_INVALID_CREDENTIALS:
						dprintf("authentication bind failed, invalid credentials");
						rc = -1;
						break;
					default:
						dprintf("authentication bind failed, some other problem: %s",
							ldap_err2string(ldrc));
						rc = 1;
						break;
					}
				}
				ldap_unbind(bindp);
			}
			if (rc == 0 && newpass)
			{
				if ((newpass_crypt=authcryptpasswd(newpass,
								   NULL))
				    == 0)
					rc= -1;
			}
		}
		else
		{
			if (auth.clearpasswd)
			{
				if (strcmp(pass,auth.clearpasswd))
				{
					if (auth_debug_login_level >= 2)
						dprintf("supplied password '%s' does not match clearpasswd '%s'",
							pass, auth.clearpasswd);
					else
						dprintf("supplied password does not match clearpasswd");
					rc= -1;
				}
			}
			else
			{
			const char *p=auth.passwd;

				if (p && strncasecmp(p, "{crypt}", 7) == 0)
					p += 7; /* For authcheckpassword */

				if (!p)
				{
					dprintf("no password to compare against!");
					rc= -1;
				}
				else if (authcheckpassword(pass, p))
					rc= -1;
			}

			if (rc == 0 && newpass && auth.passwd)
			{
				if ((newpass_crypt=authcryptpasswd(newpass,
								   auth.passwd)
				     ) == 0)
					rc= -1;
			}
		}
        }

	if (rc == 0 && newpass)
	{
		LDAPMod *mods[3];
		int mod_index=0;

		LDAPMod mod_clear, mod_crypt;
		char *mod_clear_vals[2], *mod_crypt_vals[2];

		if (attributes[3])
		{
			mods[mod_index]= &mod_clear;
			mod_clear.mod_op=LDAP_MOD_REPLACE;
			mod_clear.mod_type=(char *)attributes[3];
			mod_clear.mod_values=mod_clear_vals;

			mod_clear_vals[0]=(char *)newpass;
			mod_clear_vals[1]=NULL;
			++mod_index;
		}

		if (attributes[4] && newpass_crypt)
		{
			mods[mod_index]= &mod_crypt;
			mod_crypt.mod_op=LDAP_MOD_REPLACE;
			mod_crypt.mod_type=(char *)attributes[4];
			mod_crypt.mod_values=mod_crypt_vals;

			mod_crypt_vals[0]=newpass_crypt;
			mod_crypt_vals[1]=NULL;
			++mod_index;
		}
		if (mod_index == 0)
			rc= -1;
		else
		{
			mods[mod_index]=0;

			if (ldap_modify_s(my_ldap_fp, dn, mods))
			{
				rc= -1;
			}
		}
	}

	if (newpass_crypt)
		free(newpass_crypt);
	free (dn);
#if DEBUG_LDAP
	dprintf("before callback rc=%d",rc);
#endif

	if (rc == 0 && callback)
		rc= (*callback)(&auth, arg);
#if DEBUG_LDAP
	dprintf("after callback rc=%d",rc);
#endif

	ldap_msgfree(result);
	if (options)	free(options);
	if (homeDir)	free(homeDir);
	if (mailDir)	free(mailDir);
	if (userPassword)	free(userPassword);
	if (cryptPassword)	free(cryptPassword);
	if (cn)		free(cn);
	if (quota)	free(quota);
	return (rc);
}

/*
** Escape a string with special LDAP characters.  Returns NULL if the original
** string does not have any special LDAP characters (so we don't allocate
** memory unless absolutely necessary).
*/

static char *escape_str(const char *user)
{
	int i;
	const char *p;
	char *q, *r;

	for (i=0, p=user; *p; p++)
		if (strchr("*()\\", *p))
			++i;

	if (i == 0)
		return NULL;	/* No need to escape anything */

	q=malloc(strlen(user)+i+1);

	if (!q)
	{
		perror("malloc");
		exit(1);
	}

	for (r=q, p=user; *p; p++)
	{
		if (strchr("*()\\", *p))
			*r++= '\\';
		*r++ = *p;
	}
	*r=0;
	return (q);
}

/**
 ** Create an emailmap search string.  I'm going to wrap this into an external
 ** variable, so I'll use generic coding here.
 */

struct varlist {
	const char *varname;
	int varname_len;
	const char *varvalue;
	int varvalue_len;
} ;

static char *var_expand(const char *, const struct varlist *);

static char *emailmap_get_search_string(const char *str, const char *email)
{
	struct varlist vl[3];
	const char *realmptr=strrchr(email, '@');/* Guaranteed nonNULL */

	static const char userid[]="user";
	static const char realm[]="realm";

	vl[0].varname=userid;
	vl[0].varname_len=sizeof(userid)-1;
	vl[0].varvalue=email;
	vl[0].varvalue_len=realmptr - email;
	vl[1].varname=realm;
	vl[1].varname_len=sizeof(realm)-1;
	vl[1].varvalue=realmptr+1;
	vl[1].varvalue_len=strlen(vl[1].varvalue);
	vl[2].varname=NULL;

	return (var_expand(str, vl));
}

static char *var_expand(const char *str, const struct varlist *vl)
{
	const char *p;
	int cnt;
	int pass;
	char *q, *r;

	cnt=0;
	q=NULL;

	/*
	** Pass 1 - count expanded string length, allocate buffer,
	** Pass 2 - generate the string.
	*/

	for (pass=0; pass<2; pass++)
	{
		if (pass)
		{
			if ((q=malloc(cnt)) == NULL)
				return (NULL);
		}
		r=q;
		cnt=1;

		p=str;

		while (*p)
		{
			if (*p == '@')
			{
				int j;

				for (j=0; vl[j].varname; j++)
				{
					if (memcmp(vl[j].varname, p+1,
						   vl[j].varname_len) == 0
					    && p[vl[j].varname_len+1] == '@')
						break;
				}

				if (vl[j].varname)
				{
					p += vl[j].varname_len+2;

					if (r)
					{
						memcpy(r, vl[j].varvalue,
						       vl[j].varvalue_len);
						r += vl[j].varvalue_len;
					}
					cnt += vl[j].varvalue_len;
					continue;
				}
			}

			if (r)
				*r++ = *p;
			++p;
			++cnt;
		}
		if (r)
			*r=0;
	}

	return (q);
}

void auth_ldap_enumerate( void(*cb_func)(const char *name,
					 uid_t uid,
					 gid_t gid,
					 const char *homedir,
					 const char *maildir,
					 void *void_arg),
			   void *void_arg)
{
	char *filter;
	const char *attributes[5];
	const char *ldap_attributes[6];

	LDAPMessage *result;
	LDAPMessage *entry;
	int n, i, j;
	struct timeval timeout;

	if (ldapopen())	return;

	if (my_ldap.filter)
	{
		filter=strdup(my_ldap.filter);
	}
	else
	{
		filter=malloc(strlen(my_ldap.mail)+4);
		if (filter)
			strcat(strcpy(filter, my_ldap.mail), "=*");

	}

	if (!filter)
	{
		perror("malloc");
		return;
	}

	read_env("LDAP_MAIL", &attributes[0], "", 0, "mail");
	read_env("LDAP_UID", &attributes[1], "", 0, 0);
	read_env("LDAP_GID", &attributes[2], "", 0, 0);
	read_env("LDAP_HOMEDIR", &attributes[3], "", 0, "homeDir");
	read_env("LDAP_MAILDIR", &attributes[4], "", 0, 0);

	j=0;
	for (i=0; i<5; i++)
	{
		if (attributes[i])
			ldap_attributes[j++]=attributes[i];
	}

	ldap_attributes[j]=0;

	timeout.tv_sec=my_ldap.timeout;
	timeout.tv_usec=0;

	if (ldaperror(ldap_search_st(my_ldap_fp,
				     (char *)my_ldap.basedn,LDAP_SCOPE_SUBTREE,
				     filter, (char **)ldap_attributes, 0,
				     &timeout, &result)
		      != LDAP_SUCCESS))
	{
		free(filter);
		return;
	}

	n=ldap_count_entries(my_ldap_fp,result);

	for (i=0; i<n; i++)
	{
		char **names;
		int n;

		entry=i ? ldap_next_entry(my_ldap_fp, result):
			ldap_first_entry(my_ldap_fp, result);

		if (entry == NULL)
			break;

		names=get_values(my_ldap_fp, entry, attributes[0]);
		n=ldap_count_values(names);
		for (i=0; i<n; i++)
		{
			const char *name=names[i];

			char *uid_s=NULL;
			char *gid_s=NULL;
			char *homedir;
			char *maildir;
			uid_t uid=my_ldap.uid;
			gid_t gid=my_ldap.gid;

			if (attributes[1])
			{
				copy_value(my_ldap_fp, entry, attributes[1],
					   &uid_s,
					   name);
			}

			if (attributes[2])
			{
				copy_value(my_ldap_fp, entry, attributes[1],
					   &gid_s, name);
			}

			copy_value(my_ldap_fp, entry, attributes[3],
				   &homedir, name);
			copy_value(my_ldap_fp, entry, attributes[4],
				   &maildir, name);

			if (uid_s)
				uid=atol(uid_s);
			if (gid_s)
				gid=atol(gid_s);

			if (name && homedir)
				(*cb_func)(name, uid, gid, homedir,
					   maildir, void_arg);


			if (uid_s)
				free(uid_s);
			if (gid_s)
				free(gid_s);
			if (homedir)
				free(homedir);
			if (maildir)
				free(maildir);
		}
		ldap_value_free(names);
	}

	ldap_msgfree(result);

	if (i == n)
		(*cb_func)(NULL, 0, 0, NULL, NULL, void_arg);
}
