/*
 * $Log: authindi.c,v $
 * Revision 2.13  2009-11-18 14:21:31+05:30  Cprogrammer
 * more readable code
 *
 * Revision 2.12  2009-10-14 20:41:24+05:30  Cprogrammer
 * check parse_quota for return status
 *
 * Revision 2.11  2009-02-18 21:24:00+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.10  2009-02-06 11:36:22+05:30  Cprogrammer
 * ignore return values of write on stderr
 *
 * Revision 2.9  2008-12-19 11:51:59+05:30  Cprogrammer
 * changed PASSWD_CACHE to QUERY_CACHE
 * restrict authentication to local users
 *
 * Revision 2.8  2008-12-16 18:35:37+05:30  Cprogrammer
 * restrict authentication to local users only for distributed domains
 *
 * Revision 2.7  2008-11-21 11:59:32+05:30  Cprogrammer
 * added environment variable MAILDIR
 *
 * Revision 2.6  2008-11-20 22:15:45+05:30  Cprogrammer
 * exit if EXIT_ONERROR is defined and POSTAUTH returns exit code 3
 *
 * Revision 2.5  2008-11-13 19:22:22+05:30  Cprogrammer
 * removed extra close_connection()
 *
 * Revision 2.4  2008-11-06 15:02:01+05:30  Cprogrammer
 * changed PASSWD_CACHE to QUERY_CACHE
 *
 * Revision 2.3  2008-08-28 21:52:50+05:30  Cprogrammer
 * close mysql connection on exit
 *
 * Revision 2.2  2008-08-24 17:43:44+05:30  Cprogrammer
 * added code to return error for password changes
 *
 * Revision 2.1  2008-08-24 14:44:56+05:30  Cprogrammer
 * courier-imap authmodule for IndiMail
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifndef lint
static char     sccsid[] = "$Id: authindi.c,v 2.13 2009-11-18 14:21:31+05:30 Cprogrammer Stab mbhangui $";
#endif
#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#endif

int             authlen = AUTH_SIZE;
static int      exec_local(char **, char *, char *, struct passwd *, char *);

void
close_connection()
{
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
		vclose();
#else /*- Not QUERY_CACHE */
	vclose();
#endif
}

