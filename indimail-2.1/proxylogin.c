/*
 * $Log: proxylogin.c,v $
 * Revision 2.46  2017-03-13 14:06:35+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.45  2013-06-10 15:46:48+05:30  Cprogrammer
 * set MAILDIRQUOTA=0S if quota is NOQUOTA
 *
 * Revision 2.44  2011-10-28 14:16:10+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.43  2011-10-25 20:49:15+05:30  Cprogrammer
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.42  2011-04-01 14:14:53+05:30  Cprogrammer
 * added code to auto provision users
 *
 * Revision 2.41  2010-05-05 14:44:59+05:30  Cprogrammer
 * added setting of AUTHSERVICE environment variable
 *
 * Revision 2.40  2010-03-06 14:55:46+05:30  Cprogrammer
 * identify STLS or STARTTLS
 *
 * Revision 2.39  2009-10-14 20:45:17+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.38  2009-09-19 19:04:19+05:30  Cprogrammer
 * fix for mandriva 2009.1
 *
 * Revision 2.37  2009-02-24 14:06:46+05:30  Cprogrammer
 * BUG - fixed incorrect passwords get matched (incorrect usage of pw_comp)
 *
 * Revision 2.36  2009-02-18 21:36:45+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 2.35  2009-02-06 11:40:08+05:30  Cprogrammer
 * ignore return value of write
 *
 * Revision 2.34  2008-12-19 11:53:09+05:30  Cprogrammer
 * set home directory defined by TMP_MAILDIR
 *
 * Revision 2.33  2008-12-16 18:37:10+05:30  Cprogrammer
 * added MSG_ONERROR
 *
 * Revision 2.32  2008-11-21 11:59:54+05:30  Cprogrammer
 * added Login_Task logic as in authindi
 *
 * Revision 2.31  2008-09-11 23:01:49+05:30  Cprogrammer
 * use pw_comp for password comparision
 *
 * Revision 2.30  2008-08-29 14:03:00+05:30  Cprogrammer
 * compare password lenght using length of crypted passwd returned by crypt ()
 *
 * Revision 2.29  2008-08-24 17:42:25+05:30  Cprogrammer
 * added NO_IMAP, NO_POP3 checks
 * added code for MIN_LOGIN_INTERVAL
 *
 * Revision 2.28  2008-07-13 19:46:21+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.27  2008-06-24 21:51:05+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.26  2008-06-03 19:46:01+05:30  Cprogrammer
 * added legacy server option
 * added mailstore ip to logs
 *
 * Revision 2.25  2008-05-28 15:14:11+05:30  Cprogrammer
 * removed ldap module
 *
 * Revision 2.24  2006-02-08 09:43:23+05:30  Cprogrammer
 * added \r\n for imap LOGIN
 *
 * Revision 2.23  2005-12-29 22:48:53+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.22  2004-09-20 20:15:42+05:30  Cprogrammer
 * run executable defined by ADDUSERCMD instead of using internal
 * adminclient routine
 * set environment variable LDAP_ATTRIBUTES using data fetched
 * by inquery - LDAP_PASSWD
 *
 * Revision 2.21  2003-06-25 12:18:31+05:30  Cprogrammer
 * displaying the cause for authentication is not good
 *
 * Revision 2.20  2003-02-03 21:54:32+05:30  Cprogrammer
 * change for new argument input_read to adminCmmd()
 *
 * Revision 2.19  2002-12-26 04:34:37+05:30  Cprogrammer
 * corrected a potential problem with passwd comparision
 *
 * Revision 2.18  2002-12-12 01:26:39+05:30  Cprogrammer
 * added LOGIN, LOGOUT messages
 *
 * Revision 2.17  2002-12-09 20:31:28+05:30  Cprogrammer
 * OE Bug. Remove quotes from password
 *
 * Revision 2.16  2002-12-06 20:29:52+05:30  Cprogrammer
 * bug fix - char ':' not allowed in gecos
 *
 * Revision 2.15  2002-12-06 09:42:58+05:30  Cprogrammer
 * added env variable LDAP_ACCESS_CHECK
 *
 * Revision 2.14  2002-12-06 01:55:29+05:30  Cprogrammer
 * added ip address/service access control
 *
 * Revision 2.13  2002-12-02 01:48:27+05:30  Cprogrammer
 * avoid a possible bug with conversion to integer
 *
 * Revision 2.12  2002-11-30 21:58:52+05:30  Cprogrammer
 * added service in gecos
 *
 * Revision 2.11  2002-11-27 01:49:44+05:30  Cprogrammer
 * made errors more meaningful
 * added code for selective blocking imap/pop3
 *
 * Revision 2.10  2002-11-21 00:58:06+05:30  Cprogrammer
 * Added Check_Login() check
 *
 * Revision 2.9  2002-10-12 22:55:37+05:30  Cprogrammer
 * moved prototype definition outside #ifdef CLUSTERED_SITE
 *
 * Revision 2.8  2002-08-13 10:17:04+05:30  Cprogrammer
 * autoAddUser() called only on successful authentication
 * added specification of quota for vadduser
 *
 * Revision 2.7  2002-08-05 01:19:50+05:30  Cprogrammer
 * added remove_quotes for password
 *
 * Revision 2.6  2002-08-03 05:19:46+05:30  Cprogrammer
 * removed port from service when inserting into lastauth
 *
 * Revision 2.5  2002-08-03 01:40:44+05:30  Cprogrammer
 * allow quoting of passwd
 *
 * Revision 2.4  2002-08-03 00:37:45+05:30  Cprogrammer
 * remove quoted usernames
 * set IMAPLOGINTAG whenever exec() is called
 *
 * Revision 2.3  2002-07-24 11:55:24+05:30  Cprogrammer
 * use Email instead of user to avoid default domain getting used when the user has specified a domain
 *
 * Revision 2.2  2002-07-22 21:02:39+05:30  Cprogrammer
 * added code to auto add users if present in ldap
 *
 * Revision 2.1  2002-04-24 15:10:53+05:30  Cprogrammer
 * added pop3d_capability() and imapd_capability()
 *
 * Revision 1.1  2002-04-09 20:58:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#define _XOPEN_SOURCE
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: proxylogin.c,v 2.46 2017-03-13 14:06:35+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
static int      LocalLogin(char **, char *, char *, char *, char *, char *);
static int      ExecImapd(char **, char *, char *, struct passwd *, char *, char *);
static int      have_imap_starttls();
static int      have_pop3_starttls();
static int      tlsrequired();
int             auth_admin(char *, char *, char *, char *, char *);
int             adminCmmd(int, int, char *, int);

static char     imaptagenv[MAX_BUFF];

int
autoAddUser(char *email, char *pass, char *service)
{
	char           *admin_user, *admin_pass, *admin_host, *admin_port,
                   *hard_quota, *ptr, *cptr, *certfile;
	char            cmdbuf[MAX_BUFF], encrypted[MAX_BUFF], tmpbuf[5];
	int             len, sfd;

	if (!getenv("AUTOADDUSERS"))
		return (1);
	if (encrypt_flag)
		scopy(encrypted, pass, MAX_BUFF);
	else
		mkpasswd3(pass, encrypted, MAX_BUFF);
	getEnvConfigStr(&ptr, "ADDUSERCMD", PREFIX"/bin/autoadduser");
	if (!access(ptr, X_OK))
	{
		snprintf(cmdbuf, sizeof(cmdbuf), "%s %s %s", ptr, email, encrypted);
		return (runcmmd(cmdbuf, 0));
	}
	if (!(admin_user = (char *) getenv("ADMIN_USER")))
	{
		fprintf(stderr, "Admin User not specified\n");
		return (-1);
	} else
	if (!(admin_pass = (char *) getenv("ADMIN_PASS")))
	{
		fprintf(stderr, "Admin Pass not specified\n");
		return (-1);
	} else
	if (!(admin_host = (char *) getenv("ADMIN_HOST")))
	{
		fprintf(stderr, "Admin Host not specified\n");
		return (-1);
	} else
	if (!(admin_port = (char *) getenv("ADMIN_PORT")))
	{
		fprintf(stderr, "Admin Port not specified\n");
		return (-1);
	} else
	if (!(certfile = (char *) getenv("CERTFILE")))
	{
		fprintf(stderr, "Client Certificate not specified\n");
		return (-1);
	}
	if ((sfd = auth_admin(admin_user, admin_pass, admin_host, admin_port, certfile)) == -1)
	{
		perror("admin user");
		return (-1);
	}
	/*- Copy 4 Bytes (plus NULL) to tmpbuf */
	for (ptr = service, cptr = tmpbuf, len = 1;*ptr && *ptr != ':' && len < sizeof(tmpbuf);*cptr++ = *ptr++, len++);
	*cptr = 0;
	getEnvConfigStr(&hard_quota, "HARD_QUOTA", HARD_QUOTA);
	snprintf(cmdbuf, sizeof(cmdbuf), "0 vadduser -e -c auto.%s.%s -q %s %s %s",
		admin_port, tmpbuf, hard_quota, email, encrypted);
	return (adminCmmd(sfd, 0, cmdbuf, strlen(cmdbuf)));
}