int
main(int argc, char **argv)
{
	char           *buf, *tmpbuf, *login, *challenge, *crypt_pass, *real_domain,
				   *prog_name, *service, *service_type;
	char            user[AUTH_SIZE], domain[AUTH_SIZE], Email[MAX_BUFF];
	int             count, offset;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;
	char           *(indiargs[]) = { INDIMAILDIR"/sbin/imaplogin", INDIMAILDIR"/libexec/authlib/authindi",
					INDIMAILDIR"/bin/imapd", "Maildir", 0 };

	if ((prog_name = strrchr(argv[0], '/')))
		prog_name++;
	else
		prog_name = argv[0];
	if (argc < 3)
	{
		fprintf(stderr, "%s: no more modules will be tried\n", prog_name);
		return(1);
	}
	if (!(tmpbuf = calloc(1, (authlen + 1) * sizeof(char))))
	{
		fprintf(stderr, "%s: malloc-%d: %s\n", prog_name, authlen + 1, strerror(errno));
		return(1);
	}
	/*-
	 * Courier-IMAP authmodules Protocol (authindi /var/indimail/bin/imapd Maildir < /tmp/input 3<&0)
	 * imap\n
	 * login\n
	 * postmaster@test.com\n
	 * pass\n
	 * newpass\n
	 * argv[0]=/var/indimail/libexec/authlib/try
	 * argv[1]=/var/indimail/libexec/authlib/authpam
	 * argv[2]=/var/indimail/bin/imapd
	 * argv[3]=Maildir
	 */
	for (offset = 0;;)
	{
		do
		{
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1)
		{
			fprintf(stderr, "read: %s\n", strerror(errno));
			return(1);
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
		{
			fprintf(stderr, "%s: auth data too long\n", prog_name);
			return(2);
		}
	}
	if (!(buf = calloc(1, (offset + 1) * sizeof(char))))
	{
		fprintf(stderr, "%s: malloc-%d: %s\n", prog_name, authlen + 1, strerror(errno));
		return(1);
	}
	memcpy(buf, tmpbuf, offset);
	count = 0;
	service = tmpbuf + count; /*- service */
	for (;tmpbuf[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset)
	{
		fprintf(stderr, "%s: auth data too short\n", prog_name);
		return(2);
	}
	tmpbuf[count++] = 0;

	service_type = tmpbuf + count; /* type (login or pass) */
	for (;tmpbuf[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset)
	{
		fprintf(stderr, "%s: auth data too short\n", prog_name);
		return(2);
	}
	tmpbuf[count++] = 0;

	login = tmpbuf + count; /*- username */
	for (;tmpbuf[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset)
	{
		fprintf(stderr, "%s: auth data too short\n", prog_name);
		return(2);
	}
	tmpbuf[count++] = 0;

	challenge = tmpbuf + count; /*- challenge (plain text) */
	for (;tmpbuf[count] != '\n' && count < offset;count++);
	tmpbuf[count++] = 0;
	if (!strncmp(service_type, "pass", 5))
	{
		fprintf(stderr, "%s: Password Change not supported\n", prog_name);
		pipe_exec(argv, buf, offset);
		return(1);
	}
	if (parse_email(login, user, domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: could not parse email [%s]\n", prog_name, login);
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (!vget_assign(domain, 0, 0, &uid, &gid)) 
	{
		fprintf(stderr, "%s: domain %s does not exist\n", prog_name, domain);
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	snprintf(Email, MAX_BUFF, "%s@%s", user, real_domain);
#ifdef CLUSTERED_SITE
	if (is_distributed_domain(real_domain))
	{
		char           *mailstore, *ptr;

#ifdef QUERY_CACHE
		if (getenv("QUERY_CACHE"))
			mailstore = inquery(HOST_QUERY, Email, 0);
		else
			mailstore = findhost(Email, 2);
#else
		mailstore = findhost(Email, 2);
#endif
		if (!mailstore)
		{
			if (!userNotFound)
				fprintf(stderr, "No mailstore for %s\n", Email);
			pipe_exec(argv, buf, offset);
			return(1);
		}
		if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
			*ptr = 0;
		for (;*mailstore && *mailstore != ':';mailstore++);
		mailstore++;
		if (!islocalif(mailstore))
		{
			fprintf(stderr, "%s not on local (mailstore %s)\n", Email, mailstore);
			return(1);
		}
	}
#endif /*- CLUSTERED_SITE */
#ifdef QUERY_CACHE
	if (getenv("QUERY_CACHE"))
		pw = inquery(PWD_QUERY, Email, 0);
	else
	{
		if (vauth_open((char *) 0))
		{
			if(!userNotFound)
				fprintf(stderr, "%s: inquery: %s\n", prog_name, strerror(errno));
			pipe_exec(argv, buf, offset);
			return (1);
		}
		pw = vauth_getpw(user, real_domain);
	}
#else
	if (vauth_open((char *) 0))
	{
		if(!userNotFound)
			fprintf(stderr, "%s: inquery: %s\n", prog_name, strerror(errno));
		pipe_exec(argv, buf, offset);
		return (1);
	}
	pw = vauth_getpw(user, real_domain);
#endif
	if (!pw)
	{
		if(!userNotFound)
			fprintf(stderr, "%s: inquery: %s\n", prog_name, strerror(errno));
		pipe_exec(argv, buf, offset);
		close_connection();
		return (1);
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
			fprintf(stderr, "%s: webmail disabled for this account", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			execv(*indiargs, argv);
			fprintf(stderr, "execv %s: %s", *indiargs, strerror(errno));
			return (1);
		}
	} else
	if (strcmp("pop3", service) == 0)
	{
		if (pw->pw_gid & NO_POP)
		{
			fprintf(stderr, "%s: pop3 disabled for this account", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			execv(*indiargs, argv);
			fprintf(stderr, "execv %s: %s", *indiargs, strerror(errno));
			return (1);
		}
	} else
	if (strcmp("imap", service) == 0)
	{
		if (pw->pw_gid & NO_IMAP)
		{
			fprintf(stderr, "%s: imap disabled for this account", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			execv(*indiargs, argv);
			fprintf(stderr, "execv %s: %s", *indiargs, strerror(errno));
			return (1);
		}
	}
	crypt_pass = pw->pw_passwd;
	if (getenv("DEBUG_LOGIN"))
	{
		fprintf(stderr, "%s: service[%s] type [%s] login [%s] challenge [%s] pw_passwd [%s]\n", 
			prog_name, service, service_type, login, challenge, crypt_pass);
	}
	if (pw_comp((unsigned char *) login, (unsigned char *) crypt_pass,
		(unsigned char *) challenge, 0))
	{
		if (argc == 3)
		{
			fprintf(stderr, "%s: no more modules will be tried\n", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			execv(*indiargs, indiargs);
			fprintf(stderr, "execv %s: %s", *indiargs, strerror(errno));
			return (1);
		}
		close_connection();
		pipe_exec(argv, buf, offset);
		return (1);
	}
	exec_local(argv + argc - 2, login, real_domain, pw, service);
	return(0);
}

static int
exec_local(char **argv, char *userid, char *TheDomain, struct passwd *pw, char *service)
{
	char            Maildir[MAX_BUFF], authenv1[MAX_BUFF], authenv2[MAX_BUFF], authenv3[MAX_BUFF],
	                authenv4[MAX_BUFF], authenv5[MAX_BUFF], authenv6[MAX_BUFF], TheUser[MAX_BUFF],
					TmpBuf[MAX_BUFF];
	char           *ptr, *cptr;
	int             status;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit;
#endif

	for (cptr = TheUser, ptr = userid;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	scopy(TmpBuf, service, MAX_BUFF);
	if ((ptr = strrchr(TmpBuf, ':')))
		*ptr = 0;
	if (Check_Login(TmpBuf, TheDomain, pw->pw_gecos))
	{
		fprintf(stderr, "Login not permitted for %s\n", TmpBuf);
		close_connection();
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
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%sS", pw->pw_shell);
#endif
	snprintf(authenv5, MAX_BUFF, "HOME=%s", pw->pw_dir);
	putenv(authenv1);
	putenv(authenv2);
	putenv(authenv3);
	putenv(authenv4);
	putenv(authenv5);
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
				close_connection();
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
	close_connection();
	if (chdir(Maildir))
	{
		fprintf(stderr, "authindi: chdir: %s: %s\n", Maildir, strerror(errno));
		return(1);
	}
	snprintf(authenv6, MAX_BUFF, "MAILDIR=%s/Maildir", Maildir);
	putenv(authenv6);
	execv(argv[0], argv);
	return(1);
}

void
getversion_authindi_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