int
proxylogin(char **argv, char *service, char *userid, char *plaintext, char *remoteip, char *imaptag, int skip_nl)
{
	char           *ptr, *cptr, *remote_port;
	char            loginbuf[MAX_BUFF], TheUser[MAX_BUFF], TheDomain[MAX_BUFF], Email[MAX_BUFF];
	char           *mailstore;
	int             retval;

	remove_quotes(userid);
	for(cptr = TheUser, ptr = userid;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for(cptr = TheDomain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!(remote_port = strchr(service, ':')))
	{
		fprintf(stdout, "Unknown Service [%s] - code %d\n", service, __LINE__);
		fprintf(stderr, "Unknown Service [%s] - code %d\n", service, __LINE__);
		fflush(stdout);
		return(1);
	} else
		remote_port++;
	if (strncmp(service, "imap", 4) && strncmp(service, "pop3", 4))
	{
		fprintf(stdout, "Unknown Service [%s:%s] - code %d\n", service, remote_port, __LINE__);
		fprintf(stderr, "Unknown Service [%s:%s] - code %d\n", service, remote_port, __LINE__);
		fflush(stdout);
		return(1);
	}
	snprintf(Email, MAX_BUFF, "%s@%s", TheUser, TheDomain);
	mailstore = (char *) 0;
	if (!(mailstore = inquery(HOST_QUERY, Email, 0)))
	{
		if (userNotFound)
		{
			switch ((retval = autoAddUser(Email, plaintext, service)))
			{
			case -1:
				printf("Temporary Service Problem provisioning user - code %d\n", __LINE__);
				fflush(stdout);
				fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
				return(1);
			case 0: /*- user successfully provisioned */
				break;
			case 1: /*- auth failure */
				if (!strncmp(service, "imap", 4))
				{
					snprintf(imaptagenv, MAX_BUFF, "IMAPLOGINTAG=%s", imaptag);
					putenv(imaptagenv);
				}
				execv(argv[0], argv);
				fprintf(stderr, "execv: %s: %s\n", argv[0], strerror(errno));
				return(1);
			}
		} else
		{
			printf("Temporary Service Problem - code %d\n", __LINE__);
			fflush(stdout);
			fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
			return(1);
		}
	}
	/*- user provisioned by autoAdduser */
	if (!mailstore && !(mailstore = inquery(HOST_QUERY, Email, 0)))
	{
		if (userNotFound)
		{
			if (!strncmp(service, "imap", 4))
			{
				snprintf(imaptagenv, MAX_BUFF, "IMAPLOGINTAG=%s", imaptag);
				putenv(imaptagenv);
			}
			execv(argv[0], argv);
			fprintf(stderr, "execv: %s: %s\n", argv[0], strerror(errno));
		} else
		{
			printf("Temporary Service Problem - code %d\n", __LINE__);
			fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
		}
		fflush(stdout);
		return(1);
	}
	if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
		*ptr = 0;
	for (;*mailstore && *mailstore != ':'; mailstore++);
	mailstore++;
	if (islocalif(mailstore)) /* - Prevent LoopBak */
		return(LocalLogin(argv, TheUser, TheDomain, service, imaptag, plaintext));
	else /*- if (islocalif(mailstore)) */
	{
		if (!strncmp(service, "imap", 4))
		{
			if (!(ptr = getenv("LEGACY_SERVER")))
				snprintf(loginbuf, MAX_BUFF, "%s LOGIN %s@%s:%s %s\r\n", 
					imaptag, TheUser, TheDomain, remoteip, plaintext);
			else
				snprintf(loginbuf, MAX_BUFF, "%s LOGIN %s@%s %s\r\n", 
					imaptag, TheUser, TheDomain, plaintext);
		} else
		if (!strncmp(service, "pop3", 4))
		{
			if (!(ptr = getenv("LEGACY_SERVER")))
				snprintf(loginbuf, MAX_BUFF, "USER %s@%s:%s\nPASS %s\n", 
					TheUser, TheDomain, remoteip, plaintext);
			else
				snprintf(loginbuf, MAX_BUFF, "USER %s@%s\nPASS %s\n", 
					TheUser, TheDomain, plaintext);
		}
		fprintf(stderr, "INFO: LOGIN, user=%s@%s, ip=[%s], mailstore=[%s]\n", TheUser, TheDomain, remoteip, mailstore);
		retval = monkey(mailstore, remote_port, loginbuf, skip_nl);
		if (retval == 2)
		{
			fprintf(stderr, "ERR: DISCONNECTED, user=%s@%s, ip=[%s], mailstore=[%s]\n", TheUser, TheDomain, remoteip, mailstore);
			printf("* BYE Disconnected for inactivity.\r\n");
			fflush(stdout);
		} else
			fprintf(stderr, "INFO: LOGOUT, user=%s@%s, ip=[%s], mailstore=[%s]\n", TheUser, TheDomain, remoteip, mailstore);
		return(retval);
	}
}

static int
LocalLogin(char **argv, char *user, char *TheDomain, char *service, 
	char *imaptag, char *plaintext)
{
	char            Email[MAX_BUFF];
	struct passwd  *pw;

	snprintf(Email, MAX_BUFF, "%s@%s", user, TheDomain);
	if (!(pw = inquery(PWD_QUERY, Email, 0)))
	{
		if (userNotFound)
		{
			if (!strncmp(service, "imap", 4))
			{
				snprintf(imaptagenv, MAX_BUFF, "IMAPLOGINTAG=%s", imaptag);
				putenv(imaptagenv);
			}
			execv(argv[0], argv);
			fprintf(stderr, "execv: %s: %s\n", argv[0], strerror(errno));
		} else
		{
			printf("Temporary Service Problem - code %d\n", __LINE__);
			fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
			fflush(stdout);
		}
		return(1);
	} 
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (strcmp("webmail", service) == 0)
	{
		if (pw->pw_gid & NO_WEBMAIL)
		{
			fprintf(stderr, "ExecImapd: webmail disabled for this account");
			return (-1);
		}
	} else
	if (strcmp("pop3", service) == 0)
	{
		if (pw->pw_gid & NO_POP)
		{
			fprintf(stderr, "ExecImapd: pop3 disabled for this account");
			return (-1);
		}
	} else
	if (strcmp("imap", service) == 0)
	{
		if (pw->pw_gid & NO_IMAP)
		{
			fprintf(stderr, "ExecImapd: imap disabled for this account");
			return (-1);
		}
	}
	remove_quotes(plaintext);
	if (pw->pw_passwd[0] && !pw_comp(0, (unsigned char *) pw->pw_passwd, 0,
		(unsigned char *) plaintext, 0))
	{
		if (!strncmp(service, "imap", 4))
		{
			snprintf(imaptagenv, MAX_BUFF, "IMAPLOGINTAG=%s", imaptag);
			putenv(imaptagenv);
		}
		ExecImapd(argv, Email, TheDomain, pw, service, imaptag);
		fprintf(stderr, "ExecImapd: %s: %s\n", argv[0], strerror(errno));
	} 
	if (!strncmp(service, "imap", 4))
	{
		snprintf(imaptagenv, MAX_BUFF, "IMAPLOGINTAG=%s", imaptag);
		putenv(imaptagenv);
	}
	execv(argv[0], argv);
	fprintf(stderr, "execv: %s: %s\n", argv[0], strerror(errno));
	return(1);
}

static int
ExecImapd(char **argv, char *userid, char *TheDomain, struct passwd *pw, char *service, char *imaptag)
{
	char            Maildir[MAX_BUFF], authenv1[MAX_BUFF], authenv2[MAX_BUFF],
	                authenv3[MAX_BUFF], authenv4[MAX_BUFF], authenv5[MAX_BUFF],
	                authenv6[MAX_BUFF], authenv7[MAX_BUFF], TheUser[MAX_BUFF],
	                TmpBuf[MAX_BUFF];
	char           *ptr, *cptr;
	int             status;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit;
#endif
	for(cptr = TheUser, ptr = userid;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	scopy(TmpBuf, service, MAX_BUFF);
	if ((ptr = strrchr(TmpBuf, ':')))
		*ptr = 0;
	if (Check_Login(TmpBuf, TheDomain, pw->pw_gecos))
	{
		fprintf(stderr, "Login not permitted for %s\n", TmpBuf);
		return (1);
	}
	snprintf(authenv1, MAX_BUFF, "AUTHENTICATED=%s", userid);
	snprintf(authenv2, MAX_BUFF, "AUTHADDR=%s@%s", TheUser, TheDomain);
	snprintf(authenv3, MAX_BUFF, "AUTHFULLNAME=%s", pw->pw_gecos);
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
	{
		fprintf(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
		return (1);
	}
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%"PRIu64"S,%"PRIu64"C", size_limit, count_limit);
#else
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%sS", 
		strncmp(pw->pw_shell, "NOQUOTA", 8) ? pw->pw_shell : "0");
#endif
	snprintf(authenv5, MAX_BUFF, "HOME=%s", pw->pw_dir);
	snprintf(authenv6, MAX_BUFF, "AUTHSERVICE=%s", service);
	putenv(authenv1);
	putenv(authenv2);
	putenv(authenv3);
	putenv(authenv4);
	putenv(authenv5);
	putenv(authenv6);
	switch ((status = Login_Tasks(pw, userid, TmpBuf)))
	{
		case 2:
			if (!(ptr = getenv("TMP_MAILDIR")))
				snprintf(Maildir, MAX_BUFF, "%s", pw->pw_dir);
			else
				snprintf(Maildir, MAX_BUFF, "%s", ptr);
			break;
		case 3:
			if ((ptr = getenv("EXIT_ONERROR")))
			{
				if ((ptr = getenv("MSG_ONERROR")) && !access(ptr, F_OK))
				{
					FILE *fp;

					if ((fp = fopen(ptr, "r")))
					{
						for (;;)
						{
							if (!fgets(TmpBuf, sizeof(TmpBuf) - 2, fp))
								break;
							fprintf(stdout, "%s", TmpBuf);
						}
						fflush(stdout);
						fclose(fp);
						/*- Flow through */
					} else
						fprintf(stderr, "%s: %s\n", ptr, strerror(errno));
				}
				fprintf(stderr, "POSTAUTH: Error on Exit\n");
				return(1);
			}
			if (!(ptr = getenv("TMP_MAILDIR")))
				snprintf(Maildir, MAX_BUFF, "%s", pw->pw_dir);
			else
				snprintf(Maildir, MAX_BUFF, "%s", ptr);
			break;
		default:
			snprintf(Maildir, MAX_BUFF, "%s", pw->pw_dir);
			break;
	}
	if (chdir(Maildir))
	{
		fprintf(stderr, "proxylogin: chdir: %s: %s\n", Maildir, strerror(errno));
		return(1);
	}
	snprintf(authenv7, MAX_BUFF, "MAILDIR=%s/Maildir", Maildir);
	putenv(authenv7);
	execv(argv[1], argv + 1);
	return(1);
}

int
AuthModuser(int argc, char **argv, unsigned timeout, unsigned errsleep)
{
	const char     *p;
	int             rc;
	char            namebuf[60];
	int             n;
	time_t          t, expinterval;
	char           *q, *r;

	p = getenv("AUTHUSER");
	if (!p && argc && argv[0][0] == '/')
	{
		/*- Set AUTHUSER from argv[0] */
		if (!(q = malloc(sizeof("AUTHUSER=") + strlen(argv[0]))))
		{
			perror("malloc");
			_exit(1);
		}
		strcat(strcpy(q, "AUTHUSER="), argv[0]);
		putenv(q);
	} else
	if (!p || *p != '/')
	{
		if (write(2, argv[0], strlen(argv[0])) == -1) ;
		if (write(2, ": AUTHUSER is not initialized to a complete path\n", 49) == -1) ;
		_exit(1);
	}
	putenv("AUTHENTICATED=");
	if (argc < 2)
	{
		if (write(2, "AUTHFAILURE\n", 12) == -1) ;
		_exit(1);
	}
	p = getenv("AUTHARGC");
	rc = p && *p && *p != '0' ? 0 : 1;
	sprintf(namebuf, "AUTHARGC=%d", argc);
	if (!(r = strdup(namebuf)))
	{
		perror("strdup");
		_exit(1);
	}
	putenv(r);
	for (n = 0; n < argc; n++)
	{
		char           *p;

		sprintf(namebuf, "AUTHARGV%d=", n);
		p = malloc(strlen(namebuf) + 1 + strlen(argv[n]));
		if (!p)
		{
			perror("malloc");
			_exit(1);
		}
		strcat(strcpy(p, namebuf), argv[n]);
		putenv(p);
	}
	if (rc == 0 && errsleep)
		sleep(errsleep);
	time(&t);
	p = getenv("AUTHEXPIRE");
	if (p && isdigit((int) (unsigned char) *p))
	{
		expinterval = 0;
		do
			expinterval = expinterval * 10 + (*p++ - '0');
		while (isdigit((int) (unsigned char) *p));
	} else
	{
		time_t          n;

		expinterval = t + timeout;
		q = namebuf + sizeof(namebuf) - 1;
		*q = 0;
		n = expinterval;
		do
			*--q = '0' + (n % 10);
		while ((n = n / 10) != 0);
		q -= sizeof("AUTHEXPIRE=") - 1;
		memcpy(q, "AUTHEXPIRE=", sizeof("AUTHEXPIRE=") - 1);
		q = strdup(q);
		if (!q)
		{
			perror("strdup");
			_exit(1);
		}
		putenv(q);
	}
	if (timeout)
	{
		if (expinterval <= t)
			_exit(1);
		alarm(expinterval - t);
	}
	return (rc);
}
#endif

static int
have_imap_starttls()
{
	const char     *p;

	if ((p = getenv("IMAP_STARTTLS")) == 0)
		return (0);
	if (*p != 'y' && *p != 'Y')
		return (0);
	p = getenv("COURIERTLS");
	if (!p || !*p)
		return (0);
	if (access(p, X_OK))
		return (0);
	return (1);
}


static int
tlsrequired()
{
	const char     *p = getenv("IMAP_TLS_REQUIRED");

	if (p && atoi(p))
		return (1);
	return (0);
}

void
imapd_capability()
{
	const char     *p;

	printf("* CAPABILITY ");

	if ((p = getenv("IMAP_TLS")) && atoi(p) && (p = getenv("IMAP_CAPABILITY_TLS")) && *p)
		printf("%s", p);
	else
	if ((p = getenv("IMAP_CAPABILITY")) != 0 && *p)
		printf("%s", p);
	else
		printf("IMAP4rev1");

	if (have_imap_starttls())
	{
		printf(" STARTTLS");
		if (tlsrequired())
			printf(" LOGINDISABLED");
	}
	printf("\r\n");
}

static int
have_pop3_starttls()
{
	const char     *p;

	if (!(p = getenv("POP3_STARTTLS")))
		return (0);
	if (*p != 'y' && *p != 'Y')
		return (0);
	p = getenv("COURIERTLS");
	if (!p || !*p)
		return (0);
	if (access(p, X_OK))
		return (0);
	return (1);
}

void pop3d_capability()
{
	const char     *p;

	printf("+OK Here's what I can do:\r\n");
	if ((p=getenv("POP3_TLS")) != 0 && atoi(p) && (p=getenv("POP3AUTH_TLS")) != 0 && *p)
		;
	else
		p=getenv("POP3AUTH");
	if (p && *p)
		printf("SASL %s\r\n", p);
	if (have_pop3_starttls())
		printf("STLS\r\n");
	printf("TOP\r\nUSER\r\nLOGIN-DELAY 10\r\nPIPELINING\r\nUIDL\r\n.\r\n");
}


void
getversion_proxylogin_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
